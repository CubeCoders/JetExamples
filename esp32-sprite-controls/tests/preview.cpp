#include "Courier.hpp"
#include "PerformanceOverlay.hpp"
#include <cassert>
#include <algorithm>
#include <thread>
#include <vector>
#include <cstdio>
static void parallel(Renderer::Scene& s) {
 std::vector<uint8_t> a(s.lastFrameDrawnTriangles),b(a.size());std::thread worker([&]{s.rasterizeBand(159,320,b.data());});s.rasterizeBand(0,159,a.data());worker.join();
 s.lastFrameRasterizedTriangles=0;for(size_t i=0;i<a.size();++i)s.lastFrameRasterizedTriangles+=!!(a[i]|b[i]);
}
int main(){
 constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
 std::vector<uint16_t> pixels(count+32,0xbeef);Renderer::Scene scene(pixels.data(),nullptr,w,h);scene.getRenderer()->interlacedMode=true;Courier::init(scene);PerformanceOverlay hud;hud.attach(scene,w);
 auto render=[&](float t,bool threaded){Courier::time=t;Courier::update(0);std::vector<uint16_t> out(w*h);
  for(int parity=0;parity<2;++parity){scene.frameCounter=parity;scene.render(threaded?parallel:nullptr);assert(scene.lastFrameRasterizedTriangles>30);assert(std::all_of(pixels.begin()+count,pixels.end(),[](auto p){return p==0xbeef;}));for(int y=1-parity;y<h;y+=2)for(int x=0;x<w;++x)out[y*w+x]=pixels[(y/2)*stride+x/2];}
  auto sprites=scene.getSprites();std::stable_sort(sprites.begin(),sprites.end(),[](auto* a,auto* b){return a->zOrder<b->zOrder;});for(int y=0;y<h;++y)Renderer::compositeSprites(out.data()+y*w,w,y,sprites.data(),int(sprites.size()));return out;
 };
 std::vector<uint16_t> montage(w*h*8);const float times[]={1,4,7.7f,9.5f,16,19,23,26.5f};
 for(int pose=0;pose<8;++pose){auto out=render(times[pose],false);assert(out==render(times[pose],true));for(int y=0;y<h;++y)std::copy(out.begin()+y*w,out.begin()+(y+1)*w,montage.begin()+((pose/2)*h+y)*w*2+(pose%2)*w);}
 for(float t:{0.f,6.99f,7.f,13.99f,14.f,20.99f,21.f,27.99f,28.f}){Courier::time=t;Courier::update(0);assert(Courier::stage==int(std::fmod(t,28.f)/7));}
 // Test this example's asymmetric artwork through each actual compositor flip.
 Renderer::Sprite2D s=Courier::ship;s.x=s.y=0;s.scale=1;s.alpha=255;Renderer::Sprite2D* ptr=&s;
 auto sprite=[&](int flags){s.textureFlags=uint8_t(flags);std::vector<uint16_t> image(64*32,0x1234);for(int y=0;y<32;++y)Renderer::compositeSprites(image.data()+y*64,64,y,&ptr,1);return image;};
 auto original=sprite(0),x=sprite(Renderer::Sprite2D::FLIP_X),y=sprite(Renderer::Sprite2D::FLIP_Y),xy=sprite(3);
 assert(original!=x&&original!=y);for(int j=0;j<32;++j)for(int i=0;i<64;++i){assert(original[j*64+i]==x[j*64+63-i]);assert(original[j*64+i]==y[(31-j)*64+i]);assert(original[j*64+i]==xy[(31-j)*64+63-i]);}
 Courier::time=16;Courier::update(0);assert(Courier::trails[0].enabled&&Courier::trailMaterial.alpha<255);assert(Courier::trails[0].alpha<Courier::trails[3].alpha);
 auto black=render(26.5f,false);assert(Courier::fadeMaterial.alpha==255);for(int j=70;j<277;++j)for(int i=0;i<w;++i)assert(black[j*w+i]==0);
 FILE* f=std::fopen("courier.ppm","wb");assert(f);std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*4);for(auto p:montage){unsigned char c[]={uint8_t(((p>>11)&31)*255/31),uint8_t(((p>>5)&63)*255/63),uint8_t((p&31)*255/31)};std::fwrite(c,1,3,f);}std::fclose(f);
 std::puts("Courier: flips, stage boundaries, full fade, field guards and eight serial/parallel composites pass");
}

#include "Courtyard.hpp"
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
int main() {
 constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
 std::vector<uint16_t> pixels(count+32,0xbeef);Renderer::Scene scene(pixels.data(),nullptr,w,h);scene.getRenderer()->interlacedMode=true;Courtyard::init(scene);PerformanceOverlay hud;hud.attach(scene,w);
 assert(Courtyard::subjects.size()==Courtyard::mirrors.size());
 for(size_t i=0;i<Courtyard::subjects.size();++i) {
  auto* a=Courtyard::subjects[i];auto* b=Courtyard::mirrors[i];assert(a->vertices.size()==b->vertices.size());
  assert(a->position.y==-b->position.y);
  for(size_t v=0;v<a->vertices.size();++v){assert(a->vertices[v].position.y==-b->vertices[v].position.y);assert(a->vertices[v].position.x==b->vertices[v].position.x&&a->vertices[v].position.z==b->vertices[v].position.z);}
  for(size_t t=0;t<a->triangles.size();++t)assert(a->triangles[t].v2==b->triangles[t].v3&&a->triangles[t].v3==b->triangles[t].v2);
 }
 auto render=[&](float time,int stage,bool threaded) {
  Courtyard::time=time;Courtyard::update(0);Courtyard::select(stage);std::vector<uint16_t> out(w*h);
  for(int parity=0;parity<2;++parity) {
   scene.frameCounter=parity;scene.render(threaded?parallel:nullptr);assert(scene.lastFrameRasterizedTriangles>30);
   assert(std::all_of(pixels.begin()+count,pixels.end(),[](auto p){return p==0xbeef;}));
   for(int y=1-parity;y<h;y+=2)for(int x=0;x<w;++x)out[y*w+x]=pixels[(y/2)*stride+x/2];
  }
  return out;
 };
 const auto bare=render(2,1,false),mirror=render(2,2,false),light=render(2,3,false),halo=render(2,4,false);
 unsigned reflected=0,brighter=0;
 for(int y=180;y<289;++y)for(int x=0;x<w;++x)reflected+=bare[y*w+x]!=mirror[y*w+x];
 for(size_t i=0;i<light.size();++i){assert(((light[i]>>11)&31)>=((mirror[i]>>11)&31));assert(((light[i]>>5)&63)>=((mirror[i]>>5)&63));assert((light[i]&31)>=(mirror[i]&31));brighter+=light[i]!=mirror[i];}
 assert(reflected>1500&&brighter>500);assert(light==halo); // Halos belong to the later sprite pass.
 assert(Courtyard::halos[0].sourceWidth()==32&&Courtyard::halos[0].sourceHeight()==32);
 auto composite=[&](std::vector<uint16_t>& out){auto sprites=scene.getSprites();std::stable_sort(sprites.begin(),sprites.end(),[](auto* a,auto* b){return a->zOrder<b->zOrder;});for(int y=0;y<h;++y)Renderer::compositeSprites(out.data()+y*w,w,y,sprites.data(),int(sprites.size()));};
 auto withHalos=halo;composite(withHalos);Courtyard::select(3);auto withoutHalos=light;composite(withoutHalos);
 unsigned haloPixels=0;for(int y=66;y<289;++y)for(int x=0;x<w;++x)haloPixels+=withHalos[y*w+x]!=withoutHalos[y*w+x];assert(haloPixels>50);
 std::vector<uint16_t> montage(w*h*8);const float times[]={0,3.5f,7,10.5f,2,2,2,2};const int modes[]={0,0,0,0,1,2,3,4};
 for(int pose=0;pose<8;++pose) {
  auto out=render(times[pose],modes[pose],false);assert(out==render(times[pose],modes[pose],true));composite(out);
  for(int y=0;y<h;++y)std::copy(out.begin()+y*w,out.begin()+(y+1)*w,montage.begin()+((pose/2)*h+y)*w*2+(pose%2)*w);
 }
 for(float t:{0.f,13.99f,14.f,20.99f,21.f,27.99f,28.f,34.99f,35.f,41.99f,42.f}){Courtyard::time=t;Courtyard::update(0);const float phase=std::fmod(t,42.f);assert(Courtyard::stage==(phase<14?0:1+int((phase-14)/7)));}
 FILE* f=std::fopen("courtyard.ppm","wb");assert(f);std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*4);
 for(auto p:montage){unsigned char c[]={uint8_t(((p>>11)&31)*255/31),uint8_t(((p>>5)&63)*255/63),uint8_t((p&31)*255/31)};std::fwrite(c,1,3,f);}std::fclose(f);
 std::printf("Courtyard: mirror positions/winding, %u reflective pixels, %u additive pixels, %u halo pixels, serial/parallel fields, guards and transitions pass\n",reflected,brighter,haloPixels);
}

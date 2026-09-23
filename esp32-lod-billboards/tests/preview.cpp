#include "Woodland.hpp"
#include "PerformanceOverlay.hpp"
#include <cassert>
#include <algorithm>
#include <thread>
#include <vector>
#include <cstdio>
static void parallel(Renderer::Scene& s){std::vector<uint8_t>a(s.lastFrameDrawnTriangles),b(a.size());std::thread t([&]{s.rasterizeBand(159,320,b.data());});s.rasterizeBand(0,159,a.data());t.join();s.lastFrameRasterizedTriangles=0;for(size_t i=0;i<a.size();++i)s.lastFrameRasterizedTriangles+=!!(a[i]|b[i]);}
int main(){
 constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
 std::vector<uint16_t> pixels(count+32,0xbeef);Renderer::Scene scene(pixels.data(),nullptr,w,h);scene.getRenderer()->interlacedMode=true;Woodland::init(scene);PerformanceOverlay hud;hud.attach(scene,w);
 auto render=[&](float t,bool threaded){Woodland::time=t;Woodland::update(0);std::vector<uint16_t> out(w*h);for(int parity=0;parity<2;++parity){scene.frameCounter=parity;scene.render(threaded?parallel:nullptr);assert(std::all_of(pixels.begin()+count,pixels.end(),[](auto p){return p==0xbeef;}));for(int y=1-parity;y<h;y+=2)for(int x=0;x<w;++x)out[y*w+x]=pixels[(y/2)*stride+x/2];}auto sprites=scene.getSprites();std::stable_sort(sprites.begin(),sprites.end(),[](auto* a,auto* b){return a->zOrder<b->zOrder;});for(int y=0;y<h;++y)Renderer::compositeSprites(out.data()+y*w,w,y,sprites.data(),int(sprites.size()));return out;};
 std::vector<uint16_t> montage(w*h*6);const float times[]={0,3,4.55f,8,16,24};int tris[6];
 for(int pose=0;pose<6;++pose){auto out=render(times[pose],false);tris[pose]=scene.lastFrameRasterizedTriangles;assert(out==render(times[pose],true));std::printf("Pose %.2f: %d triangles; distance %d; stage %d\n",times[pose],tris[pose],Woodland::distance,Woodland::stage);for(int y=0;y<h;++y)std::copy(out.begin()+y*w,out.begin()+(y+1)*w,montage.begin()+((pose/2)*h+y)*w*2+(pose%2)*w);}
 assert(tris[0]>tris[1]);assert(tris[1]>tris[3]);assert(tris[5]>tris[3]+40);assert(tris[0]==tris[4]);
 // Opaque pine visibility should not depend on the source triangle order.
 for(float t=0.f;t<32.f;t+=.5f) {
  const auto normal=render(t,false);
  std::reverse(Woodland::tree->triangles.begin(),Woodland::tree->triangles.end());
  std::reverse(Woodland::simple->triangles.begin(),Woodland::simple->triangles.end());
  const auto reversed=render(t,false);unsigned changed=0;
  for(size_t i=0;i<normal.size();++i)changed+=normal[i]!=reversed[i];
  std::printf("Painter order check %.1f: %u differing pixels\n",t,changed);
  assert(changed<=16); // Allow at most eight half-width boundary samples with equal sort depth.
  std::reverse(Woodland::tree->triangles.begin(),Woodland::tree->triangles.end());
  std::reverse(Woodland::simple->triangles.begin(),Woodland::simple->triangles.end());
 }
 assert(Woodland::impostor->isBillboard);assert(Woodland::impostor->triangles.size()==2);assert(Woodland::tree->lodMeshes[0]==Woodland::simple);
 // Test the exact engine distance boundaries with a level camera at the shared centre.
 auto check=[&](int distance){Woodland::time=0;Woodland::update(0);Woodland::camera.setPosition(0,280,-distance);Woodland::camera.lookAt({0,280,0});scene.frameCounter=0;scene.render();return scene.lastFrameRasterizedTriangles;};
 int high=check(1799),low=check(1801),transition=check(2500),far=check(2601);assert(high>low+20);assert(transition>far+20);std::printf("Boundary tris: %d -> %d -> %d -> %d\n",high,low,transition,far);
 // Distance/scale shrinks the world-space billboard; it is not a screen overlay.
 assert(Woodland::impostor->position.y==Woodland::tree->position.y);
 FILE* f=std::fopen("woodland.ppm","wb");assert(f);std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*3);for(auto p:montage){unsigned char c[]={uint8_t(((p>>11)&31)*255/31),uint8_t(((p>>5)&63)*255/63),uint8_t((p&31)*255/31)};std::fwrite(c,1,3,f);}std::fclose(f);
 std::puts("Woodland: LOD boundaries, actual triangle reductions, reference path, field guards and serial/parallel rendering pass");
}

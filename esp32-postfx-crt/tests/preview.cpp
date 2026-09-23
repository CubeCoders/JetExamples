#include "CrtDemo.hpp"
#include "PerformanceOverlay.hpp"
#include <cassert>
#include <algorithm>
#include <cstdio>
#include <thread>
#include <vector>
static void parallel(Renderer::Scene& scene) {
 const int n=scene.lastFrameDrawnTriangles;std::vector<uint8_t> a(n),b(n);
 std::thread worker([&]{scene.rasterizeBand(157,320,b.data());});scene.rasterizeBand(0,157,a.data());worker.join();
 scene.lastFrameRasterizedTriangles=0;for(int i=0;i<n;++i)scene.lastFrameRasterizedTriangles+=!!(a[i]|b[i]);
}
int main() {
 constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
 std::vector<uint16_t> fields[2]={std::vector<uint16_t>(count+32,0xbeef),std::vector<uint16_t>(count+32,0xbeef)};
 Renderer::Scene scene(fields[0].data(),nullptr,w,h);scene.getRenderer()->interlacedMode=true;
 CrtDemo::init(scene);PerformanceOverlay stats;stats.attach(scene,w);
 auto render=[&](float time,bool threaded) {
  CrtDemo::time=time;CrtDemo::update(0);
  for(int parity=0;parity<2;++parity) {
   auto& pixels=fields[parity];scene.setFramebuffer(pixels.data());scene.frameCounter=parity;
   scene.render(threaded?parallel:nullptr);
   assert(scene.lastFrameRasterizedTriangles>=12);
   assert(std::all_of(pixels.begin()+count,pixels.end(),[](auto p){return p==0xbeef;}));
  }
  std::vector<uint16_t> out(w*h);
  for(int y=0;y<h;++y)for(int x=0;x<w;++x)out[y*w+x]=fields[(y&1)?0:1][(y/2)*stride+x/2];
  return out;
 };
 std::vector<uint16_t> montage(w*h*4);int pose=0;
 for(float t:{1.5f,4.f}) {
  auto off=render(t,false),on=render(t+7,false);
  assert(off==render(t,true));assert(on==render(t+7,true));unsigned changed=0;
  for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
   const auto p=off[y*w+x];const double factor=143.0/255;
   uint16_t expected=p;
   if(y&1)expected=uint16_t((int(((p>>11)&31)*factor+.5)<<11)|(int(((p>>5)&63)*factor+.5)<<5)|int((p&31)*factor+.5));
   assert(on[y*w+x]==expected);changed+=on[y*w+x]!=p;
  }
  assert(changed>w*h/3);
  for(int m=0;m<2;++m) {
   auto frame=m?on:off;CrtDemo::time=t+m*7;CrtDemo::update(0);auto& sprites=scene.getSprites();
   for(int y=0;y<h;++y) {
    Renderer::compositeSprites(frame.data()+y*w,w,y,sprites.data(),int(sprites.size()));
    std::copy(frame.begin()+y*w,frame.begin()+(y+1)*w,montage.begin()+((pose/2)*h+y)*w*2+(pose%2)*w);
   }
   ++pose;
  }
 }
 // A redraw prevents accumulated darkening; toggling off restores the same scene.
 assert(render(8.5f,false)==render(8.5f,false));assert(render(1.5f,false)==render(1.5f,false));
 CrtDemo::time=6.99f;CrtDemo::update(.02f);assert(scene.crtEnabled);
 CrtDemo::time=13.99f;CrtDemo::update(.02f);assert(!scene.crtEnabled);
 FILE* f=std::fopen("crt-preview.ppm","wb");assert(f);std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*2);
 for(auto p:montage){unsigned char rgb[]={uint8_t(((p>>11)&31)*255/31),uint8_t(((p>>5)&63)*255/63),uint8_t((p&31)*255/31)};std::fwrite(rgb,1,3,f);}std::fclose(f);
 std::puts("CRT scene: matched motion, exact physical row darkening, guards, serial/parallel output and toggles pass");
}

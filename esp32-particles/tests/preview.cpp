#include "ParticleDemo.hpp"
#include "PerformanceOverlay.hpp"
#include <cassert>
#include <algorithm>
#include <thread>
#include <vector>
#include <cstdio>
static void parallel(Renderer::Scene& scene) {
 std::vector<uint8_t> a(scene.lastFrameDrawnTriangles),b(a.size());
 std::thread worker([&]{scene.rasterizeBand(157,320,b.data());});scene.rasterizeBand(0,157,a.data());worker.join();
 scene.lastFrameRasterizedTriangles=0;for(size_t i=0;i<a.size();++i)scene.lastFrameRasterizedTriangles+=!!(a[i]|b[i]);
}
int main() {
 constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
 std::vector<uint16_t> pixels(count+16,0xbeef),montage(w*h*6);Renderer::Scene scene(pixels.data(),nullptr,w,h);
 scene.getRenderer()->interlacedMode=true;std::srand(123);ParticleDemo::init(scene);PerformanceOverlay hud;hud.attach(scene,w);
 const int frames[]={168,1008,1848,2688,2940,3312};int pose=0;bool seen[4]={};unsigned comparisons=0;
 for(int frame=1;frame<=3360;++frame) {
  ParticleDemo::update(1.f/120.f);seen[ParticleDemo::mode]=true;assert(ParticleDemo::particles.activeCount()<=200);
  if(pose>=6||frame!=frames[pose])continue;
  for(int parity=0;parity<2;++parity) {
   scene.frameCounter=parity;scene.render();const int geometry=scene.lastFrameRasterizedTriangles;
   const unsigned drawn=ParticleDemo::renderEffects(scene);scene.lastFrameRasterizedTriangles+=drawn;
   assert(geometry>10&&drawn<=ParticleDemo::particles.activeCount());
   assert(std::all_of(pixels.begin()+count,pixels.end(),[](auto p){return p==0xbeef;}));
   const auto reference=pixels;const int total=scene.lastFrameRasterizedTriangles;
   scene.frameCounter=parity;scene.render(parallel);scene.lastFrameRasterizedTriangles+=ParticleDemo::renderEffects(scene);
   assert(pixels==reference&&scene.lastFrameRasterizedTriangles==total);++comparisons;
   if(pose==4)assert(ParticleDemo::particles.activeCount()>0&&drawn==0);else assert(drawn>0);
   for(int y=1-parity;y<h;y+=2)for(int x=0;x<w;++x)montage[((pose/2)*h+y)*w*2+(pose%2)*w+x]=reference[(y/2)*stride+x/2];
  }
  const auto& sprites=scene.getSprites();for(int y=0;y<h;++y)Renderer::compositeSprites(montage.data()+((pose/2)*h+y)*w*2+(pose%2)*w,w,y,sprites.data(),int(sprites.size()));
  ++pose;
 }
 assert(pose==6&&ParticleDemo::peakLive==200&&std::all_of(seen,seen+4,[](bool v){return v;}));
 ParticleDemo::update(.02f);assert(ParticleDemo::mode==0);
 FILE* f=std::fopen("particles.ppm","wb");assert(f);std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*3);
 for(auto p:montage){unsigned char c[]={uint8_t(((p>>11)&31)*255/31),uint8_t(((p>>5)&63)*255/63),uint8_t((p&31)*255/31)};std::fwrite(c,1,3,f);}std::fclose(f);
 std::printf("Particle Lab: full cycle, 200-slot peak, distance culling, %u parallel comparisons, inclusive triangle counts and guards pass\n",comparisons);
}

#include "TextureLab.hpp"
#include "PerformanceOverlay.hpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <thread>
#include <vector>
int main() {
 constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
 std::vector<uint16_t> fields[2]={std::vector<uint16_t>(count+16,0xbeef),std::vector<uint16_t>(count+16,0xbeef)};
 Renderer::Scene scene(fields[0].data(),nullptr,w,h);scene.getRenderer()->interlacedMode=true;
 TextureLab::init(scene);PerformanceOverlay stats;stats.attach(scene,w);
 auto render=[&](float seconds,bool parallel=false) {
  TextureLab::time=seconds;TextureLab::update(0);
  for(int parity=0;parity<2;++parity) {
   auto& pixels=fields[parity];scene.setFramebuffer(pixels.data());scene.frameCounter=parity;
   scene.prepareFrame();scene.rasterizeBand(0,h);auto reference=pixels;
   assert(scene.lastFrameRasterizedTriangles>4);
   assert(std::all_of(pixels.begin()+count,pixels.end(),[](auto p){return p==0xbeef;}));
   if(parallel)for(int split:{1,157,319}) {
    scene.prepareFrame();std::vector<uint8_t> a(scene.lastFrameDrawnTriangles),b(a.size());
    std::thread worker([&]{scene.rasterizeBand(split,h,b.data());});
    scene.rasterizeBand(0,split,a.data());worker.join();assert(pixels==reference);
   }
  }
  std::vector<uint16_t> frame(w*h);
  for(int y=0;y<h;++y)for(int x=0;x<w;++x)frame[y*w+x]=fields[(y&1)?0:1][(y/2)*stride+x/2];
  return frame;
 };
 auto& tile=TextureLab::tile;
 tile.addressMode=Renderer::WRAP;
 assert(tile.getPixel(-256,128)==tile.getPixel(768,128));
 tile.addressMode=Renderer::CLAMP;
 assert(tile.getPixel(-256,128)==tile.getPixel(0,128));
 tile.addressMode=Renderer::ZERO;
 assert(tile.getPixel(-1,128)==0 && tile.getPixel(1024,128)==0);
 assert(TextureLab::foliageKeyed.getPixel(0,0)==0xf81f);
 assert(TextureLab::foliageOpaque.data==TextureLab::foliageKeyed.data);
 const auto lava0=render(28);const auto lava1=render(29);
 assert(lava0!=lava1 && TextureLab::lava.paletteOffset==18);
 assert(TextureLab::lava.data==reinterpret_cast<const uint16_t*>(Assets::lavaIndices));
 const auto keyed=render(22.5f);
 unsigned leftKey=0,rightKey=0;
 for(int y=70;y<258;++y)for(int x=50;x<430;++x)if(keyed[y*w+x]==0xf81f) {
  if(x<w/2)++leftKey;else ++rightKey;
 }
 assert(leftKey>500 && rightKey==0);
 const auto lodOff=render(38.5f),lodOn=render(45.5f);
 assert(lodOff!=lodOn);
 assert(lodOn[160*w+240]==TextureLab::panelMaterial.color);
 assert(TextureLab::panel->position.z>scene.getRenderer()->textureLodFar);
 const auto nearOff=render(35.f),nearOn=render(42.f);assert(nearOff==nearOn);
 std::vector<uint16_t> montage(w*h*8);
 const float poses[]={1.5f,8.5f,15.5f,22.5f,29.5f,36.5f,43.5f,45.5f};
 int pose=0;
 for(float seconds:poses) {
  auto frame=render(seconds,true);auto& sprites=scene.getSprites();
  for(int y=0;y<h;++y) {
   Renderer::compositeSprites(frame.data()+y*w,w,y,sprites.data(),int(sprites.size()));
   std::copy(frame.begin()+y*w,frame.begin()+(y+1)*w,montage.begin()+((pose/2)*h+y)*w*2+(pose%2)*w);
  }
  ++pose;
 }
 // Both sides of every transition and the full-cycle wrap stay in range.
 for(int stage=0;stage<7;++stage) {
  render(stage*7.f);assert(TextureLab::activeStage==stage);
  render(stage*7.f+6.99f);assert(TextureLab::activeStage==stage);
 }
 TextureLab::time=48.99f;TextureLab::update(.02f);assert(TextureLab::activeStage==0);
 FILE* f=std::fopen("texture-lab.ppm","wb");assert(f);std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*4);
 for(auto p:montage){unsigned char rgb[]={uint8_t(((p>>11)&31)*255/31),uint8_t(((p>>5)&63)*255/63),uint8_t((p&31)*255/31)};std::fwrite(rgb,1,3,f);}std::fclose(f);
 std::puts("Texture Lab: addressing, keyed holes, palette motion, LOD endpoints, transitions, guards and 48 parallel cases pass");
}

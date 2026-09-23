#include "CelTeapot.hpp"
#include "PerformanceOverlay.hpp"
#include <cassert>
#include <algorithm>
#include <thread>
#include <vector>
#include <set>
int main() {
 static_assert(Z_BUFFERING && POSTFX_CELLSHADING && !FAST_Z);
 constexpr int w=480,h=320,stride=w/2,count=stride*h/2,depthCount=stride*h;
 std::vector<uint16_t> pixels(count+16,0xbeef),depth(depthCount+16,0xd00d);
 Renderer::Scene scene(pixels.data(),depth.data(),w,h);scene.getRenderer()->interlacedMode=true;CelTeapot::init(scene);
 std::vector<uint16_t> montage(w*h*4);unsigned changed=0,parallelCases=0;std::set<uint16_t> colours[2];
 for(int pose=0;pose<28;++pose) {
  const float phase=pose*.25f;std::vector<uint16_t> smooth[2],smoothDepth[2];
  for(int mode=0;mode<2;++mode) {
   CelTeapot::time=phase+7*mode;CelTeapot::update(0);assert(scene.getRenderer()->celShadingEnabled==bool(mode));
   assert(CelTeapot::glaze.shadingMode==Renderer::ShadingMode::PHONG);
   for(int parity=0;parity<2;++parity) {
    std::fill(depth.begin(),depth.begin()+depthCount,0xd00d);scene.frameCounter=parity;scene.render();assert(scene.lastFrameRasterizedTriangles>100);
    assert(std::all_of(pixels.begin()+count,pixels.end(),[](auto p){return p==0xbeef;}));
    assert(std::all_of(depth.begin()+depthCount,depth.end(),[](auto p){return p==0xd00d;}));
    const auto reference=pixels,referenceDepth=depth;
    if(!mode){smooth[parity]=pixels;smoothDepth[parity]=depth;}
    else {
     assert(depth==smoothDepth[parity]);
     for(int y=1-parity;y<h;y+=2)for(int x=0;x<stride;++x) {
      const int i=y/2*stride+x;
      if(depth[y*stride+x]==0xffff)assert(pixels[i]==smooth[parity][i]);
      else {changed+=pixels[i]!=smooth[parity][i];colours[0].insert(smooth[parity][i]);colours[1].insert(pixels[i]);}
     }
    }
    if(pose%7==0)for(int split:{1,159,319}) {
     scene.frameCounter=parity;scene.prepareFrame();std::vector<uint8_t> a(scene.lastFrameDrawnTriangles),b(a.size());
     std::thread worker([&]{scene.rasterizeBand(split,h,b.data());});scene.rasterizeBand(0,split,a.data());worker.join();
     assert(pixels==reference&&depth==referenceDepth);++parallelCases;
    }
    if(pose==6||pose==12) {
     const int row=pose==6?0:1;for(int y=1-parity;y<h;y+=2)for(int x=0;x<w;++x)montage[(row*h+y)*w*2+mode*w+x]=reference[(y/2)*stride+x/2];
    }
   }
  }
 }
 assert(changed>10000);assert(colours[1].size()<colours[0].size());
 for(float t:{0.f,6.99f,7.f,13.99f,14.f}){CelTeapot::time=t;CelTeapot::update(0);assert(CelTeapot::activeMode==int(t/7)%2);}
 CelTeapot::time=13.99f;CelTeapot::update(.02f);assert(CelTeapot::activeMode==0);
 PerformanceOverlay overlay;overlay.attach(scene,w);
 for(int row=0;row<2;++row)for(int mode=0;mode<2;++mode) {
  Labels::select(mode);const auto& sprites=scene.getSprites();for(int y=0;y<h;++y)Renderer::compositeSprites(montage.data()+(row*h+y)*w*2+mode*w,w,y,sprites.data(),int(sprites.size()));
 }
 FILE* f=std::fopen("cel-teapot.ppm","wb");assert(f);std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*2);
 for(auto p:montage){unsigned char c[]={uint8_t(((p>>11)&31)*255/31),uint8_t(((p>>5)&63)*255/63),uint8_t((p&31)*255/31)};std::fwrite(c,1,3,f);}std::fclose(f);
 std::printf("Cel: 28 matched poses, %u parallel comparisons, %u changed samples, colour counts %zu/%zu; guards/depth/background unchanged\n",parallelCases,changed,colours[0].size(),colours[1].size());
}

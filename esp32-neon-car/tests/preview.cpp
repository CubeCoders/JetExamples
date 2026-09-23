#include "Workshop.hpp"
#include "PerformanceOverlay.hpp"
#include <cassert>
#include <thread>
#include <vector>
#include <algorithm>
int main() {
 constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
 std::vector<uint16_t> fields[2]={std::vector<uint16_t>(count+16,0xbeef),std::vector<uint16_t>(count+16,0xbeef)};
 Renderer::Scene scene(fields[0].data(),nullptr,w,h);scene.getRenderer()->interlacedMode=true;
 Workshop::init(scene);PerformanceOverlay stats;stats.attach(scene,w);
 assert(Workshop::car->triangles.size()+Workshop::windows[0]->triangles.size()+Workshop::windows[1]->triangles.size()==404);
 assert(Workshop::materials.size()==6);
 std::vector<uint16_t> montage(w*h*4);
 unsigned changes=0;auto uv=Workshop::windows[0]->vertices[0].uv;
 for(int pose=0;pose<4;++pose) {
  Workshop::time=pose*5.f;Workshop::update(0);
  auto next=Workshop::windows[0]->vertices[0].uv;changes+=next.x!=uv.x||next.y!=uv.y;
  for(int parity=0;parity<2;++parity) {
   auto& pixels=fields[parity];scene.setFramebuffer(pixels.data());scene.frameCounter=parity;
   scene.prepareFrame();scene.rasterizeBand(0,h);const auto reference=pixels;
   assert(scene.lastFrameRasterizedTriangles>100);
   assert(std::all_of(pixels.begin()+count,pixels.end(),[](auto p){return p==0xbeef;}));
   for(int split:{1,157,200,319}) {
    scene.prepareFrame();std::vector<uint8_t> a(scene.lastFrameDrawnTriangles),b(a.size());
    std::thread worker([&]{scene.rasterizeBand(split,h,b.data());});
    scene.rasterizeBand(0,split,a.data());worker.join();assert(pixels==reference);
   }
  }
  auto& sprites=scene.getSprites();std::vector<uint16_t> row(w);
  for(int y=0;y<h;++y) {
   for(int x=0;x<w;++x)row[x]=fields[(y&1)?0:1][(y/2)*stride+x/2];
   Renderer::compositeSprites(row.data(),w,y,sprites.data(),int(sprites.size()));
   std::copy(row.begin(),row.end(),montage.begin()+((pose/2)*h+y)*w*2+(pose%2)*w);
  }
 }
 assert(changes>=3);
 FILE* f=std::fopen("car.ppm","wb");assert(f);std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*2);
 for(auto p:montage){unsigned char rgb[]={uint8_t(((p>>11)&31)*255/31),uint8_t(((p>>5)&63)*255/63),uint8_t((p&31)*255/31)};std::fwrite(rgb,1,3,f);}std::fclose(f);
 std::printf("Car: loaded 404 triangles; four orbit views and 32 parallel band cases match\n");
}

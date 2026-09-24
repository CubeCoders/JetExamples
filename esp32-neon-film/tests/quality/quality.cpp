#include "Film.hpp"
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif
// Full 2880x1920 frames, interpolated per-pixel depth, filtered perspective UVs.
// The encoder downsamples to 1920x1280 for spatial anti-aliasing. No alternating
// fields, half-width storage, HUD or fabricated hardware FPS measurements.
int main(int argc,char** argv){
 const float start=argc>1?float(std::atof(argv[1])):0;
 const float seconds=argc>2?float(std::atof(argv[2])):Film::duration+1;
 constexpr int w=Film::renderWidth,h=Film::renderHeight,count=w*h;
#ifdef _WIN32
 _setmode(_fileno(stdout),_O_BINARY);
#endif
 constexpr size_t guard=64;constexpr uint16_t sentinel=0xA55A;
 std::vector<uint16_t> a(count+guard*2,sentinel),b(count+guard*2,sentinel),depth(count+guard*2,sentinel),composed(count+guard*2,sentinel);
 std::vector<unsigned char> rgb(count*3);
 Renderer::Scene scene(a.data()+guard,depth.data()+guard,w,h);Film::init(scene);
 for(int frame=0;frame<int(seconds*60);++frame){
  Film::seek(std::min(Film::duration,start+frame/60.f));
  scene.lodScale=0;
  for(auto& m:Film::bank.materials)m->perspectiveCorrect=true;
  auto* write=(frame%2?b:a).data()+guard;auto* read=(frame%2?a:b).data()+guard;
  scene.setFramebuffer(write);scene.getRenderer()->reflectBuffer=frame?read:nullptr;
  scene.prepareFrame();
  if(Film::water){
   // The hardware's byte-sized reflection offset is too small at this size.
   // Fold the full-resolution shore offset into the reflection axis instead.
   auto v=Film::camera.transformDirection(Vector3{0,0,1900}-Film::camera.position);
   int shore=h/2-int(v.y*Film::camera.fovFactor/v.z);
   scene.getRenderer()->waterlineY=std::max(scene.getRenderer()->waterlineY,shore);
   Film::water->waterYBias=0;
  }
  scene.rasterizeBand(0,h);++scene.frameCounter;Film::effects(scene);
  // Like firmware scanout, overlays never enter the next reflection source.
  // Otherwise water mirrors credits/fades and accumulates sprite halos.
  auto* present=composed.data()+guard;std::copy(write,write+count,present);
  // Geometry and particles, then glow/fade overlays on the presentation copy.
  auto sprites=scene.getSprites();std::stable_sort(sprites.begin(),sprites.end(),[](auto* a,auto* b){return a->zOrder<b->zOrder;});
  for(int y=0;y<h;++y)Renderer::compositeSprites(present+y*w,w,y,sprites.data(),int(sprites.size()));
  for(auto* buffer:{&a,&b,&depth,&composed})for(size_t i=0;i<guard;++i)if((*buffer)[i]!=sentinel||(*buffer)[count+guard+i]!=sentinel){std::fprintf(stderr,"Frame buffer guard overwritten at frame %d\n",frame);return 2;}
  for(int i=0;i<count;++i){auto p=present[i];rgb[i*3]=((p>>11)&31)*255/31;rgb[i*3+1]=((p>>5)&63)*255/63;rgb[i*3+2]=(p&31)*255/31;}
  if(std::fwrite(rgb.data(),1,rgb.size(),stdout)!=rgb.size())return 1;
  if(frame%600==0)std::fprintf(stderr,"Quality render %.1f / %.1f seconds\n",frame/60.f,seconds);
 }
 Film::bank.clear();return 0;
}

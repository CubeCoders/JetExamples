#include "Exhibition.hpp"
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif
// Stream the same packed alternating fields used on the S3 as RGB24. The
// encoder controls playback; native render throughput is not hardware FPS.
int main(int argc,char** argv){
 const float start=argc>1?float(std::atof(argv[1])):0;
 const float seconds=argc>2?float(std::atof(argv[2])):Matter::duration+1;
 constexpr int w=480,h=320,count=w*h/4;
#ifdef _WIN32
 _setmode(_fileno(stdout),_O_BINARY);
#endif
 std::vector<uint16_t>a(count),b(count),out(w*h);
 std::vector<unsigned char> rgb(w*h*3);
 Renderer::Scene scene(a.data(),nullptr,w,h);scene.getRenderer()->interlacedMode=true;Matter::init(scene);
 for(int frame=0;frame<int(seconds*60);++frame){
  Matter::seek(std::min(Matter::duration,start+frame/60.f));
  auto& write=frame%2?b:a;auto& read=frame%2?a:b;
  scene.setFramebuffer(write.data());scene.getRenderer()->reflectBuffer=read.data();
  scene.render();Matter::effects(scene);
  for(int y=0;y<h;++y){auto& source=(y&1)?a:b;for(int x=0;x<w;++x)out[y*w+x]=source[(y/2)*(w/2)+x/2];}
  auto sprites=scene.getSprites();std::stable_sort(sprites.begin(),sprites.end(),[](auto* a,auto* b){return a->zOrder<b->zOrder;});
  for(int y=0;y<h;++y)Renderer::compositeSprites(out.data()+y*w,w,y,sprites.data(),int(sprites.size()));
  for(int i=0;i<w*h;++i){auto p=out[i];rgb[i*3]=((p>>11)&31)*255/31;rgb[i*3+1]=((p>>5)&63)*255/63;rgb[i*3+2]=(p&31)*255/31;}
  if(std::fwrite(rgb.data(),1,rgb.size(),stdout)!=rgb.size())return 1;
 }
 Matter::bank.clear();return 0;
}

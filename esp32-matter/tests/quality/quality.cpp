#include "Exhibition.hpp"
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif
int main(int argc,char** argv){
 const float start=argc>1?float(std::atof(argv[1])):0;
 const float seconds=argc>2?float(std::atof(argv[2])):Matter::duration+1;
 constexpr int w=Matter::width,h=Matter::height,n=w*h,guard=128;
#ifdef _WIN32
 _setmode(_fileno(stdout),_O_BINARY);
#endif
 std::vector<uint16_t> pixels(n+guard*2,0xA55A),depth(n+guard*2,0xA55A);
 std::vector<unsigned char> rgb(n*3);
 Renderer::Scene scene(pixels.data()+guard,depth.data()+guard,w,h);Matter::init(scene);
 Matter::room.bilinear=true;
 for(int frame=0;frame<int(seconds*60);++frame){
  Matter::seek(std::min(Matter::duration,start+frame/60.f));
  for(auto& material:Matter::bank.materials)material->perspectiveCorrect=true;
  scene.render();Matter::effects(scene);
  for(int i=0;i<guard;++i)if(pixels[i]!=0xA55A||pixels[n+guard+i]!=0xA55A||depth[i]!=0xA55A||depth[n+guard+i]!=0xA55A)return 2;
  auto sprites=scene.getSprites();std::stable_sort(sprites.begin(),sprites.end(),[](auto* a,auto* b){return a->zOrder<b->zOrder;});
  for(int y=0;y<h;++y)Renderer::compositeSprites(pixels.data()+guard+y*w,w,y,sprites.data(),int(sprites.size()));
  for(int i=0;i<n;++i){auto p=pixels[guard+i];rgb[i*3]=((p>>11)&31)*255/31;rgb[i*3+1]=((p>>5)&63)*255/63;rgb[i*3+2]=(p&31)*255/31;}
  if(std::fwrite(rgb.data(),1,rgb.size(),stdout)!=rgb.size())return 1;
 }
 Matter::clearGlows();Matter::bank.clear();return 0;
}

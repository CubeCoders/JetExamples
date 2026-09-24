#include "Film.hpp"
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif
int main(int argc,char** argv){
 const float seconds=argc>2?float(std::atof(argv[2])):16.f;
 constexpr int w=480,h=320,count=w*h/4;
#ifdef _WIN32
 _setmode(_fileno(stdout),_O_BINARY);
#endif
 std::vector<uint16_t>a(count),b(count),out(w*h);std::vector<unsigned char> rgb(w*h*3);
 Renderer::Scene scene(a.data(),nullptr,w,h);scene.getRenderer()->interlacedMode=true;
 Film::scene=&scene;scene.setCamera(&Film::camera);scene.setClearBuffer(true);scene.setDirectionalLight(&Film::key);scene.setAmbientLight(&Film::ambient);
 Film::camera.setFOV(62,w);Film::camera.nearPlane=40;Film::camera.farPlane=5000;
 uint16_t background[h];for(int y=0;y<h;++y)background[y]=Film::rgb(((48+y/6)<<16)|((65+y/6)<<8)|(82+y/6));scene.backgroundGradientColors=background;
 Film::Vehicle car;car.build(false,false,true);
 for(int frame=0;frame<int(seconds*60);++frame){float t=frame/60.f;
  Film::camera.setPosition(t<8?Vector3{620,250,820}:Vector3{-620,-160,820});Film::camera.lookAt({0,70,0});
  Film::wheelPhase=0;car.pose({0,0,0},t*45,Film::clamp((t-8)/3));for(auto& part:car.parts)if(part.kind==11)part.object->enabled=false;
  auto& write=frame%2?b:a;auto& read=frame%2?a:b;scene.setFramebuffer(write.data());scene.getRenderer()->reflectBuffer=read.data();scene.render();
  for(int y=0;y<h;++y){auto& source=(y&1)?a:b;for(int x=0;x<w;++x)out[y*w+x]=source[(y/2)*(w/2)+x/2];}
  for(int i=0;i<w*h;++i){auto p=out[i];rgb[i*3]=((p>>11)&31)*255/31;rgb[i*3+1]=((p>>5)&63)*255/63;rgb[i*3+2]=(p&31)*255/31;}
  if(std::fwrite(rgb.data(),1,rgb.size(),stdout)!=rgb.size())return 1;
 }
 Film::bank.clear();return 0;
}

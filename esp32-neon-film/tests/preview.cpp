#include "Film.hpp"
#include "PerformanceOverlay.hpp"
#include "camera_checks.hpp"
#include <cassert>
#include <thread>
#include <fstream>
static void parallel(Renderer::Scene& s){std::vector<uint8_t>a(s.lastFrameDrawnTriangles),b(a.size());std::thread t([&]{s.rasterizeBand(160,320,b.data());});s.rasterizeBand(0,160,a.data());t.join();s.lastFrameRasterizedTriangles=0;for(size_t i=0;i<a.size();++i)s.lastFrameRasterizedTriangles+=!!(a[i]|b[i]);}
int main(){constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
 std::vector<uint16_t> a(count+4096,0xbeef),b(count+4096,0xbeef);Renderer::Scene scene(a.data()+2048,nullptr,w,h);scene.getRenderer()->interlacedMode=true;Film::init(scene);PerformanceOverlay hud;hud.attach(scene,w);
 auto render=[&](float time,bool threaded){Film::seek(time);checkCameraClearance(time);checkTraffic(time);checkVehicleCameras(time);checkStreetFurniture(time);std::vector<uint16_t> out(w*h);for(int parity=0;parity<4;++parity){auto& write=parity%2?b:a;auto& read=parity%2?a:b;scene.setFramebuffer(write.data()+2048);scene.getRenderer()->reflectBuffer=read.data()+2048;scene.frameCounter=parity;scene.render(threaded?parallel:nullptr);Film::effects(scene);assert(std::all_of(write.begin(),write.begin()+2048,[](auto p){return p==0xbeef;}));assert(std::all_of(write.begin()+count+2048,write.end(),[](auto p){return p==0xbeef;}));if(parity>=2)for(int y=1-(parity%2);y<h;y+=2)for(int x=0;x<w;++x)out[y*w+x]=write[2048+(y/2)*stride+x/2];}
  auto sprites=scene.getSprites();std::stable_sort(sprites.begin(),sprites.end(),[](auto* a,auto* b){return a->zOrder<b->zOrder;});for(int y=0;y<h;++y)Renderer::compositeSprites(out.data()+y*w,w,y,sprites.data(),int(sprites.size()));return out;};
 std::ofstream digests("render-digests.txt");
 std::vector<uint16_t> montage(w*h*12);float start=0;
 for(int i=0;i<11;++i){float t=start+Film::durations[i]*(i==9?.48f:.5f);auto out=render(t,true);assert(Film::shot==i);std::printf("Preview cut %d at %.2f: %d accepted triangles; %zu objects\n",i+1,t,scene.lastFrameRasterizedTriangles,scene.getObjects().size());for(int y=0;y<h;++y)std::copy(out.begin()+y*w,out.begin()+(y+1)*w,montage.begin()+((i/3)*h+y)*w*3+(i%3)*w);start+=Film::durations[i];}
 auto black=render(Film::duration,true);for(int y=80;y<290;++y)for(int x=0;x<w;++x)assert(black[y*w+x]==0);assert(start==Film::duration);
 // Crossing every cut repeatedly must release scene-owned assets and preserve HUD.
 size_t firstObjects=0,firstMaterials=0,firstSprites=0;
 for(int pass=0;pass<3;++pass){start=0;for(int i=0;i<11;++i){Film::seek(start+.1f);assert(Film::shot==i);if(i==0){assert(Film::bank.lodStorage.empty());if(pass){assert(Film::bank.objects.size()==firstObjects&&Film::bank.materials.size()==firstMaterials&&scene.getSprites().size()==firstSprites);}firstObjects=Film::bank.objects.size();firstMaterials=Film::bank.materials.size();firstSprites=scene.getSprites().size();}start+=Film::durations[i];}}
 for(float t:{16.f,27.f,36.f,43.f,51.f,60.f,71.f,85.f,96.f}){auto serial=render(t,false);assert(serial==render(t,true));}
 // Sweep the complete camera paths, including near-plane crossings between montage poses.
 for(int frame=0;frame<=2020;++frame){if(frame%100==0){std::printf("Sweep %.2f\n",frame/20.f);std::fflush(stdout);}auto pixels=render(frame/20.f,true);uint64_t hash=14695981039346656037ULL;for(auto pixel:pixels){hash^=pixel;hash*=1099511628211ULL;}digests<<frame<<" "<<hash<<"\n";}
 // Faster travel can cross a thin sign between rendered sample poses.
 for(int step=0;step<=10100;++step){float t=step/100.f;Film::seek(t);checkStreetFurniture(t);checkVehicleCameras(t);checkTraffic(t);}
 Film::seek(53+Film::turnEnd-.001f);auto beforeTurn=Film::hero.position;Film::seek(53+Film::turnEnd);auto afterTurn=Film::hero.position;assert(std::abs(beforeTurn.z-afterTurn.z)<4 && std::abs(beforeTurn.x-afterTurn.x)<4);
 Film::seek(61);Film::seek(54);auto backwards=Film::road.items.front().object->position;Film::load(6);Film::seek(54);auto replay=Film::road.items.front().object->position;assert(backwards.x==replay.x && backwards.y==replay.y && backwards.z==replay.z);
 assert(std::abs(Film::roadSpeed/Film::carLength-6.95f)<.02f);
 assert(std::abs(Film::chaseSpeed/Film::carLength-8.74f)<.02f);
 for(float t:{.2f,.5f,1.f,4.f}){
  Film::seek(88+t);auto forward=Film::hero.direction({0,0,1024});float rise=(Film::launchHeight(t+.01f)-Film::launchHeight(t-.01f))/.02f;
  float expected=std::atan2(rise,Film::chaseSpeed),actual=std::atan2(float(forward.y),float(forward.z));assert(std::abs(actual-expected)<Film::pi/180);
 }
 std::ofstream f("film.ppm",std::ios::binary);f<<"P6\n"<<w*3<<" "<<h*4<<"\n255\n";for(auto p:montage){char c[]={char(((p>>11)&31)*255/31),char(((p>>5)&63)*255/63),char((p&31)*255/31)};f.write(c,3);}
 Film::bank.clear();std::puts("Film: eleven cuts, end fade, 3 ownership cycles, field guards and serial/parallel renders pass");
}

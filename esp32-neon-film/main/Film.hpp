#pragma once
#include "World.hpp"
#include "Vehicle.hpp"
#include "City.hpp"
#include "RoadTrack.hpp"
#include "ParticleSystem.hpp"
#include <cstdio>
#if defined(ESP_PLATFORM)
#include <esp_system.h>
#include <esp_heap_caps.h>
#endif
namespace Film {
inline constexpr float durations[]={12,10,7,9,9,6,11,14,10,5,8};
inline constexpr float duration=101;
inline const char* names[]={"THE RIVER","RAIN DISTRICT","THE COURIER","REAR VIEW","NO DRIVER","EIGHTY EIGHT","BOULEVARD","PURSUIT","FLIGHT MODE","IGNITION","ABOVE IT ALL"};
inline float time=0,shotTime=0;
inline int shot=-1;
inline bool finished=false;
inline int heroGlow=-1;
inline uint16_t sky[320];
inline Vehicle hero,police[2],traffic[6];
inline ParticleSystem particles(1.2f);
inline std::vector<std::pair<Object*,Vector3>> scrolling;
inline std::vector<Material*> digits,scanner;
inline std::vector<Object*> dashboardParts;
inline Material fadeMat(0,uint8_t(255)),flashMat(0xffff,uint8_t(0)),barsMat(0,uint8_t(255));
inline Sprite2D fade,flash,topBar,bottomBar;
inline void digitsBuild(){
 // Seven-segment speed display, actual illuminated dashboard geometry.
 for(int d=0;d<2;++d){int x=-175+d*62,y=151,z=-69;
  const int rects[7][4]={{5,65,36,6},{41,36,6,28},{41,4,6,28},{5,0,36,6},{0,4,6,28},{0,36,6,28},{5,32,36,6}};
  for(auto& r:rects){auto* m=bank.paint(0xFD6836);digits.push_back(m);panel({x+r[0],y+r[1],z},{x+r[0]+r[2],y+r[1],z},{x+r[0]+r[2],y+r[1]+r[3],z},{x+r[0],y+r[1]+r[3],z},m);}
 }
}
inline void speed(int value){const int masks[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};for(int d=0;d<2;++d){int n=d==0?value/10:value%10;for(int i=0;i<7;++i)digits[d*7+i]->color=rgb(masks[n]&(1<<i)?0xFF7343:0x382025);}}
inline void cockpit(){
 const size_t first=bank.objects.size();
 box(0,121,-30,520,88,175,bank.paint(0x101B2C));
 panel({-245,140,-70},{245,140,-70},{245,242,-70},{-245,242,-70},bank.paint(0x152638));
 panel({5,151,-72},{225,151,-72},{225,230,-72},{5,230,-72},bank.texture(&dashTex));
 digitsBuild();
 for(int i=0;i<14;++i){auto* m=bank.paint(0x8A1733);scanner.push_back(m);box(-225+i*34,130,-95,26,7,9,m);}
 for(int side:{-1,1}){box(side*263,255,-36,25,150,30,bank.paint(0x24394A));box(side*234,114,-106,32,20,22,bank.paint(0x74EDC9));}
 for(int i=0;i<12;++i)box(-210+i*39,95,-130,25,12,16,bank.paint(i%3==0?0xF5B73C:i%3==1?0x45D8C2:0xF85589));
 // The cabin shares the exterior's scale: 260 wide, 160 to the roof.
 for(size_t i=first;i<bank.objects.size();++i){auto* o=bank.objects[i].get();o->bakeScale(1,2);o->position=o->position.divide(2)+Vector3{0,-20,120};o->cachePositions();dashboardParts.push_back(o);}
 auto* bonnet=bank.paint(0x647DAA);
 panel({-97,111,121},{97,111,121},{108,73,285},{-108,73,285},bonnet);
}
inline void relocate(size_t first,Vector3 origin,float yaw){for(size_t i=first;i<bank.objects.size();++i){auto* o=bank.objects[i].get();o->position=origin+yawed(o->position,yaw);o->rotation.y+=int(yaw);}for(auto& g:glows)g.position=origin+yawed(g.position,yaw);}
// Reset the fixed particle pool in place. A ParticleSystem temporary consumes
// over 7 KiB and overflows the ESP32 runtime task stack during scene creation.
inline void load(int which){
 road.clear();architecture.clear();searchlights.clear();holograms.clear();hero.parts.clear();for(auto& c:police)c.parts.clear();for(auto& c:traffic)c.parts.clear();scrolling.clear();digits.clear();scanner.clear();dashboardParts.clear();glows.clear();flicker.clear();water=nullptr;heroGlow=-1;for(auto& particle:particles.pool)particle.active=false;particles.lastRenderedTriangles=0;particles.additiveSparks=true;bank.clear();
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
 // Preserve internal RAM for live raster/transform scratch. Mesh storage can
 // live in PSRAM and is replaced as a unit at each hard cut.
 heap_caps_malloc_extmem_enable(128);
#endif
 shot=which;
 camera.setFOV(62.f,480);camera.nearPlane=40;camera.farPlane=(which>=1&&which<=9)?5200:15000;
 for(int y=0;y<320;++y)sky[y]=rgb(((10+y*15/320)<<16)|((15+y*22/320)<<8)|(34+y*40/320));
 if(shot==0){skyline();}
 else if(shot==1){street(320,7,true);}
 else if(shot==2||shot==3){street(320,12,false);road.capture(8640);hero.build();}
 else if(shot==4||shot==5){if(shot==4){street(320,12,false);road.capture(8640);}cockpit();camera.setFOV(shot==4?68.f:48.f,480);}
 else if(shot==6){
  street(320,8,false);relocate(0,{0,0,-3500},0);
  size_t first=bank.objects.size(),glowFirst=glows.size();boulevard(20);
  for(size_t i=first;i<bank.objects.size();++i){auto* o=bank.objects[i].get();o->position=Vector3{1900,0,2100}+yawed(o->position,90);o->rotation.y+=90;}
  for(size_t i=glowFirst;i<glows.size();++i)glows[i].position=Vector3{1900,0,2100}+yawed(glows[i].position,90);
  hero.build();for(int i=0;i<6;++i)traffic[i].build(false,true,false,i);
 }
 else if(shot==7){boulevard();road.capture(8640);hero.build();for(int i=0;i<6;++i)traffic[i].build(false,true,false,i);for(auto& c:police)c.build(true,true);}
 else if(shot==8||shot==9){boulevard();road.capture(8640);hero.build(false,false,true);for(auto& c:police)c.build(true,true);}
 else{cityGrid();}
 if(!hero.parts.empty()){heroGlow=int(glows.size());for(int i=0;i<6;++i)glow({0,0,0},i<2?1:2);}
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
 heap_caps_malloc_extmem_enable(CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL);
#endif
 std::fprintf(stderr,"FILM CUT %02d / %s: %u objects, %u materials\n",shot+1,names[shot],unsigned(bank.objects.size()),unsigned(bank.materials.size()));
#if defined(ESP_PLATFORM)
 std::fprintf(stderr,"Film heap: internal %u, PSRAM %u, largest internal %u\n",unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)),unsigned(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)),unsigned(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)));
#endif
}
inline void weather(float t,bool debris,Vector3 origin={0,0,0}){
 for(auto& p:particles.pool)p.active=false;
 for(int i=0;i<(debris?26:0);++i){auto& p=particles.pool[i];float a=std::fmod(t*(debris?1.8f:.8f)+i*.618f,1.f);p.active=true;p.maxLife=1;p.life=debris?.22f*(1-a):.6f;p.kind=debris?ParticleKind::Splash:ParticleKind::Spark;
  if(debris){auto local=yawed({int((i%2?1:-1)*(110+a*85)),int(8+std::sin(a*pi)*30),int(-250-a*700)},hero.heading);p.pos={float(origin.x+local.x),float(origin.y+local.y),float(origin.z+local.z)};auto velocity=yawed({(i%2?1:-1)*35,40,-520},hero.heading);p.vel={float(velocity.x),float(velocity.y),float(velocity.z)};}
  else{p.pos={float(camera.position.x+(i*347)%1300-650),float(850-a*900),float(camera.position.z+180+(i*179)%1400)};p.vel={80,-2200,0};}
 }

}
inline void pose(float p){
 Vector3 car{0,0,0};float yaw=0;const float t=shotTime;
 float wheelTravel=t*1900;
 if(shot==8){float braking=std::min(6.f,std::max(0.f,t-2));wheelTravel=1800*(std::min(t,2.f)+braking-braking*braking/12);}
 if(shot==9)wheelTravel=1800*5;
 wheelPhase=std::fmod(wheelTravel*180/(44*pi),360.f);
 if(shot==0){camera.setPosition(lerp({1700,440,-2200},{-1700,440,-2200},p));camera.lookAt(lerp({700,730,2400},{-700,730,2400},p));}
 if(shot==1){camera.setPosition(lerp({80,290,-850},{25,65,-100},p));camera.lookAt(lerp({0,170,2300},{0,65,2900},p));weather(t,false);}
 if(shot==2){road.advance(t*1900);car={0,0,0};camera.setPosition(lerp({185,800,-700},{215,700,-400},p));camera.lookAt({0,80,70});hero.pose(car,0);weather(t,true,car);}
 if(shot==3){road.advance(t*1900);car={0,0,0};float a=(-65+130*p)*pi/180;camera.setPosition({int(240*std::sin(a)),190,int(780*std::cos(a))});camera.lookAt({0,90,40});hero.pose(car,0);weather(t,true,car);}
 if(shot==4||shot==5){if(shot==4)road.advance(t*1900);
  if(shot==4){camera.setPosition(lerp({-35,136,-125},{-35,136,-110},p));camera.lookAt({-35,128,1300});speed(70);}
  else{camera.setPosition(lerp({-61,75,-30},{-61,75,9},p));camera.lookAt({-61,75,86});speed(70+int(18*clamp(t/5.f)+.5f));}
  for(size_t i=0;i<scanner.size();++i){float cursor=std::fmod(t*9,26.f);if(cursor>13)cursor=26-cursor;scanner[i]->color=rgb(std::abs(float(i)-cursor)<1.6f?0xFF403C:0x4A1727);}
 }

 if(shot==6){
  if(t<3){car={0,0,int(-3400+4850*t/3)};yaw=0;camera.setPosition(car+Vector3{200,450,-1100});}
  else if(t<5){float a=(t-3)*pi/4;car={int(650*(1-std::cos(a))),0,1450+int(650*std::sin(a))};yaw=(t-3)*45;camera.setPosition(lerp({-270,850,1600},{-270,900,2150},(t-3)/2));}
  else{float q=t-5;auto lane=cornerLane(q);car={650+int(1900*q),0,2100-int(lanePosition(q,lane))};yaw=90+steeringYaw(laneVelocity(q,lane),1900);camera.setPosition({car.x+800,260,2620});}
  camera.lookAt(car+Vector3{0,70,0});hero.pose(car,yaw);
  const int lanes[]={-380,0,380,380,-380,-380};
  for(int i=0;i<6;++i)traffic[i].pose({2500+i*1600+int(800*(t-5)),0,2100-lanes[i]},90);
  weather(t,true,car);
 }
 if(shot==7){
  road.advance(1800*t);auto lane=chaseLane(t);car={int(lanePosition(t,lane)),0,0};yaw=steeringYaw(laneVelocity(t,lane),1800);
  // Hard cuts between authored tracking rigs. Position changes are linear;
  // all rigs remain within the 1360-unit boulevard, clear of street furniture.
  if(t<4)camera.setPosition(lerp({-140,265,-900},{-120,250,-780},t/4));
  else if(t<7)camera.setPosition(lerp({555,180,820},{525,195,740},(t-4)/3));
  else if(t<10)camera.setPosition(lerp({-555,250,-670},{-525,280,-560},(t-7)/3));
  else camera.setPosition(lerp({200,1050,-650},{290,1200,-320},(t-10)/4));
  camera.lookAt(car+Vector3{0,75,0});hero.pose(car,yaw);
  const int starts[]={2300,6900,11500,5000,9500,15100},lanes[]={-380,0,380,380,-380,-380};
  for(int i=0;i<6;++i)traffic[i].pose({lanes[i],0,starts[i]-int(1150*t)},0);
  for(int i=0;i<2;++i){float delayed=t-(i?1.6f:.85f);auto chase=chaseLane(delayed);police[i].pose({int(lanePosition(delayed,chase))+(i?0:-40),0,i?-1750:-900},steeringYaw(laneVelocity(delayed,chase),1800),0,0,t);}
  weather(t,true,car);
 }
 if(shot==8){road.advance(t*1800);car={0,int(55*clamp((t-2)/6)),0};float h=clamp((t-2)/6);camera.setPosition(lerp({-510,145,-650},{-600,180,-700},p));camera.lookAt(car+Vector3{0,85,-50});hero.pose(car,0,h);for(int i=0;i<2;++i)police[i].pose({i?380:-380,0,-1000-i*500},0,0,0,t);}
 if(shot==9){road.advance(0);car={0,110+int(2900*clamp((t-.6f)/4.4f)),int(700*p)};camera.setPosition(lerp({-500,240,-1050},{-550,1200,-1200},p));camera.lookAt(lerp({0,100,0},{0,2700,500},p));hero.pose(car,0,1,1);for(int i=0;i<2;++i)police[i].pose({i?380:-380,0,-1000-i*500},0,0,0,.1f);weather(t,true,car);}
 if(shot==10){camera.setPosition(lerp({-1300,4100,-2300},{1100,5800,-3200},p));camera.lookAt({0,0,2200});}
 animateCity(t);
 for(size_t i=0;i<flicker.size();++i){float v=std::fmod(t*11+i*3.7f,13.f);flicker[i]->alpha=uint8_t(v<.45f?70:255);}
 if(water){auto v=camera.transformDirection(Vector3{0,0,1900}-camera.position);int32_t cx,sx,cy,sy,cz,sz;camera.getRotationMatrix(cx,sx,cy,sy,cz,sz);int horizon=160+int(sx*camera.fovFactor/1024.f);float shore=160-v.y*camera.fovFactor/v.z;water->waterYBias=uint8_t(std::clamp(int(2*(shore-horizon)),0,255));water->waterReflectionMaxY=int16_t(shore);scene->waterTime=t;}
 if(heroGlow>=0){glows[heroGlow].position=hero.position+yawed({-78,57,290},hero.heading);glows[heroGlow+1].position=hero.position+yawed({78,57,290},hero.heading);for(int i=0;i<4;++i)glows[heroGlow+2+i].position=hero.position+yawed({i%2?150:-150,35,i<2?-175:175},hero.heading);}
 projectGlows();
 if(shot==4||shot==5)for(auto& g:glows)g.sprite->enabled=g.sprite->enabled && shot==4 && g.sprite->y<160;
 if(heroGlow>=0){
  auto facing=yawed({0,0,1024},hero.heading);auto view=camera.position-hero.position;
  bool front=int64_t(facing.x)*view.x+int64_t(facing.z)*view.z>0;
  for(int i=0;i<2;++i)glows[heroGlow+i].sprite->enabled=glows[heroGlow+i].sprite->enabled && front;
  for(int i=0;i<4;++i){auto* halo=glows[heroGlow+2+i].sprite;halo->enabled=halo->enabled && (shot==9||(shot==8&&t>2));halo->material->alpha=shot==9?220:uint8_t(180*clamp((t-2)/6));}
 }
}
inline void seek(float absolute){
 time=absolute;int next=0;float local=absolute;while(next<10&&local>=durations[next]){local-=durations[next];++next;}shotTime=std::min(local,durations[next]);
 if(next!=shot)load(next);
 pose(clamp(shotTime/durations[shot]));
 int alpha=shot==0?int(255*(1-clamp(shotTime/3))):shot==10?int(255*clamp((shotTime-5)/3)):0;
 setSolidRectAlpha(fade,uint8_t(alpha));float burst=shot==9?std::max(0.f,1-std::abs(shotTime-.55f)/.3f):0;setSolidRectAlpha(flash,uint8_t(burst*245));

}
inline void update(float dt){time+=dt;if(time>=duration+1){finished=true;
#if defined(ESP_PLATFORM)
 std::fprintf(stderr,"FILM COMPLETE / REBOOT\n");esp_restart();
#else
 seek(duration);return;
#endif
 }seek(time);}
inline unsigned effects(Scene& target){
 particles.render(&target,&camera,480,320);unsigned count=particles.lastRenderedTriangles;
 if(shot!=1)return count;
 // Brief 1/250 s rain exposure: narrow dim streaks, not broad spark wedges.
 auto* raster=target.getRenderer();const bool parity=(target.frameCounter-1)%2==0;
 Material rain(rgb(0x91B4CC));rain.emissive=true;rain.shadingMode=ShadingMode::UNLIT;rain.alpha=42;
 for(int i=0;i<92;++i){float age=std::fmod(shotTime*4.5f+i*.6180339f,1.f);Vector3 world{(i*193)%570-285,int(1100-age*1200),camera.position.z+200+(i*137)%1700};
  auto a=camera.transformDirection(world-camera.position),b=camera.transformDirection(world+Vector3{1,-22,0}-camera.position);
  if(a.z<=80||b.z<=80)continue;
  int x=240+int(a.x*camera.fovFactor/a.z),y=160-int(a.y*camera.fovFactor/a.z),tx=240+int(b.x*camera.fovFactor/b.z),ty=160-int(b.y*camera.fovFactor/b.z);
  RenderVertex v0,v1,v2;v0.position={x,y,a.z};v1.position={x+1,y,a.z};v2.position={tx,ty,b.z};
  if(raster->drawTriangle(v0,v1,v2,&rain,nullptr,nullptr,parity,false,true,0,255))++count;
 }
 return count;
}
inline void init(Scene& target){scene=&target;scene->setCamera(&camera);scene->setClearBuffer(true);scene->setDirectionalLight(&key);scene->setAmbientLight(&ambient);scene->backgroundGradientColors=sky;
 fade=makeFullScreenFade(480,320,0,&fadeMat);fade.zOrder=900000;scene->addSprite(&fade);
 flash=makeFullScreenFade(480,320,0xffff,&flashMat);flash.zOrder=900001;scene->addSprite(&flash);
 topBar=makeSolidRect(0,0,480,12,&barsMat);bottomBar=makeSolidRect(0,308,480,12,&barsMat);topBar.zOrder=bottomBar.zOrder=800000;scene->addSprite(&topBar);scene->addSprite(&bottomBar);
 seek(0);
}
}

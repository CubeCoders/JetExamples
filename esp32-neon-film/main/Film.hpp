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
inline uint16_t sky[renderHeight];
inline Vehicle hero,police[2],traffic[6];
inline ParticleSystem particles(1.2f);
inline std::vector<std::pair<Object*,Vector3>> scrolling;
inline std::vector<Material*> digits,scanner;
inline std::vector<Object*> dashboardParts;
inline Material fadeMat(0,uint8_t(255)),barsMat(0,uint8_t(255));
inline Sprite2D fade,topBar,bottomBar;
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
 // The gauge insert needs only its instrument binnacle. The surrounding
 // cabin is built for the wider driver view.
 if(shot==4){
 // Enclose the eye position with a real interior shell. The sloping
 // pillars and roof header frame the road; instruments sit inside this cabin.
 auto* roof=bank.paint(0x101928);auto* trim=bank.paint(0x344357);auto* bevel=bank.paint(0x586985);auto* cyan=bank.paint(0x399CBA);
 panel({-170,175,-230},{170,175,-230},{145,175,-20},{-145,175,-20},roof);
 panel({-145,175,-20},{145,175,-20},{102,157,-15},{-102,157,-15},trim);
 panel({-102,157,-15},{102,157,-15},{102,160,-16},{-102,160,-16},cyan);
 for(int side:{-1,1}){
  panel({side*88,162,-20},{side*108,169,-20},{side*150,78,115},{side*128,88,115},trim);
  panel({side*86,161,-19},{side*89,162,-19},{side*129,89,114},{side*126,88,114},cyan);
  // Door top, inner door card, armrest and a lit control recess.
  panel({side*155,82,-220},{side*155,82,115},{side*128,92,115},{side*128,92,-220},bevel);
  panel({side*155,0,-220},{side*155,0,120},{side*128,88,120},{side*128,88,-220},roof);
  panel({side*150,72,-125},{side*121,76,-125},{side*121,76,40},{side*150,72,40},trim);
  panel({side*129,93,-135},{side*129,93,110},{side*129,95,110},{side*129,95,-135},cyan);
 }
 // Faceted console flows from the dashboard towards the seats. It is empty:
 // autonomy is indicated by the scanner and the physical display, not a driver.
 panel({-30,48,-160},{30,48,-160},{43,90,65},{-43,90,65},roof);
 panel({-30,48,-160},{-43,90,65},{-48,85,65},{-37,43,-160},trim);
 panel({30,48,-160},{43,90,65},{48,85,65},{37,43,-160},trim);
 auto* amber=bank.paint(0xE7A54D);
 for(int i=0;i<5;++i){int z=10+i*14,y=76+i*4;panel({-21,y,z},{21,y,z},{21,y+2,z+9},{-21,y+2,z+9},i%2?cyan:amber);}
 // Small squared-off yoke, deliberately unattended, in front of the left seat.
 auto* yoke=bank.paint(0x4D607A);
 panel({-73,85,-35},{-17,85,-35},{-17,95,-27},{-73,95,-27},yoke);
 panel({-74,85,-35},{-65,90,-32},{-66,120,-23},{-79,120,-23},yoke);
 panel({-25,90,-32},{-16,85,-35},{-11,120,-23},{-24,120,-23},yoke);
 }
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
 shot=which;prepareFarFacades();scene->lodScale=(which>=1&&which<=9)?3200:0;
 // Fixed lenses per cut: wider inside the narrow lane, tighter on the wheel transformation.
 constexpr float lenses[]={62,68,62,74,74,48,62,62,56,64,68};
 camera.setFOV(lenses[which],renderWidth);camera.nearPlane=40;camera.farPlane=(which>=1&&which<=9)?9000:15000;
 for(int y=0;y<renderHeight;++y)sky[y]=rgb(((10+y*15/renderHeight)<<16)|((15+y*22/renderHeight)<<8)|(34+y*40/renderHeight));
 if(shot==0){skyline();}
 else if(shot==1){street(320,14,true,7);}
 else if(shot==2||shot==3){street(320,16,false);road.capture(11520);hero.build();}
 else if(shot==4||shot==5){if(shot==4){street(320,16,false);road.capture(11520);}cockpit();}
 else if(shot==6){
  street(320,8,false);relocate(0,{0,0,-3500},0);
  size_t first=bank.objects.size(),glowFirst=glows.size();boulevard(28);
  for(size_t i=first;i<bank.objects.size();++i){auto* o=bank.objects[i].get();o->position=Vector3{1900,0,2100}+yawed(o->position,90);o->rotation.y+=90;}
  for(size_t i=glowFirst;i<glows.size();++i)glows[i].position=Vector3{1900,0,2100}+yawed(glows[i].position,90);
  hero.build();for(int i=0;i<6;++i)traffic[i].build(false,true,false,i);
 }
 else if(shot==7){boulevard(16);road.capture(11520);hero.build();for(int i=0;i<6;++i)traffic[i].build(false,true,false,i);for(auto& c:police)c.build(true,true);}
 else if(shot==8||shot==9){boulevard(16);road.capture(11520);hero.build(false,false,true);for(auto& c:police)c.build(true,true);}
 else{cityGrid();relocate(0,{0,0,-3400},20);hero.build(false,false,true);}
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
 if(shot>=9)wheelTravel=1800*5;
 wheelPhase=std::fmod(wheelTravel*180/(44*pi),360.f);
 if(shot==0){camera.setPosition(lerp({2300,410,-1900},{-1100,460,-2300},p));camera.lookAt(lerp({-300,700,2400},{600,760,2400},p));}
 if(shot==1){camera.setPosition(lerp({-210,290,-900},{-150,65,-150},p));camera.lookAt(lerp({130,165,900},{170,70,1500},p));weather(t,false);}
 if(shot==2){road.advance(t*1900);car={0,0,0};camera.setPosition(lerp({260,700,-590},{245,590,-530},p));camera.lookAt({-25,65,-45});hero.pose(car,0);weather(t,true,car);}
 if(shot==3){road.advance(t*1900,true);car={45,0,0};float a=(-35+11*p)*pi/180;camera.setPosition(car+Vector3{int(520*std::sin(a)),int(145+20*p),int(520*std::cos(a))});camera.lookAt(car+lerp({35,95,-20},{20,100,-35},p));hero.pose(car,0);weather(t,true,car);}
 if(shot==4||shot==5){if(shot==4)road.advance(t*1900);
  if(shot==4){camera.setPosition(lerp({-46,132,-145},{-36,130,-130},p));camera.lookAt(lerp({50,112,560},{90,110,620},p));speed(70);}
  else{camera.setPosition(lerp({-90,92,-25},{-82,88,8},p));camera.lookAt({-61,75,86});speed(70+int(18*clamp(t/5.f)+.5f));}
  for(size_t i=0;i<scanner.size();++i){float cursor=std::fmod(t*9,26.f);if(cursor>13)cursor=26-cursor;scanner[i]->color=rgb(std::abs(float(i)-cursor)<1.6f?0xFF403C:0x4A1727);}
 }

 if(shot==6){
  if(t<3){car={0,0,int(-3400+4850*t/3)};yaw=0;camera.setPosition(car+Vector3{-230,280,-680});}
  else if(t<5){float a=(t-3)*pi/4;car={int(650*(1-std::cos(a))),0,1450+int(650*std::sin(a))};yaw=(t-3)*45;camera.setPosition(lerp({-270,650,1600},{-270,760,2150},(t-3)/2));}
  else{float q=t-5;auto lane=cornerLane(q);car={650+int(1900*q),0,2100-int(lanePosition(q,lane))};yaw=90+steeringYaw(laneVelocity(q,lane),1900);camera.setPosition({car.x+800,260,2620});}
  camera.lookAt(car+(t<3?Vector3{80,100,160}:t<5?Vector3{60,100,100}:Vector3{120,100,-80}));hero.pose(car,yaw);
  const int lanes[]={-380,0,380,380,-380,-380};
  for(int i=0;i<6;++i)traffic[i].pose({2500+i*1600+int(800*(t-5)),0,2100-lanes[i]},90);
  weather(t,true,car);
 }
 if(shot==7){
  road.advance(1800*t,t>=4&&t<7);auto lane=chaseLane(t);car={int(lanePosition(t,lane)),0,0};yaw=steeringYaw(laneVelocity(t,lane),1800);
  // Hard cuts between authored tracking rigs. Position changes are linear;
  // all rigs remain within the 1360-unit boulevard, clear of street furniture.
  if(t<4)camera.setPosition(lerp({240,280,-850},{280,265,-720},t/4));
  else if(t<7)camera.setPosition(lerp({-515,245,700},{-475,235,590},(t-4)/3));
  else if(t<10)camera.setPosition(lerp({-555,250,-580},{-525,270,-500},(t-7)/3));
  else camera.setPosition(lerp({500,850,-450},{425,1100,-250},(t-10)/4));
  camera.lookAt(car+(t<4?Vector3{-65,105,150}:t<7?Vector3{50,100,-80}:t<10?Vector3{40,95,90}:Vector3{-60,60,100}));hero.pose(car,yaw);
  const int starts[]={2300,6900,11500,5000,9500,15100},lanes[]={-380,0,380,380,-380,-380};
  for(int i=0;i<6;++i)traffic[i].pose({lanes[i],0,starts[i]-int(1150*t)},0);
  for(int i=0;i<2;++i){float delayed=t-(i?1.6f:.85f);auto chase=chaseLane(delayed);police[i].pose({int(lanePosition(delayed,chase))+(i?0:-40),0,i?-1750:-900},steeringYaw(laneVelocity(delayed,chase),1800),0,0,t);}
  weather(t,true,car);
 }
 if(shot==8){road.advance(t*1800);car={0,int(55*clamp((t-2)/6)),0};float h=clamp((t-2)/6);camera.setPosition(lerp({-510,145,-650},{-570,165,-570},p));camera.lookAt(car+Vector3{50,90,20});hero.pose(car,0,h);for(int i=0;i<2;++i)police[i].pose({i?380:-380,0,-1000-i*500},0,0,0,t);}
 // Maintain the street's 1800-unit/s forward speed through the climb.
 // The tracking rig translates linearly with the car while the city stops.
 if(shot==9){road.advance(0);car={0,55+int(650*t),int(1800*t)};camera.setPosition({-650,260+int(580*t),1000+int(1800*t)});camera.lookAt(car+Vector3{40,60,0});hero.pose(car,0,1,1);for(int i=0;i<2;++i)police[i].pose({i?380:-380,0,-1000-i*500},0,0,0,.1f);}
 // A stationary camera is ahead of the flight path. The same velocity carries
 // the coupe up past the lens; hold on the city after it leaves, then fade.
 if(shot==10){car={0,3305+int(650*t),-3600+int(1800*t)};camera.setPosition({240,5250,1800});camera.lookAt(lerp({0,3305,-3600},{0,900,-3600},p));hero.pose(car,0,1,1);}
 animateCity(t);
 for(size_t i=0;i<flicker.size();++i){float v=std::fmod(t*11+i*3.7f,13.f);flicker[i]->alpha=uint8_t(v<.45f?70:255);}
 if(water){auto v=camera.transformDirection(Vector3{0,0,1900}-camera.position);int32_t cx,sx,cy,sy,cz,sz;camera.getRotationMatrix(cx,sx,cy,sy,cz,sz);int horizon=renderHeight/2+int(sx*camera.fovFactor/1024.f);float shore=renderHeight/2-v.y*camera.fovFactor/v.z;water->waterYBias=uint8_t(std::clamp(int(2*(shore-horizon)),0,255));water->waterReflectionMaxY=int16_t(shore);scene->waterTime=t;}
 if(heroGlow>=0){glows[heroGlow].position=hero.position+yawed({-78,57,290},hero.heading);glows[heroGlow+1].position=hero.position+yawed({78,57,290},hero.heading);for(int i=0;i<4;++i)glows[heroGlow+2+i].position=hero.position+yawed({i%2?150:-150,35,i<2?-175:175},hero.heading);}
 projectGlows();
 if(shot==4||shot==5)for(auto& g:glows)g.sprite->enabled=g.sprite->enabled && shot==4 && g.sprite->y<renderHeight/2;
 if(heroGlow>=0){
  auto facing=yawed({0,0,1024},hero.heading);auto view=camera.position-hero.position;
  bool front=int64_t(facing.x)*view.x+int64_t(facing.z)*view.z>0;
  for(int i=0;i<2;++i)glows[heroGlow+i].sprite->enabled=glows[heroGlow+i].sprite->enabled && front;
  for(int i=0;i<4;++i){auto* halo=glows[heroGlow+2+i].sprite;halo->enabled=halo->enabled && (shot>=9||(shot==8&&t>2));halo->material->alpha=shot>=9?220:uint8_t(180*clamp((t-2)/6));}
 }
}
inline void seek(float absolute){
 time=absolute;int next=0;float local=absolute;while(next<10&&local>=durations[next]){local-=durations[next];++next;}shotTime=std::min(local,durations[next]);
 if(next!=shot)load(next);
 pose(clamp(shotTime/durations[shot]));
 int alpha=shot==0?int(255*(1-clamp(shotTime/3))):shot==10?int(255*clamp((shotTime-5)/3)):0;
 setSolidRectAlpha(fade,uint8_t(alpha));

}
inline void update(float dt){time+=dt;if(time>=duration+1){finished=true;
#if defined(ESP_PLATFORM)
 std::fprintf(stderr,"FILM COMPLETE / REBOOT\n");esp_restart();
#else
 seek(duration);return;
#endif
 }seek(time);}
inline unsigned effects(Scene& target){
 particles.render(&target,&camera,renderWidth,renderHeight);unsigned count=particles.lastRenderedTriangles;
 if(shot!=1)return count;
 // Brief 1/250 s rain exposure: narrow dim streaks, not broad spark wedges.
 auto* raster=target.getRenderer();const bool parity=raster->interlacedMode && (target.frameCounter-1)%2==0;
 Material rain(rgb(0x91B4CC));rain.emissive=true;rain.shadingMode=ShadingMode::UNLIT;rain.alpha=42;
 for(int i=0;i<92;++i){float age=std::fmod(shotTime*4.5f+i*.6180339f,1.f);Vector3 world{(i*193)%570-285,int(1100-age*1200),camera.position.z+200+(i*137)%1700};
  auto a=camera.transformDirection(world-camera.position),b=camera.transformDirection(world+Vector3{1,-22,0}-camera.position);
  if(a.z<=80||b.z<=80)continue;
  int x=renderWidth/2+int(a.x*camera.fovFactor/a.z),y=renderHeight/2-int(a.y*camera.fovFactor/a.z),tx=renderWidth/2+int(b.x*camera.fovFactor/b.z),ty=renderHeight/2-int(b.y*camera.fovFactor/b.z);
  RenderVertex v0,v1,v2;v0.position={x,y,a.z};v1.position={x+renderScale,y,a.z};v2.position={tx,ty,b.z};
  if(raster->drawTriangle(v0,v1,v2,&rain,nullptr,nullptr,parity,false,true,0,255))++count;
 }
 return count;
}
inline void init(Scene& target){scene=&target;scene->setCamera(&camera);scene->setClearBuffer(true);scene->setDirectionalLight(&key);scene->setAmbientLight(&ambient);scene->backgroundGradientColors=sky;
 fade=makeFullScreenFade(renderWidth,renderHeight,0,&fadeMat);fade.zOrder=900000;scene->addSprite(&fade);
 topBar=makeSolidRect(0,0,renderWidth,12*renderScale,&barsMat);bottomBar=makeSolidRect(0,308*renderScale,renderWidth,12*renderScale,&barsMat);topBar.zOrder=bottomBar.zOrder=800000;scene->addSprite(&topBar);scene->addSprite(&bottomBar);
 seek(0);
}
}

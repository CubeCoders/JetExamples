#pragma once
#include "World.hpp"
#include "Vehicle.hpp"
#include "City.hpp"
#include "RoadTrack.hpp"
#include "ParticleSystem.hpp"
#include "CreditMask.hpp"
#include <cstdio>
#if defined(ESP_PLATFORM)
#include <esp_system.h>
#include <esp_heap_caps.h>
#endif
namespace Film {
inline constexpr float durations[]={12,10,7,9,9,6,11,14,10,5,8,15};
inline constexpr float duration=116;
inline constexpr int cutCount=sizeof(durations)/sizeof(durations[0]);
inline constexpr float creditsStart=2,creditsFadeIn=2,creditsHold=8,creditsFadeOut=3;
// Linear lens pulls accent the gauge, launch and final approach. This changes
// projection without adding a depth-of-field buffer or moving the camera rig.
inline float shotLens(int cut,float local){
 if(cut==5)return 62-16*clamp(local/5.f);
 if(cut==9)return 54+16*clamp(local/5.f);
 if(cut==10)return 34+34*clamp(local/2.f);
 constexpr float lenses[]={62,68,62,74,74,48,62,62,56,64,68,62};
 return lenses[cut];
}
inline const char* names[]={"THE RIVER","RAIN DISTRICT","THE COURIER","REAR VIEW","NO DRIVER","EIGHTY EIGHT","BOULEVARD","PURSUIT","FLIGHT MODE","IGNITION","ABOVE IT ALL","ESP 88"};
inline float time=0,shotTime=0;
inline int shot=-1;
inline bool finished=false;
inline int heroGlow=-1;
inline uint16_t sky[renderHeight];
inline Vehicle hero,police[2],traffic[6];
inline ParticleSystem particles(1.2f);
inline std::vector<std::pair<Object*,Vector3>> scrolling;
inline std::vector<Vector3> approachGlows;
inline std::vector<Material*> scanner;
inline Material fadeMat(0,uint8_t(255)),barsMat(0,uint8_t(255));
inline Sprite2D fade,topBar,bottomBar;
inline std::vector<uint16_t> creditPixels;
inline Texture creditTexture(360*renderScale,168*renderScale,nullptr,true,0);
inline Sprite2D* creditSprite=nullptr;
inline Sprite2D* creditDim=nullptr;
inline void credits(){
 creditPixels.assign(creditTexture.width*creditTexture.height,0);
 for(size_t i=0;i<creditPixels.size();++i)if(creditMask[i/8]&(0x80>>(i%8)))creditPixels[i]=0xffff;
 creditTexture.data=creditPixels.data();
 creditDim=bank.sprite(bank.paint(0,0),0,0,810000);creditDim->width=renderWidth;creditDim->height=renderHeight;
 creditSprite=bank.sprite(bank.texture(&creditTexture,0),60*renderScale,68*renderScale,820000);
}
inline std::vector<uint16_t> speedPixels;
inline Texture speedTexture(128,64,nullptr);
inline int displayedSpeed=-1;
inline void speed(int value){
 if(value==displayedSpeed)return;
 displayedSpeed=value;
 std::fill(speedPixels.begin(),speedPixels.end(),rgb(0x091420));
 const int masks[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
 const int rects[7][4]={{5,46,32,5},{37,26,5,24},{37,3,5,22},{5,0,32,5},{0,3,5,22},{0,26,5,24},{5,23,32,5}};
 for(int d=0;d<2;++d)for(int i=0;i<7;++i){auto& r=rects[i];int digit=d?value%10:value/10;auto c=rgb(masks[digit]&(1<<i)?0xFF7343:0x241B21);
  for(int y=0;y<r[3];++y)for(int x=0;x<r[2];++x)speedPixels[(6+r[1]+y)*128+8+d*56+r[0]+x]=c;
 }
}
inline void cockpit(){
 auto* dark=bank.paint(0x101B2C);auto* trim=bank.paint(0x344357);auto* cyan=bank.paint(0x399CBA);
 // One shallow, continuous dashboard. The two screens replace sections of
 // its face rather than floating above overlapping boxes or control panels.
 auto face=[&](int x0,int y0,int x1,int y1,Material* m){return panel({x0,y0,100},{x1,y0,100},{x1,y1,100},{x0,y1,100},m);};
 face(-128,38,128,51,dark);face(-128,93,128,100,trim);
 face(-128,51,-117,93,trim);face(-19,51,1,93,trim);face(117,51,128,93,trim);
 face(-117,51,-19,54,dark);face(-117,90,-19,93,dark);
 face(1,51,117,54,dark);face(1,90,117,93,dark);
 speedPixels.assign(128*64,rgb(0x091420));speedTexture.data=speedPixels.data();displayedSpeed=-1;
 face(-117,54,-19,90,bank.texture(&speedTexture));face(1,54,117,90,bank.texture(&dashTex));
 panel({-128,100,100},{128,100,100},{119,105,137},{-119,105,137},dark);
 // A single flush KITT-like scanner, with real gaps between its segments.
 for(int i=0;i<14;++i){auto* m=bank.paint(0x8A1733);scanner.push_back(m);face(-111+i*16,96,-99+i*16,98,m);}
 if(shot==4){
  auto* roof=bank.paint(0x101928);
  panel({-156,183,-230},{156,183,-230},{137,175,-25},{-137,175,-25},roof);
  panel({-137,175,-25},{137,175,-25},{111,167,-12},{-111,167,-12},trim);
  for(int side:{-1,1}){
   panel({side*106,167,-12},{side*121,171,-12},{side*141,87,115},{side*128,94,115},trim);
   panel({side*105,167,-11},{side*108,168,-11},{side*130,94,114},{side*128,94,114},cyan);
   panel({side*150,0,-220},{side*150,0,115},{side*131,84,115},{side*131,84,-220},roof);
   panel({side*150,80,-220},{side*150,80,115},{side*131,87,115},{side*131,87,-220},trim);
   panel({side*131,87,-200},{side*131,87,100},{side*131,89,100},{side*131,89,-200},cyan);
  }
 }
 // No centre tower or yoke: leave a clear view of both displays and the road.
 auto* bonnet=bank.paint(0x647DAA);
 panel({-97,111,121},{97,111,121},{108,73,285},{-108,73,285},bonnet);
}
inline void relocate(size_t first,Vector3 origin,float yaw){for(size_t i=first;i<bank.objects.size();++i){auto* o=bank.objects[i].get();o->position=origin+yawed(o->position,yaw);o->rotation.y+=int(yaw);}for(auto& g:glows)g.position=origin+yawed(g.position,yaw);}
// Reset the fixed particle pool in place. A ParticleSystem temporary consumes
// over 7 KiB and overflows the ESP32 runtime task stack during scene creation.
inline void load(int which){
 creditSprite=nullptr;creditDim=nullptr;std::vector<uint16_t>().swap(creditPixels);creditTexture.data=nullptr;neonStrokes.clear();road.clear();architecture.clear();searchlights.clear();holograms.clear();hero.parts.clear();for(auto& c:police)c.parts.clear();for(auto& c:traffic)c.parts.clear();scrolling.clear();approachGlows.clear();std::vector<uint16_t>().swap(speedPixels);speedTexture.data=nullptr;scanner.clear();glows.clear();flicker.clear();water=nullptr;heroGlow=-1;for(auto& particle:particles.pool)particle.active=false;particles.lastRenderedTriangles=0;particles.additiveSparks=true;bank.clear();
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
 // Preserve internal RAM for live raster/transform scratch. Mesh storage can
 // live in PSRAM and is replaced as a unit at each hard cut.
 heap_caps_malloc_extmem_enable(128);
#endif
 shot=which;prepareFarFacades();scene->lodScale=(which>=1&&which<=9)?3200:0;
 camera.setFOV(shotLens(which,0),renderWidth);camera.nearPlane=40;camera.farPlane=(which>=1&&which<=9)?9000:15000;
 for(int y=0;y<renderHeight;++y)sky[y]=rgb(((10+y*15/renderHeight)<<16)|((15+y*22/renderHeight)<<8)|(34+y*40/renderHeight));
 if(shot==0||shot==11){skyline();if(shot==11)credits();}
 else if(shot==1){street(320,14,true,7);}
 else if(shot==2||shot==3){street(320,16,false);road.capture(11520);hero.build();}
 else if(shot==4||shot==5){if(shot==4){street(320,16,false);road.capture(11520);}cockpit();}
 else if(shot==6){
  street(320,15,false);relocate(0,{0,0,-8500},0);
  // Continue the metropolis behind the junction's reverse-angle skid shot.
  for(int i=0;i<3;++i)building(-1300-i*1200,3300,850,820,1900+i*210,i,false);
  for(int i=0;i<2;++i)building(-1700-i*1300,750,850,800,1750+i*240,i+2,false);
  for(auto& o:bank.objects)scrolling.push_back({o.get(),o->position});
  for(auto& g:glows)approachGlows.push_back(g.position);
  size_t first=bank.objects.size(),glowFirst=glows.size();boulevard(16);
  for(size_t i=first;i<bank.objects.size();++i){auto* o=bank.objects[i].get();o->position=Vector3{1900,0,2100}+yawed(o->position,90);o->rotation.y+=90;}
  for(size_t i=glowFirst;i<glows.size();++i)glows[i].position=Vector3{1900,0,2100}+yawed(glows[i].position,90);
  road.capture(11520,first,glowFirst,true);hero.build();for(int i=0;i<6;++i)traffic[i].build(false,true,false,i);for(auto& c:police)c.build(true,true);
 }
 else if(shot==7){boulevard(16);road.capture(11520);hero.build();for(int i=0;i<6;++i)traffic[i].build(false,true,false,i);for(auto& c:police)c.build(true,true);}
 else if(shot==8||shot==9){boulevard(16);road.capture(11520);hero.build(false,false,true);for(auto& c:police)c.build(true,true);}
 else{cityGrid();relocate(0,{0,0,-3400},20);hero.build(false,false,true);}
 prepareNeon();
 if(!hero.parts.empty()){heroGlow=int(glows.size());for(int i=0;i<6;++i)glow({0,0,0},i<2?1:2);}
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
 heap_caps_malloc_extmem_enable(CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL);
#endif
 std::fprintf(stderr,"FILM CUT %02d / %s: %u objects, %u materials\n",shot+1,names[shot],unsigned(bank.objects.size()),unsigned(bank.materials.size()));
#if defined(ESP_PLATFORM)
 std::fprintf(stderr,"Film heap: internal %u, PSRAM %u, largest internal %u\n",unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)),unsigned(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)),unsigned(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)));
#endif
}
// Independent hashes avoid the diagonal lattice produced by modular X/Z steps.
inline uint32_t weatherHash(uint32_t v){v^=v>>16;v*=0x7feb352du;v^=v>>15;v*=0x846ca68bu;return v^(v>>16);}
inline void weather(float t,bool debris,Vector3 origin={0,0,0}){
 for(auto& p:particles.pool)p.active=false;
 for(int i=0;i<(debris?26:34);++i){auto& p=particles.pool[i];float a=std::fmod(t*(debris?1.8f:2.6f)+i*.618f,1.f);p.active=true;p.maxLife=debris?1:.22f;p.life=.22f*(1-a);p.kind=ParticleKind::Splash;
  if(debris){auto local=yawed({int((i%2?1:-1)*(110+a*85)),int(8+std::sin(a*pi)*30),int(-250-a*700)},hero.heading);p.pos={float(origin.x+local.x),float(origin.y+local.y),float(origin.z+local.z)};auto velocity=yawed({(i%2?1:-1)*35,40,-520},hero.heading);p.vel={float(velocity.x),float(velocity.y),float(velocity.z)};}
  else{uint32_t cycle=uint32_t(std::floor(t*2.6f+i*.618f)),seed=uint32_t(i)*0x9e3779b9u+cycle*0x85ebca6bu;
   p.pos={float(int(weatherHash(seed)%520)-260),float(3+std::sin(a*pi)*10),float(camera.position.z+140+int(weatherHash(seed^0xa3c59ac3u)%1200))};p.vel={float(i%2?55:-55),150,0};}
 }

}
inline void pose(float p){
 Vector3 car{0,0,0};float yaw=0;const float t=shotTime;
 const float lens=shotLens(shot,t);if(camera.fov!=lens)camera.setFOV(lens,renderWidth);
 float wheelTravel=t*(shot<6?roadSpeed:chaseSpeed);
 if(shot==6)wheelTravel=approachTravel(std::min(t,turnStart))+turnRadius*pi/2*clamp((t-turnStart)/(turnEnd-turnStart))+exitTravel(std::max(0.f,t-turnEnd));
 if(shot==8){float braking=std::min(6.f,std::max(0.f,t-2));wheelTravel=chaseSpeed*(std::min(t,2.f)+braking-braking*braking/12);}
 if(shot>=9)wheelTravel=chaseSpeed*5;
 wheelPhase=std::fmod(wheelTravel*180/(44*pi),360.f);
 if(shot==0||shot==11){camera.setPosition(lerp({2300,410,-1900},{-1100,460,-2300},p));camera.lookAt(lerp({-300,700,2400},{600,760,2400},p));}
 if(shot==1){camera.setPosition(lerp({-175,270,-900},{-140,65,-150},p));camera.lookAt(lerp({130,165,900},{170,70,1500},p));weather(t,false);}
 if(shot==2){road.advance(t*roadSpeed);car={0,0,0};camera.setPosition(lerp({140,660,-590},{120,600,-530},p));camera.lookAt({-25,65,-45});hero.pose(car,0);weather(t,true,car);}
 if(shot==3){road.advance(t*roadSpeed,true);car={45,0,0};float a=(-30+12*p)*pi/180;camera.setPosition(car+Vector3{int(520*std::sin(a)),int(145+20*p),int(520*std::cos(a))});camera.lookAt(car+lerp({35,95,-20},{20,100,-35},p));hero.pose(car,0);weather(t,true,car);}
 if(shot==4||shot==5){if(shot==4)road.advance(t*roadSpeed);
  if(shot==4){camera.setPosition(lerp({-45,133,-150},{-35,131,-135},p));camera.lookAt(lerp({35,115,560},{65,114,620},p));speed(70);}
  else{camera.setPosition(lerp({-88,79,-24},{-82,77,-6},p));camera.lookAt({-72,73,100});speed(70+int(18*clamp(t/5.f)+.5f));}
  for(size_t i=0;i<scanner.size();++i){float cursor=std::fmod(t*9,26.f);if(cursor>13)cursor=26-cursor;scanner[i]->color=rgb(std::abs(float(i)-cursor)<1.6f?0xFF403C:0x4A1727);}
 }

 if(shot==6){
  float q=std::max(0.f,t-turnEnd),travel=exitTravel(q),pitch=0,lean=0;
  float rebase=t>=exitTrackingStart?travel:0;int roadTravel=int(rebase);constexpr float trafficSpeed=chaseSpeed-1100;
  for(auto& o:scrolling)o.first->position=o.second+Vector3{-roadTravel,0,0};
  for(size_t i=0;i<approachGlows.size();++i)glows[i].position=approachGlows[i]+Vector3{-roadTravel,0,0};
  if(t<exitTrackingStart)road.restore();else road.advance(travel,true);
  if(t<turnStart){
   car={0,0,1450+int(approachTravel(t)-approachTravel(turnStart))};
   pitch=-6*clamp((t-brakeStart)/.18f)*(1-clamp((t-1.65f)/.6f));
   // Braking rig stays ahead and inside the narrow street; linear camera
   // travel lets the decelerating coupe grow into the frame.
   float startZ=1450+approachTravel(0)-approachTravel(turnStart);
   camera.setPosition(lerp({-175,240,int(startZ)+2800},{-175,360,3100},t/turnStart));
  }else if(t<turnEnd){
   float u=(t-turnStart)/(turnEnd-turnStart),a=u*pi/2;
   car={int(turnRadius*(1-std::cos(a))),0,1450+int(turnRadius*std::sin(a))};
   yaw=u*90+42*std::sin(u*pi);pitch=-3*(1-u);lean=5*std::sin(u*pi);
   // Cut to a low front-quarter camera across the junction as the rear
   // steps out. The body slides sideways while the tyres stay on the road.
   camera.setPosition(lerp({880,160,2590},{1860,270,2590},u));
  }else{
   float overtaking=std::max(0.f,(travel-trafficSpeed*q)/1100);auto lane=cornerLane(overtaking);
   car={int(turnRadius+travel-rebase),0,2100-int(lanePosition(overtaking,lane))};
   float lateral=laneVelocity(overtaking,lane)*std::max(0.f,(exitSpeed(q)-trafficSpeed)/1100);
   yaw=90+steeringYaw(lateral,exitSpeed(q));pitch=3*std::sin(pi*clamp(q/exitAccelerationSeconds));
   lean=-2*std::sin(pi*clamp(q/.7f));
   if(t<exitTrackingStart)camera.setPosition({1860+int(40*q),270+int(12*q),2590+int(15*q)});
   else camera.setPosition({int(turnRadius)+800,260,2540});
  }
  // Keep the skid camera looking down the street as the accelerating car
  // passes underneath. Only then cut to the rebased moving tracking rig.
  if(t>=turnEnd && t<exitTrackingStart)camera.lookAt({int(turnRadius),75,2480});
  else camera.lookAt(car+(t<turnStart?Vector3{0,80,0}:t<turnEnd?Vector3{0,75,0}:Vector3{100,100,-70}));
  hero.pose(car,yaw,0,0,0,pitch,lean);
  const int lanes[]={0,-380,380,380,-380,-380};
  float trafficTravel=trafficSpeed*(t-turnEnd)-rebase;
  for(int i=0;i<6;++i)traffic[i].pose({int(turnRadius)+6000+i*1800+int(trafficTravel),0,2100-lanes[i]},90);
  // Establish distant flashing cars and let them close the gap before the
  // pursuit cut. Their relative approach is linear and continuous at the cut.
  float catchup=clamp((t-exitTrackingStart)/(durations[6]-exitTrackingStart));
  for(int i=0;i<2;++i){float gap=(i?6500.f:5200.f)+(i?-4100.f:-3600.f)*catchup;police[i].pose({int(turnRadius-gap),0,2480+(i?0:40)},90,0,0,t);if(t<exitTrackingStart)for(auto& part:police[i].parts)part.object->enabled=false;}
  weather(t,true,car);
 }
 if(shot==7){
  road.advance(chaseSpeed*t,t<2||(t>=4&&t<7));auto lane=chaseLane(t);car={int(lanePosition(t,lane)),0,0};yaw=steeringYaw(laneVelocity(t,lane),chaseSpeed);
  // Hard cuts between authored tracking rigs. Position changes are linear;
  // all rigs remain within the 1360-unit boulevard, clear of street furniture.
  if(t<2)camera.setPosition(lerp({440,460,-3500},{400,440,-3000},t/2));
  else if(t<4)camera.setPosition(lerp({240,280,-850},{280,265,-720},(t-2)/2));
  else if(t<7)camera.setPosition(lerp({-515,245,700},{-475,235,590},(t-4)/3));
  else if(t<10)camera.setPosition(lerp({-555,250,-580},{-525,270,-500},(t-7)/3));
  else camera.setPosition(lerp({500,850,-450},{425,1100,-250},(t-10)/4));
  camera.lookAt(car+(t<2?Vector3{0,80,-500}:t<4?Vector3{-65,105,150}:t<7?Vector3{50,100,-80}:t<10?Vector3{40,95,90}:Vector3{-60,60,100}));hero.pose(car,yaw);
  const int starts[]={2300,6900,11500,5000,9500,15100},lanes[]={-380,0,380,380,-380,-380};
  for(int i=0;i<6;++i)traffic[i].pose({lanes[i],0,starts[i]-int(1150*t)},0);
  for(int i=0;i<2;++i){float delayed=t-(i?1.6f:.85f);auto chase=chaseLane(delayed);police[i].pose({int(lanePosition(delayed,chase))+(i?0:-40),0,int(i?-2400+650*clamp(t/4):-1600+700*clamp(t/4))},steeringYaw(laneVelocity(delayed,chase),chaseSpeed),0,0,t);}
  weather(t,true,car);
 }
 if(shot==8){road.advance(t*chaseSpeed);car={0,int(55*clamp((t-2)/6)),0};float h=clamp((t-2)/6);camera.setPosition(lerp({-510,145,-650},{-570,165,-570},p));camera.lookAt(car+Vector3{50,90,20});hero.pose(car,0,h);for(int i=0;i<2;++i)police[i].pose({i?380:-380,0,-1000-i*500},0,0,0,t);}
 // Rebase the moving shot around the car. Road parallax still represents
 // the full forward velocity; police remain at fixed world positions below.
 if(shot==9){float travel=chaseSpeed*t;road.advance(travel,true);car={0,int(launchHeight(t)),0};camera.setPosition({-480,260+int(580*t),1000});camera.lookAt(car+Vector3{40,60,0});hero.pose(car,0,1,1,0,launchPitch(t));for(int i=0;i<2;++i)police[i].pose({i?380:-380,0,-1000-i*500-int(travel)},0,0,0,.1f);}
 // Identical velocity and pitch across the final cut. The camera waits two
 // seconds ahead of the flight path and clears the outboard wheel pods.
 // This establishing cut is staged 1500 units lower, just above the rooftops.
 if(shot==10){float startY=launchHeight(durations[9])-1500;car={0,int(startY+climbSpeed*t),int(chaseSpeed*(t-2))};camera.setPosition({260,int(startY+climbSpeed*2),0});camera.lookAt(lerp({0,int(startY),int(-chaseSpeed*2)},{0,0,-2000},p));hero.pose(car,0,1,1,0,launchPitch(durations[9]));}
 animateCity(t);animateNeon();
 for(size_t i=0;i<flicker.size();++i){float v=std::fmod(t*11+i*3.7f,13.f);flicker[i]->alpha=uint8_t(v<.45f?70:255);}
 if(water){auto v=camera.transformDirection(Vector3{0,0,1900}-camera.position);int32_t cx,sx,cy,sy,cz,sz;camera.getRotationMatrix(cx,sx,cy,sy,cz,sz);int horizon=renderHeight/2+int(sx*camera.fovFactor/1024.f);float shore=renderHeight/2-v.y*camera.fovFactor/v.z;water->waterYBias=uint8_t(std::clamp(int(2*(shore-horizon)),0,255));water->waterReflectionMaxY=int16_t(shore);scene->waterTime=t;}
 if(heroGlow>=0){glows[heroGlow].position=hero.worldPoint({-78,57,290});glows[heroGlow+1].position=hero.worldPoint({78,57,290});for(int i=0;i<4;++i)glows[heroGlow+2+i].position=hero.worldPoint({i%2?150:-150,35,i<2?-175:175});}
 projectGlows();
 if(shot==4||shot==5)for(auto& g:glows)g.sprite->enabled=g.sprite->enabled && shot==4 && g.sprite->y<renderHeight/2;
 if(heroGlow>=0){
  auto facing=hero.direction({0,0,1024});auto view=camera.position-hero.position;
  bool front=int64_t(facing.x)*view.x+int64_t(facing.y)*view.y+int64_t(facing.z)*view.z>0;
  for(int i=0;i<2;++i)glows[heroGlow+i].sprite->enabled=glows[heroGlow+i].sprite->enabled && front;
  for(int i=0;i<4;++i){auto* halo=glows[heroGlow+2+i].sprite;halo->enabled=halo->enabled && (shot>=9||(shot==8&&t>2));halo->material->alpha=shot>=9?220:uint8_t(180*clamp((t-2)/6));}
 }
}
inline void seek(float absolute){
 time=absolute;int next=0;float local=absolute;while(next<cutCount-1&&local>=durations[next]){local-=durations[next];++next;}shotTime=std::min(local,durations[next]);
 if(next!=shot)load(next);
 pose(clamp(shotTime/durations[shot]));
 int alpha=shot==0?int(255*(1-clamp(shotTime/3))):shot==10?int(255*clamp((shotTime-5)/3)):0;
 if(shot==11)alpha=int(255*std::max(1-clamp(shotTime/2),clamp((shotTime-creditsStart-creditsFadeIn-creditsHold)/creditsFadeOut)));
 setSolidRectAlpha(fade,uint8_t(alpha));
 if(creditSprite){int reveal=int(255*clamp((shotTime-creditsStart)/creditsFadeIn));setSolidRectAlpha(*creditSprite,uint8_t(reveal));setSolidRectAlpha(*creditDim,uint8_t(reveal*90/255));}

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
 if(shot==4||shot==5)return count;
 // Fine fast rain: one packed pixel wide on S3, with enough contrast to
 // survive the LCD and video encoding. Keep screen-space width proportional.
 auto* raster=target.getRenderer();const bool parity=raster->interlacedMode && (target.frameCounter-1)%2==0;
 Material rain(rgb(0x91B4CC));rain.emissive=true;rain.shadingMode=ShadingMode::UNLIT;rain.alpha=125;
 // A camera-centred rain volume covers reverse views and aerial shots too.
 // Drops travel along world gravity transformed into view space. Independent
 // births avoid a fixed screen grid; no persistent particle allocation is needed.
 auto fall=camera.transformDirection({3,-54,0});
 for(int i=0;i<220;++i){float phase=shotTime*4.5f+i*.6180339f,age=phase-std::floor(phase);uint32_t seed=uint32_t(i)*0x9e3779b9u+uint32_t(std::floor(phase))*0x85ebca6bu;
  float z=200+weatherHash(seed)%1800;
  float px=float(weatherHash(seed^0xc2b2ae35u)%uint32_t(renderWidth+80*renderScale))-renderWidth/2-40*renderScale;
  float py=float(weatherHash(seed^0x27d4eb2fu)%uint32_t(renderHeight+80*renderScale))-renderHeight/2-40*renderScale;
  float travel=(age-.5f)*1200/54;
  Vector3 a{int(px*z/camera.fovFactor+fall.x*travel),int(py*z/camera.fovFactor+fall.y*travel),int(z+fall.z*travel)},b=a+fall;
  if(a.z<=80||b.z<=80)continue;
  int x=renderWidth/2+int(a.x*camera.fovFactor/a.z),y=renderHeight/2-int(a.y*camera.fovFactor/a.z),tx=renderWidth/2+int(b.x*camera.fovFactor/b.z),ty=renderHeight/2-int(b.y*camera.fovFactor/b.z);
  RenderVertex v0,v1,v2;v0.position={x,y,a.z};v1.position={x+2*renderScale,y,a.z};v2.position={tx,ty,b.z};
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

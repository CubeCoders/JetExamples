#pragma once
#include "Scene.hpp"
#include "ParticleSystem.hpp"
#include "Primitives.hpp"
#include "Display.hpp"
#include "Labels.hpp"
#include <cmath>
#include <cstdio>
namespace ParticleDemo {
using namespace Renderer;
inline Scene* scene=nullptr;
inline Camera camera;
inline ParticleSystem particles(1.f);
inline float time=0,accumulator=0,sparkClock=0,splashClock=0;
inline int mode=-1;
inline unsigned peakLive=0;
inline uint16_t gradient[Display::RENDER_HEIGHT];
inline Material *leftLight=nullptr,*rightLight=nullptr;
inline uint16_t rgb(unsigned c){return uint16_t(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));}
inline Material* paint(unsigned c){auto* m=new Material(rgb(c));m->shadingMode=ShadingMode::UNLIT;return m;}
inline void put(Object* o) {o->calculateBoundingBox();o->cachePositions();scene->addObject(o);}
inline void box(int x,int y,int z,int w,int h,int d,Material* m) {auto* o=Primitives::createCube(w,h,d,m);o->setPosition(x,y,z);put(o);}
inline void select(int next) {
 if(next==mode)return;
 mode=next;sparkClock=splashClock=0;
 for(auto& p:particles.pool)p.active=false;
 const char* names[]={"ADDITIVE SPARKS","WATER SPRAY","POOL LIMIT / 200","DISTANCE CULL"};
 Labels::select(mode);std::printf("Particles: %s\n",names[mode]);
}
inline void step(float dt) {
 time=std::fmod(time+dt,28.f);select(int(time/7));particles.update(dt);
 sparkClock+=dt;splashClock+=dt;
 const bool spark=mode!=1,water=mode!=0;
 if(spark&&sparkClock>=.55f) {
  sparkClock-=.55f;
  // A request above capacity is safely capped by the same 200-slot pool.
  particles.emitSparks({-175,-140,0},{.25f,1,0},1100.f,mode==2?260:52,{0,320,0});
 }
 if(water&&splashClock>=.035f) {
  splashClock-=.035f;const float a=time*4;
  particles.emitWaterSplash({175,-140,0},{0,1,0},{std::cos(a),0,std::sin(a)},{0,1,0},1300,6,{850*std::cos(a),1500,500*std::sin(a)});
 }
 // Keep disabled emitter clocks bounded, so mode changes have no catch-up burst.
 if(!spark)sparkClock=0;
 if(!water)splashClock=0;
 peakLive=std::max(peakLive,particles.activeCount());
}
inline void update(float dt) {
 // Stable emission/physics at 120 Hz, with bounded catch-up after a long pause.
 accumulator+=std::min(std::max(dt,0.f),.1f);
 constexpr float tick=1.f/120.f;
 while(accumulator>=tick){step(tick);accumulator-=tick;}
 const float phase=std::fmod(time,7.f);
 const float pullback=mode==3?1100.f*(.5f-.5f*std::cos(phase*6.2831853f/7.f)):0;
 camera.setPosition(0,0,-int(850+pullback));
 leftLight->color=rgb(mode==1?0x344354:0x66E9FF);rightLight->color=rgb(mode==0?0x344354:0xEDB5FF);
 Labels::counts(particles.activeCount(),particles.lastRenderedTriangles);
}
inline unsigned renderEffects(Scene& target) {
 particles.render(&target,&camera,Display::RENDER_WIDTH,Display::RENDER_HEIGHT);
 Labels::counts(particles.activeCount(),particles.lastRenderedTriangles);
 return particles.lastRenderedTriangles;
}
inline void init(Scene& target) {
 scene=&target;camera.setFOV(58.f,Display::RENDER_WIDTH);camera.nearPlane=40;camera.farPlane=4000;
 scene->setCamera(&camera);scene->setClearBuffer(true);particles.additiveSparks=true;
 for(int y=0;y<Display::RENDER_HEIGHT;++y)gradient[y]=rgb(((12+y*13/320)<<16)|((21+y*22/320)<<8)|(39+y*28/320));
 scene->backgroundGradientColors=gradient;
 auto* wall=paint(0x152A43);auto* floorA=paint(0x253E53);auto* floorB=paint(0x1C3044);
 auto* metal=paint(0x426679);auto* inset=paint(0x223B4F);leftLight=paint(0x66E9FF);rightLight=paint(0xEDB5FF);
 box(0,0,290,1080,490,20,wall);
 for(int x=-500;x<=500;x+=100)box(x,0,270,3,440,4,paint(0x28485C));
 for(int y=-200;y<=200;y+=100)box(0,y,268,1020,3,4,paint(0x28485C));
 auto* floor=Primitives::createGrid(980,600,4,6,floorA,floorB);floor->setPosition(0,-220,80);put(floor);
 for(int x:{-175,175}) {
  box(x,-198,0,170,44,160,inset);box(x,-170,0,130,18,120,metal);
  box(x,-155,0,86,12,78,x<0?leftLight:rightLight);
 }
 Labels::add(target);select(0);update(0);
 std::printf("Particle pool: %d slots, %u bytes; fixed 120 Hz update; additive sparks / alpha droplets\n",PARTICLE_POOL_SIZE,unsigned(sizeof(particles.pool)));
}
}

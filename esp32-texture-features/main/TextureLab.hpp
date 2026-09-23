#pragma once
#include "Scene.hpp"
#include "Primitives.hpp"
#include "Display.hpp"
#include "Assets.hpp"
#include "Labels.hpp"
#include <cmath>

namespace TextureLab {
using namespace Renderer;
inline constexpr float pi=3.14159265359f,stageSeconds=7.f,cycleSeconds=49.f;
inline Scene* scene=nullptr;
inline Camera camera;
inline float time=0;
inline int activeStage=-1;
inline uint16_t background[Display::RENDER_HEIGHT];
inline uint16_t rgb(unsigned hex) {return uint16_t(((hex>>19)&31)<<11|((hex>>10)&63)<<5|((hex>>3)&31));}
inline Texture tile(64,64,const_cast<uint16_t*>(Assets::tile));
inline Texture checker(32,32,const_cast<uint16_t*>(Assets::checker));
inline Texture foliageOpaque(96,128,const_cast<uint16_t*>(Assets::foliage),false,0xf81f,false,CLAMP);
inline Texture foliageKeyed(96,128,const_cast<uint16_t*>(Assets::foliage),true,0xf81f,false,CLAMP);
inline Texture lava(64,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::lavaIndices)),false,0,false,WRAP,const_cast<uint16_t*>(Assets::lavaPalette));
inline Material panelMaterial(rgb(0x27677D),&tile),backMaterial(rgb(0x1C384C),&checker);
inline Material opaqueMaterial(0xffff,&foliageOpaque),keyedMaterial(0xffff,&foliageKeyed);
inline Object *panel=nullptr,*back=nullptr,*plants[2]={};
inline Material* paint(unsigned hex) {auto* m=new Material(rgb(hex));m->shadingMode=ShadingMode::UNLIT;return m;}
inline void put(Object* o) {o->noWriteZBuffer=true;o->calculateBoundingBox();o->cachePositions();scene->addObject(o);}
inline Object* plane(int width,int height,int x,int y,int z,Material* material) {
 auto* o=new Object;o->cullingMode=CullingMode::NO_CULLING;
 o->addVertex({{-width/2,-height/2,0},{0,1024},{0,0,-1024}});
 o->addVertex({{ width/2,-height/2,0},{1024,1024},{0,0,-1024}});
 o->addVertex({{ width/2, height/2,0},{1024,0},{0,0,-1024}});
 o->addVertex({{-width/2, height/2,0},{0,0},{0,0,-1024}});
 o->addFace(0,1,2,3,material);o->setPosition(x,y,z);put(o);return o;
}
inline void uvRect(Object* o,int u,int v,int width,int height) {
 o->vertices[0].uv={u,v+height};o->vertices[1].uv={u+width,v+height};
 o->vertices[2].uv={u+width,v};o->vertices[3].uv={u,v};
}
inline void box(int x,int y,int z,int w,int h,int d,Material* material) {
 auto* o=Primitives::createCube(w,h,d,material);o->setPosition(x,y,z);put(o);
}
inline void update(float dt) {
 time=std::fmod(time+dt,cycleSeconds);
 const int stage=int(time/stageSeconds);const float phase=time-stage*stageSeconds;
 if(stage!=activeStage) {
  activeStage=stage;
  constexpr const char* names[]={"WRAP","CLAMP","ZERO + BLACK KEY","COLOUR KEY","PALETTE CYCLING","LOD OFF","LOD ON"};
  std::printf("Texture Lab: %s\n",names[stage]);
 }
 auto* raster=scene->getRenderer();raster->textureLodEnabled=stage==6;
 panel->enabled=stage!=3;plants[0]->enabled=plants[1]->enabled=stage==3;
 backMaterial.diffuseMap=stage>=5?nullptr:&checker;
 panelMaterial.diffuseMap=stage==4?&lava:&tile;
 tile.addressMode=stage==0?WRAP:stage==2?ZERO:CLAMP;
 tile.hasAlpha=stage==2;tile.alphaColor=0;
 panel->setPosition(0,0,1000);
 int lodState=0;
 if(stage<=2) {
  // Identical UV animation for each addressing mode; scrolling reaches beyond
  // all four image edges. Geometry stays front-facing, so affine UVs are exact.
  uvRect(panel,int(-600+350*std::sin(phase*.9f)),int(-350+220*std::cos(phase*.8f)),2304,1536);
 } else if(stage==4) {
  uvRect(panel,0,0,2048,1024);
  // Absolute time avoids frame-rate-dependent rounding in tiny dt increments.
  lava.paletteOffset=int(phase*18.f)%64;
 } else {
  uvRect(panel,0,0,1024,1024);
  if(stage>=5) {
   const int distance=int(850+1050*(.5f-.5f*std::cos(phase*2*pi/stageSeconds)));
   panel->setPosition(0,0,distance);
   lodState=distance>=raster->textureLodFar?2:distance>raster->textureLodNear?1:0;
  }
 }
 Labels::select(stage,lodState);
}
inline void init(Scene& target) {
 scene=&target;camera.setFOV(58.f,Display::RENDER_WIDTH);camera.nearPlane=40;camera.farPlane=3500;
 scene->setCamera(&camera);scene->setClearBuffer(true);
 for(int y=0;y<Display::RENDER_HEIGHT;++y) {
  const unsigned red=12+y*14/Display::RENDER_HEIGHT,green=29+y*31/Display::RENDER_HEIGHT,blue=48+y*32/Display::RENDER_HEIGHT;
  background[y]=uint16_t((red>>3)<<11|(green>>2)<<5|(blue>>3));
 }
 scene->backgroundGradientColors=background;
 for(auto* m:{&panelMaterial,&backMaterial,&opaqueMaterial,&keyedMaterial})m->shadingMode=ShadingMode::UNLIT;
 lava.paletteSize=64;
 // A simple unlit 3D console frames the panels; no Z allocation is needed.
 auto* dark=paint(0x172B40);auto* edge=paint(0x426879);auto* cyan=paint(0x33DEC8);auto* pink=paint(0xEB639E);
 box(0,-245,1170,900,85,290,dark);
 plane(810,440,0,0,1100,edge);
 plane(780,410,0,0,1080,dark);
 back=plane(750,380,0,0,1060,&backMaterial);uvRect(back,0,0,4096,2048);
 plane(16,380,-400,0,1040,cyan);plane(16,380,400,0,1040,pink);
 // These panels are submitted last so keyed holes reveal the checker behind.
 panel=plane(720,360,0,0,1000,&panelMaterial);panel->noWriteZBuffer=false;
 plants[0]=plane(270,350,-182,0,960,&opaqueMaterial);
 plants[1]=plane(270,350,182,0,960,&keyedMaterial);
 plants[0]->noWriteZBuffer=plants[1]->noWriteZBuffer=false;
 auto* raster=scene->getRenderer();raster->textureLodNear=1100;raster->textureLodFar=1650;
 Labels::add(*scene);update(0);
}
}

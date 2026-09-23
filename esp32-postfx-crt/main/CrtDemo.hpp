#pragma once
#include "Scene.hpp"
#include "Primitives.hpp"
#include "Display.hpp"
#include "Assets.hpp"
#include "Labels.hpp"
#include <cmath>
#include <cstdio>
namespace CrtDemo {
using namespace Renderer;
inline Scene* scene=nullptr;
inline Camera camera;
inline Object* cube=nullptr;
inline float time=0;
inline int mode=-1;
inline uint16_t gradient[Display::RENDER_HEIGHT];
inline Texture backdrop(256,128,const_cast<uint16_t*>(Assets::backdrop));
inline Texture tiles[]={Texture(32,32,const_cast<uint16_t*>(Assets::cyan)),Texture(32,32,const_cast<uint16_t*>(Assets::pink)),Texture(32,32,const_cast<uint16_t*>(Assets::gold))};
inline Material poster(0xffff,&backdrop),faces[]={Material(0xffff,&tiles[0]),Material(0xffff,&tiles[1]),Material(0xffff,&tiles[2])};
inline uint16_t rgb(unsigned c) {return uint16_t(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));}
inline Material* paint(unsigned c) {auto* m=new Material(rgb(c));m->shadingMode=ShadingMode::UNLIT;return m;}
inline Object* plane(int w,int h,int x,int y,int z,Material* m) {
 auto* o=new Object;o->cullingMode=CullingMode::NO_CULLING;
 o->addVertex({{-w/2,-h/2,0},{0,1024},{0,0,-1024}});o->addVertex({{w/2,-h/2,0},{1024,1024},{0,0,-1024}});
 o->addVertex({{w/2,h/2,0},{1024,0},{0,0,-1024}});o->addVertex({{-w/2,h/2,0},{0,0},{0,0,-1024}});
 o->addFace(0,1,2,3,m);o->setPosition(x,y,z);o->noWriteZBuffer=true;
 o->calculateBoundingBox();o->cachePositions();scene->addObject(o);return o;
}
inline void update(float dt) {
 time=std::fmod(time+dt,14.f);const int next=int(time/7.f);const float phase=std::fmod(time,7.f);
 if(next!=mode) {mode=next;std::printf("CRT: %s; intensity 112/255; no extra image buffer\n",mode?"ON":"OFF");}
 scene->crtEnabled=mode==1;scene->crtIntensity=112;Labels::select(mode);
 // Exactly one revolution per seven seconds; the same path repeats in both modes.
 cube->setRotation(int(20+14*std::sin(phase*6.2831853f/7)),int(phase*360.f/7),int(9*std::sin(phase*12.566371f/7)));
}
inline void init(Scene& target) {
 scene=&target;camera.setFOV(58.f,Display::RENDER_WIDTH);camera.nearPlane=40;camera.farPlane=2500;
 scene->setCamera(&camera);scene->setClearBuffer(true);
 for(int y=0;y<Display::RENDER_HEIGHT;++y)gradient[y]=rgb(((18+y*12/320)<<16)|((23+y*20/320)<<8)|(49+y*29/320));
 scene->backgroundGradientColors=gradient;
 poster.shadingMode=ShadingMode::UNLIT;for(auto& m:faces)m.shadingMode=ShadingMode::UNLIT;
 plane(1090,555,0,0,1350,paint(0x6B719A));
 plane(1060,530,0,0,1335,paint(0x1B213F));
 plane(1024,512,0,0,1320,&poster);
 plane(14,490,-543,0,1310,paint(0x33F1DE));plane(14,490,543,0,1310,paint(0xFF659D));
 cube=Primitives::createCube(240,240,240,&faces[0]);
 for(unsigned i=0;i<cube->triangles.size();++i)cube->triangles[i].material=&faces[(i/2)%3];
 cube->setPosition(190,10,870);cube->cachePositions();scene->addObject(cube);
 Labels::add(target);update(0);
}
}

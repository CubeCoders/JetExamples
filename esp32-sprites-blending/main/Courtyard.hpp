#pragma once
#include "Scene.hpp"
#include "Primitives.hpp"
#include "Display.hpp"
#include "Assets.hpp"
#include "Labels.hpp"
#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>
namespace Courtyard {
using namespace Renderer;
inline constexpr float pi=3.14159265359f;
inline Scene* scene=nullptr;
inline Camera camera;
inline float time=0;
inline int stage=-1;
inline std::vector<Object*> subjects,mirrors,lightMeshes;
inline Object *jewel=nullptr,*jewelMirror=nullptr;
inline Material *floorMaterial=nullptr;
inline uint16_t sky[Display::RENDER_HEIGHT];
inline Texture signTexture(128,48,const_cast<uint16_t*>(Assets::sign));
inline Texture warmGlow(16,16,const_cast<uint16_t*>(Assets::warmGlow),true,0),coolGlow(16,16,const_cast<uint16_t*>(Assets::coolGlow),true,0);
inline Material haloMaterials[]={Material(0xffff,&warmGlow),Material(0xffff,&coolGlow)};
inline Sprite2D halos[2];
inline const Vector3 lampHeads[]={{-330,432,110},{330,432,180}};
inline uint16_t rgb(unsigned c){return uint16_t(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));}
inline Material* paint(unsigned c,int alpha=255,bool add=false) {
 auto* m=new Material(rgb(c));m->alpha=uint8_t(alpha);m->shadingMode=add?ShadingMode::ADDITIVE:ShadingMode::UNLIT;return m;
}
inline void finish(Object* o){o->calculateBoundingBox();o->cachePositions();}
inline void quad(Object* o,Vector3 a,Vector3 b,Vector3 c,Vector3 d,Material* m) {
 const int n=int(o->vertices.size());o->addVertex({a,{0,1024},{0,1024,0}});o->addVertex({b,{1024,1024},{0,1024,0}});
 o->addVertex({c,{1024,0},{0,1024,0}});o->addVertex({d,{0,0},{0,1024,0}});o->addFace(n,n+1,n+2,n+3,m);
}
inline Object* box(int x,int y,int z,int w,int h,int d,Material* m) {
 auto* o=Primitives::createCube(w,h,d,m);o->setPosition(x,y,z);finish(o);subjects.push_back(o);return o;
}
inline Object* mirrorOf(Object* source) {
 auto* o=new Object(*source);o->invalidatePositions();
 for(auto& v:o->vertices){v.position.y=-v.position.y;v.normal.y=-v.normal.y;}
 for(auto& t:o->triangles)std::swap(t.v2,t.v3);
 o->setPosition(source->position.x,-source->position.y,source->position.z);
 o->setRotation(-source->rotation.x,source->rotation.y,-source->rotation.z);
 o->noWriteZBuffer=true;finish(o);mirrors.push_back(o);return o;
}
inline Object* floorQuad(int x0,int z0,int x1,int z1,int y,Material* m) {
 auto* o=new Object;o->cullingMode=CullingMode::NO_CULLING;
 quad(o,{x0,y,z0},{x1,y,z0},{x1,y,z1},{x0,y,z1},m);o->noWriteZBuffer=true;finish(o);scene->addObject(o);return o;
}
inline void crystal(int x,int y,int z,int size,bool main=false) {
 auto* o=new Object;const Vector3 points[]={{0,size,0},{size*2/3,0,0},{0,0,size*2/3},{-size*2/3,0,0},{0,0,-size*2/3},{0,-size,0}};
 for(auto p:points)o->addVertex({p,{0,0},{0,1024,0}});
 const unsigned colours[]={0xFC92BE,0xA54C98,0x61E5CF,0x36758B,0xF3BF98,0x824E88,0x39A5AE,0xB86FA9};
 for(int i=0;i<4;++i){int a=1+i,b=1+(i+1)%4;o->addTriangle(0,b,a,paint(colours[i]));o->addTriangle(5,a,b,paint(colours[i+4]));}
 o->setPosition(x,y,z);finish(o);subjects.push_back(o);if(main)jewel=o;
}
inline void plant(int x,int z) {
 box(x,30,z,72,60,72,paint(0x7F5578));box(x,63,z,80,9,80,paint(0xD3939F));
 auto* leaves=new Object;leaves->cullingMode=CullingMode::NO_CULLING;
 for(int i=0;i<7;++i) {
  float a=i*2*pi/7;const Vector3 base{x,65,z},tip{x+int(85*std::cos(a)),110+(i%3)*45,z+int(85*std::sin(a))};
  Vector3 left{x+int(28*std::cos(a-.5f)),120,z+int(28*std::sin(a-.5f))},right{x+int(28*std::cos(a+.5f)),120,z+int(28*std::sin(a+.5f))};
  quad(leaves,base,left,tip,right,paint(i%2?0x438F89:0x68BB9E));
 }
 finish(leaves);subjects.push_back(leaves);
}
inline void pool(Vector3 head,unsigned colour) {
 // Nested additive discs give a soft pool without a texture or real light.
 for(int ring=0;ring<5;++ring) {
  const int radius=185-ring*30;auto* o=new Object;o->cullingMode=CullingMode::NO_CULLING;
  auto* m=paint(colour,7+ring*3,true);o->addVertex({{head.x,2,head.z},{0,0},{0,1024,0}});
  for(int i=0;i<16;++i){float a=i*2*pi/16;o->addVertex({{head.x+int(radius*std::cos(a)),2,head.z+int(radius*std::sin(a))},{0,0},{0,1024,0}});}
  for(int i=0;i<16;++i)o->addTriangle(0,1+i,1+(i+1)%16,m);
  o->noWriteZBuffer=true;finish(o);scene->addObject(o);lightMeshes.push_back(o);
 }
 // Thin translucent cone shells suggest dust catching the lamp beam.
 for(int ring=0;ring<2;++ring) {
  const int radius=160-ring*45;auto* o=new Object;o->cullingMode=CullingMode::NO_CULLING;auto* m=paint(colour,ring?4:5,true);
  o->addVertex({{head.x,head.y-8,head.z},{0,0},{0,0,-1024}});
  for(int i=0;i<12;++i){float a=i*2*pi/12;o->addVertex({{head.x+int(radius*std::cos(a)),4,head.z+int(radius*std::sin(a))},{0,0},{0,0,-1024}});}
  for(int i=0;i<12;++i)o->addTriangle(0,1+i,1+(i+1)%12,m);
  finish(o);scene->addObject(o);lightMeshes.push_back(o);
 }
}
inline void select(int next) {
 if(next==stage)return;
 stage=next;const bool reflection=stage!=1,lights=stage==0||stage>=3,glow=stage==0||stage==4;
 for(auto* o:mirrors)o->enabled=reflection;
 for(auto* o:lightMeshes)o->enabled=lights;
 floorMaterial->alpha=reflection?158:255;
 for(auto& s:halos)s.enabled=glow;
 Labels::select(stage);
 const char* names[]={"AFTER HOURS / ALL LAYERS","UNLIT GEOMETRY","MIRRORS + ALPHA FLOOR","ADDITIVE LIGHT MESHES","SPRITE HALOS"};
 std::printf("Courtyard: %s\n",names[stage]);
}
inline void update(float dt) {
 time=std::fmod(time+dt,42.f);select(time<14?0:1+int((time-14)/7));
 const float a=time*2*pi/14;
 // A closer, wider sweep with a gentle rise and fall reveals the floor trick.
 camera.setPosition(int(360*std::sin(a)),480+int(65*std::sin(a+.6f)), -1100+int(90*std::cos(a)));camera.lookAt({0,135,90});
 const int yaw=int(time*360/14)%360,pitch=int(10*std::sin(a));jewel->setRotation(pitch,yaw,0);jewelMirror->setRotation(-pitch,yaw,0);
 for(int i=0;i<2;++i) {
  const auto view=camera.transformDirection(lampHeads[i]-camera.position);
  halos[i].x=240+int(view.x*camera.fovFactor/view.z)-32;halos[i].y=160-int(view.y*camera.fovFactor/view.z)-32;
 }
}
inline void init(Scene& target) {
 scene=&target;camera.setFOV(58.f,Display::RENDER_WIDTH);camera.nearPlane=40;camera.farPlane=6000;scene->setCamera(&camera);scene->setClearBuffer(true);
 for(int y=0;y<Display::RENDER_HEIGHT;++y)sky[y]=rgb(((17+y*10/320)<<16)|((25+y*25/320)<<8)|(48+y*26/320));
 scene->backgroundGradientColors=sky;
 // Skyline is behind the courtyard and deliberately excluded from reflection.
 auto* skyline=new Object;skyline->cullingMode=CullingMode::NO_CULLING;
 for(int i=0;i<9;++i){int x=-1050+i*250;quad(skyline,{x,0,1100},{x+200,0,1100},{x+200,260+(i%3)*110,1100},{x,260+(i%3)*110,1100},paint(i%2?0x24324E:0x2C3B57));}
 skyline->noWriteZBuffer=true;finish(skyline);
 auto* moon=new Object;moon->cullingMode=CullingMode::NO_CULLING;moon->addVertex({{-950,200,1300},{0,0},{0,0,-1024}});
 for(int i=0;i<24;++i){float a=i*2*pi/24;moon->addVertex({{-950+int(92*std::cos(a)),200+int(92*std::sin(a)),1300},{0,0},{0,0,-1024}});}
 auto* moonPaint=paint(0xC3C6CB);for(int i=0;i<24;++i)moon->addTriangle(0,1+i,1+(i+1)%24,moonPaint);moon->noWriteZBuffer=true;finish(moon);scene->addObject(moon);scene->addObject(skyline);
 // Low cafe wall and luminous world-space sign.
 box(0,135,490,680,270,45,paint(0x3A405A));box(0,277,480,710,18,65,paint(0x64868D));
 for(int x:{-260,260}){box(x,145,461,88,190,8,paint(0x243348));box(x,145,454,7,180,5,paint(0x6BBDB9));}
 auto* sign=new Object;sign->cullingMode=CullingMode::NO_CULLING;auto* signMat=new Material(0xffff,&signTexture);signMat->shadingMode=ShadingMode::UNLIT;
 quad(sign,{-190,82,460},{190,82,460},{190,224,460},{-190,224,460},signMat);finish(sign);subjects.push_back(sign);
 box(0,26,40,190,52,160,paint(0x657E8F));box(0,57,40,160,10,134,paint(0xB8A4B6));crystal(0,202,40,110,true);
 for(int i=0;i<2;++i){const auto p=lampHeads[i];const int sx=p.x+(i?40:-40);box(sx,205,p.z,15,410,15,paint(0x7598A6));box(p.x,425,p.z,96,15,47,paint(0x7392A0));box(p.x,415,p.z,77,5,35,paint(i?0x92F5FF:0xFFD6A2));box(sx,12,p.z,60,24,60,paint(0x435B74));}
 plant(-480,-65);plant(470,35);
 // Reflect positions and winding about Y=0. Keep floor after every mirror in
 // the stable background band; ordinary scene objects then paint over it.
 std::stable_sort(subjects.begin(),subjects.end(),[](auto* a,auto* b){return a->position.z+a->centreVolume.z>b->position.z+b->centreVolume.z;});
 for(auto* source:subjects){auto* mirror=mirrorOf(source);scene->addObject(mirror);if(source==jewel)jewelMirror=mirror;}
 floorMaterial=paint(0x25424E,158);floorQuad(-1500,-850,1500,1500,0,floorMaterial);
 auto* seam=paint(0x71939F,48);for(int z=-600;z<=600;z+=200)floorQuad(-720,z-3,720,z+3,1,seam);
 for(int x=-600;x<=600;x+=200)floorQuad(x-3,-750,x+3,650,1,seam);
 pool(lampHeads[0],0xFFD198);pool(lampHeads[1],0x7DE9FA);
 for(auto* o:subjects)scene->addObject(o);
 for(int i=0;i<2;++i){auto& h=halos[i];h.material=&haloMaterials[i];h.blendMode=BlendMode::BLEND_ADD;h.textureFlags=Sprite2D::MIRROR_X|Sprite2D::MIRROR_Y;h.scale=2;h.zOrder=10;scene->addSprite(&h);}
 Labels::add(target);update(0);
}
}

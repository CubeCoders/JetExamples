#pragma once
#include "Scene.hpp"
#include "Primitives.hpp"
#include "Display.hpp"
#include "Assets.hpp"
#include "TreeData.hpp"
#include "Labels.hpp"
#include <cmath>
#include <cstdio>
#include <map>
namespace Woodland {
using namespace Renderer;
inline constexpr float pi=3.14159265359f;
inline Scene* scene=nullptr;
inline Camera camera;
inline float time=0;
inline int stage=-1;
inline bool reference=false;
inline int distance=0;
inline Object *tree=nullptr,*simple=nullptr,*impostor=nullptr;
inline uint16_t sky[Display::RENDER_HEIGHT];
inline Texture treeTexture(128,160,const_cast<uint16_t*>(Assets::pine),true,0);
inline Material treeMaterial(0xffff,&treeTexture);
inline uint16_t rgb(unsigned c){return uint16_t(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));}
inline Material* paint(unsigned c){auto* m=new Material(rgb(c));m->shadingMode=ShadingMode::UNLIT;return m;}
inline void finish(Object* o){o->calculateBoundingBox();o->cachePositions();}
inline void quad(Object* o,Vector3 a,Vector3 b,Vector3 c,Vector3 d,Material* m){const int n=int(o->vertices.size());for(auto v:{a,b,c,d})o->addVertex({v});o->addFace(n,n+1,n+2,n+3,m);}
template<size_t N> Object* mesh(const TreeData::Face (&data)[N]) {
 auto* o=new Object;std::map<uint16_t,Material*> colours;
 for(const auto& f:data){auto*& m=colours[f.colour];if(!m){m=new Material(f.colour);m->shadingMode=ShadingMode::UNLIT;}int n=int(o->vertices.size());for(int v=0;v<3;++v)o->addVertex({{f.p[v*3],f.p[v*3+1],f.p[v*3+2]}});o->addTriangle(n,n+1,n+2,m);}
 o->cullingMode=CullingMode::CULL_BACKFACES;finish(o);return o;
}
inline void update(float dt){
 time=std::fmod(time+dt,32.f);reference=time>=16;const float phase=std::fmod(time,16.f),a=phase*2*pi/16;
 const float radius=1250+2050*(.5f-.5f*std::cos(a)),angle=.30f*std::sin(a);
 camera.setPosition(int(radius*std::sin(angle)),300,-int(radius*std::cos(angle)));camera.lookAt({0,280,0});
 const auto delta=camera.position-tree->position;distance=int(std::sqrt(float(delta.x*delta.x+delta.y*delta.y+delta.z*delta.z)));
 scene->lodScale=reference?0:1800;tree->fadeNear=2400;tree->fadeFar=reference?0:2600;impostor->enabled=!reference;
 const int next=reference?4:(distance<1800?0:distance<=2400?1:distance<2600?2:3);
 if(next!=stage){stage=next;Labels::select(stage,reference);const char* names[]={"FULL MESH","SIMPLE MESH","TRANSITION","BILLBOARD","REFERENCE FULL MESH"};std::printf("Woodland: %s; distance %d\n",names[stage],distance);}
}
inline void init(Scene& target){
 scene=&target;camera.setFOV(58.f,Display::RENDER_WIDTH);camera.nearPlane=40;camera.farPlane=9000;scene->setCamera(&camera);scene->setClearBuffer(true);
 for(int y=0;y<Display::RENDER_HEIGHT;++y)sky[y]=rgb(((111+y*70/320)<<16)|((170+y*49/320)<<8)|(207+y*28/320));
 scene->backgroundGradientColors=sky;
 auto* hills=new Object;hills->noWriteZBuffer=true;hills->cullingMode=CullingMode::NO_CULLING;auto* hill=paint(0x8DA993);
 for(int i=0;i<12;++i){int x=-6500+i*1100,n=int(hills->vertices.size());for(auto v:{Vector3{x,0,4300},Vector3{x+630,450+(i*211)%400,4300},Vector3{x+1300,0,4300}})hills->addVertex({v});hills->addTriangle(n,n+1,n+2,hill);}finish(hills);scene->addObject(hills);
 auto* ground=new Object;ground->noWriteZBuffer=true;ground->cullingMode=CullingMode::NO_CULLING;quad(ground,{-9000,0,-6500},{9000,0,-6500},{9000,0,6500},{-9000,0,6500},paint(0x9EAD70));finish(ground);scene->addObject(ground);
 // A short path and fence provide scale as the camera retreats.
 auto* path=new Object;path->noWriteZBuffer=true;path->cullingMode=CullingMode::NO_CULLING;quad(path,{-470,1,-5000},{-260,1,-5000},{-260,1,3500},{-470,1,3500},paint(0xC6B896));finish(path);scene->addObject(path);
 auto* fence=new Object;fence->noWriteZBuffer=true;fence->cullingMode=CullingMode::NO_CULLING;auto* wood=paint(0x8A7455);
 for(int z=-650;z<=2300;z+=350)quad(fence,{530,0,z-10},{530,0,z+10},{530,110,z+10},{530,110,z-10},wood);
 for(int y:{40,85})quad(fence,{530,y,-650},{530,y,2300},{530,y+10,2300},{530,y+10,-650},wood);
 finish(fence);scene->addObject(fence);
 treeMaterial.shadingMode=ShadingMode::UNLIT;
 for(int i=0;i<6;++i){auto* o=Primitives::createBillboard(512,640,&treeMaterial);o->setPosition((i-3)*670+290,280,2100+(i%2)*300);o->cullingMode=CullingMode::NO_CULLING;finish(o);scene->addObject(o);}
 tree=mesh(TreeData::high);
 // Use a symmetric conservative box so mesh and billboard measure the same distance.
 tree->boundingBoxMin.z=-205;tree->boundingBoxMax.z=205;tree->centreVolume={0,0,0};
 simple=mesh(TreeData::low);simple->enabled=false;tree->lodMeshes.push_back(simple);tree->setPosition(0,280,0);scene->addObject(tree);
 impostor=Primitives::createBillboard(512,640,&treeMaterial);impostor->setPosition(0,280,0);impostor->cullingMode=CullingMode::NO_CULLING;impostor->appearNear=2400;impostor->appearFar=2600;finish(impostor);scene->addObject(impostor);
 Labels::add(target);update(0);
}
}

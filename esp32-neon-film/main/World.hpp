#pragma once
#include "Scene.hpp"
#include "Primitives.hpp"
#include "EnvironmentMapping.hpp"
#include "Assets.hpp"
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
namespace Film {
using namespace Renderer;
inline constexpr float pi=3.14159265359f;
inline Scene* scene=nullptr;
inline Camera camera;
inline DirectionalLight key({225,40,0},{180,220,255},255);
inline AmbientLight ambient({115,104,155});
inline uint16_t rgb(unsigned c){return uint16_t(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));}
inline float clamp(float t){return std::clamp(t,0.f,1.f);}
inline Vector3 lerp(Vector3 a,Vector3 b,float t){return {int(a.x+(b.x-a.x)*t),int(a.y+(b.y-a.y)*t),int(a.z+(b.z-a.z)*t)};}
inline Vector3 yawed(Vector3 p,float a){a*=pi/180;return {int(p.x*std::cos(a)+p.z*std::sin(a)),p.y,int(-p.x*std::sin(a)+p.z*std::cos(a))};}
inline Texture facades[]={Texture(64,128,const_cast<uint16_t*>(Assets::facade0)),Texture(64,128,const_cast<uint16_t*>(Assets::facade1)),Texture(64,128,const_cast<uint16_t*>(Assets::facade2)),Texture(64,128,const_cast<uint16_t*>(Assets::facade3))};
inline Texture signs[]={Texture(128,64,const_cast<uint16_t*>(Assets::sign0)),Texture(128,64,const_cast<uint16_t*>(Assets::sign1)),Texture(128,64,const_cast<uint16_t*>(Assets::sign2)),Texture(128,64,const_cast<uint16_t*>(Assets::sign3)),Texture(128,64,const_cast<uint16_t*>(Assets::sign4)),Texture(128,64,const_cast<uint16_t*>(Assets::sign5))};
inline Texture shops[]={Texture(128,64,const_cast<uint16_t*>(Assets::shop0)),Texture(128,64,const_cast<uint16_t*>(Assets::shop1)),Texture(128,64,const_cast<uint16_t*>(Assets::shop2)),Texture(128,64,const_cast<uint16_t*>(Assets::shop3))};
inline Texture env(128,64,const_cast<uint16_t*>(Assets::environment)),holoTex(64,96,const_cast<uint16_t*>(Assets::hologram),true,0),dashTex(256,128,const_cast<uint16_t*>(Assets::dashboard));
inline Texture glowTex(16,16,const_cast<uint16_t*>(Assets::glow),true,0);
// All textures are immutable flash-backed objects. Scene-local meshes/materials
// can be released during update: scanout retains copies of sprite/material state.
struct Bank {
 std::vector<std::unique_ptr<Object>> objects;
 std::vector<std::unique_ptr<Material>> materials;
 std::vector<std::unique_ptr<Sprite2D>> sprites;
 void clear(){scene->getObjects().clear();auto& list=scene->getSprites();for(auto& s:sprites)list.erase(std::remove(list.begin(),list.end(),s.get()),list.end());sprites.clear();objects.clear();materials.clear();}
 Object* own(Object* o){objects.emplace_back(o);return o;}
 Material* paint(unsigned c,int alpha=255,ShadingMode mode=ShadingMode::UNLIT){materials.emplace_back(new Material(rgb(c)));auto* m=materials.back().get();m->alpha=uint8_t(alpha);m->shadingMode=mode;return m;}
 Material* texture(Texture* t,int alpha=255){auto* m=paint(0xffffff,alpha);m->diffuseMap=t;m->perspectiveCorrect=false;return m;}
 Object* finish(Object* o,bool background=false){o->noWriteZBuffer=background;o->calculateBoundingBox();o->cachePositions();scene->addObject(o);return o;}
 Object* object(){auto* o=own(new Object);o->cullingMode=CullingMode::NO_CULLING;return o;}
 Sprite2D* sprite(Material* m,int x,int y,int z=30){sprites.emplace_back(new Sprite2D);auto* s=sprites.back().get();s->material=m;s->x=x;s->y=y;s->zOrder=z;scene->addSprite(s);return s;}
} ;
inline Bank bank;
inline std::vector<Object*> architecture;
inline void quad(Object* o,Vector3 a,Vector3 b,Vector3 c,Vector3 d,Material* m){int n=int(o->vertices.size());o->addVertex({a,{0,0},{0,1024,0}});o->addVertex({b,{1024,0},{0,1024,0}});o->addVertex({c,{1024,1024},{0,1024,0}});o->addVertex({d,{0,1024},{0,1024,0}});o->addFace(n,n+1,n+2,n+3,m);}
inline Object* box(int x,int y,int z,int w,int h,int d,Material* m,bool bg=false){auto* o=bank.own(Primitives::createCube(w,h,d,m));o->setPosition(x,y,z);return bank.finish(o,bg);}
inline Object* wall(int x,int y,int z,int w,int h,int d,Material* m){auto* o=box(x,y,z,w,h,d,m);architecture.push_back(o);return o;}
inline Object* panel(Vector3 a,Vector3 b,Vector3 c,Vector3 d,Material* m,bool bg=false){auto* o=bank.object();quad(o,a,b,c,d,m);return bank.finish(o,bg);}
inline Object* disc(int x,int y,int z,int radius,Material* m,bool bg=false){auto* o=bank.object();o->addVertex({{x,y,z}});for(int i=0;i<16;++i){float a=i*2*pi/16;o->addVertex({{x+int(radius*std::cos(a)),y,z+int(radius*std::sin(a))}});}for(int i=0;i<16;++i)o->addTriangle(0,i+1,(i+1)%16+1,m);return bank.finish(o,bg);}
// Combine adjacent static details into one transform/cull operation. Keep
// billboard lamps and background floor layers separate. Materials are shared.
inline void batchStaticDetails(size_t first){
 auto* combined=new Object;combined->cullingMode=CullingMode::NO_CULLING;
 std::vector<Object*> removed;
 for(size_t i=first;i<bank.objects.size();++i){auto* source=bank.objects[i].get();if(source->isBillboard||source->noWriteZBuffer)continue;
  uint16_t base=uint16_t(combined->vertices.size());
  for(auto v:source->vertices){v.position=v.position+source->position;combined->addVertex(v);}
  for(auto t:source->triangles)combined->addTriangle(base+t.v1,base+t.v2,base+t.v3,t.material);
  removed.push_back(source);
 }
 auto& list=scene->getObjects();for(auto* dead:removed)list.erase(std::remove(list.begin(),list.end(),dead),list.end());
 bank.objects.erase(std::remove_if(bank.objects.begin()+first,bank.objects.end(),[&](const auto& o){return std::find(removed.begin(),removed.end(),o.get())!=removed.end();}),bank.objects.end());
 bank.finish(bank.own(combined));
}
inline Object* reflected(Object* source){auto* o=bank.own(new Object(*source));o->invalidatePositions();for(auto& v:o->vertices){v.position.y=-v.position.y;v.normal.y=-v.normal.y;}for(auto& t:o->triangles)std::swap(t.v2,t.v3);o->position.y=-o->position.y;return bank.finish(o,true);}
inline void billboardGlow(Vector3 position,int size){auto* m=bank.texture(&glowTex);auto* s=bank.sprite(m,0,0,10);s->blendMode=BlendMode::BLEND_ADD;s->textureFlags=Sprite2D::MIRROR_X|Sprite2D::MIRROR_Y;s->scale=size;const auto v=camera.transformDirection(position-camera.position);if(v.z>40){s->x=240+int(v.x*camera.fovFactor/v.z)-16*size;s->y=160-int(v.y*camera.fovFactor/v.z)-16*size;}else s->enabled=false;}
}

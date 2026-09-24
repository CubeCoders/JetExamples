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
#ifndef FILM_RENDER_SCALE
#define FILM_RENDER_SCALE 1
#endif
inline constexpr int renderScale=FILM_RENDER_SCALE;
inline constexpr int renderWidth=480*renderScale,renderHeight=320*renderScale;
inline constexpr float pi=3.14159265359f;
inline Scene* scene=nullptr;
inline Camera camera;
inline DirectionalLight key({225,40,0},{180,220,255},255);
inline AmbientLight ambient({115,104,155});
// Grade authored colours once, before lighting/rasterization. Keep this curve
// in sync with tools/prepare_assets.py; no full-screen postprocess is needed.
inline int contrastChannel(int value){return std::clamp((value*5-80+2)/4,0,255);}
inline uint16_t rgb(unsigned c){int r=contrastChannel((c>>16)&255),g=contrastChannel((c>>8)&255),b=contrastChannel(c&255);return uint16_t((r>>3)<<11|(g>>2)<<5|(b>>3));}
inline float clamp(float t){return std::clamp(t,0.f,1.f);}
inline Vector3 lerp(Vector3 a,Vector3 b,float t){return {int(a.x+(b.x-a.x)*t),int(a.y+(b.y-a.y)*t),int(a.z+(b.z-a.z)*t)};}
inline Vector3 yawed(Vector3 p,float a){a*=pi/180;return {int(p.x*std::cos(a)+p.z*std::sin(a)),p.y,int(-p.x*std::sin(a)+p.z*std::cos(a))};}
inline Texture facades[]={Texture(64,128,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::facade0)),false,0,false,WRAP,Assets::facade0Palette),Texture(64,128,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::facade1)),false,0,false,WRAP,Assets::facade1Palette),Texture(64,128,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::facade2)),false,0,false,WRAP,Assets::facade2Palette),Texture(64,128,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::facade3)),false,0,false,WRAP,Assets::facade3Palette)};
// Small filtered facade mipmaps live in internal BSS (8 KiB on ESP32).
// Fill once before rendering, then treat them as immutable like the flash art.
alignas(4) inline uint16_t farFacadePixels[4][32*32];
inline Texture farFacades[]={Texture(32,32,farFacadePixels[0]),Texture(32,32,farFacadePixels[1]),Texture(32,32,farFacadePixels[2]),Texture(32,32,farFacadePixels[3])};
inline void prepareFarFacades(){
 static bool ready=false;if(ready)return;ready=true;
 for(int k=0;k<4;++k)for(int y=0;y<32;++y)for(int x=0;x<32;++x){
  int r=0,g=0,b=0;
  for(int dy=0;dy<4;++dy)for(int dx=0;dx<2;++dx){uint16_t c=facades[k].palette[reinterpret_cast<const uint8_t*>(facades[k].data)[(y*4+dy)*64+x*2+dx]];r+=(c>>11)&31;g+=(c>>5)&63;b+=c&31;}
  farFacadePixels[k][y*32+x]=uint16_t((r/8)<<11|(g/8)<<5|b/8);
 }
}
inline Texture signs[]={Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::sign0)),false,0,false,WRAP,Assets::sign0Palette),Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::sign1)),false,0,false,WRAP,Assets::sign1Palette),Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::sign2)),false,0,false,WRAP,Assets::sign2Palette),Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::sign3)),false,0,false,WRAP,Assets::sign3Palette),Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::sign4)),false,0,false,WRAP,Assets::sign4Palette),Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::sign5)),false,0,false,WRAP,Assets::sign5Palette)};
inline Texture shops[]={Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::shop0)),false,0,false,WRAP,Assets::shop0Palette),Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::shop1)),false,0,false,WRAP,Assets::shop1Palette),Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::shop2)),false,0,false,WRAP,Assets::shop2Palette),Texture(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::shop3)),false,0,false,WRAP,Assets::shop3Palette)};
inline Texture env(128,64,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::environment)),false,0,false,WRAP,Assets::environmentPalette),holoTex(64,96,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::hologram)),true,0,false,WRAP,Assets::hologramPalette),dashTex(256,128,reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(Assets::dashboard)),false,0,false,WRAP,Assets::dashboardPalette);
inline Texture glowTex(16,16,const_cast<uint16_t*>(Assets::glow),true,0);
// Texture data is immutable after setup: flash art plus the small DRAM mip cache.
// Scene-local meshes/materials
// can be released during update: scanout retains copies of sprite/material state.
struct Bank {
 std::vector<std::unique_ptr<Object>> objects;
 std::vector<std::unique_ptr<Object>> lodStorage;
 std::vector<std::unique_ptr<Material>> materials;
 std::vector<std::unique_ptr<Sprite2D>> sprites;
 void clear(){scene->getObjects().clear();auto& list=scene->getSprites();for(auto& s:sprites)list.erase(std::remove(list.begin(),list.end(),s.get()),list.end());sprites.clear();objects.clear();lodStorage.clear();materials.clear();}
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
// Luminous tubes keep a minimum projected footprint in packed half-width
// fields. They remain depth-sorted geometry; these are not overlay sprites.
struct NeonStroke {Object* mesh;int vertex;Vector3 a,b;float width;};
inline std::vector<NeonStroke> neonStrokes;
inline Object* neonLine(Vector3 a,Vector3 b,Material* material,float width=7){
 auto* o=bank.object();quad(o,a+Vector3{-4,0,0},a+Vector3{4,0,0},b+Vector3{4,0,0},b+Vector3{-4,0,0},material);
 neonStrokes.push_back({o,0,a,b,width});return bank.finish(o);
}
inline void prepareNeon(){
 std::vector<Object*> prepared;
 for(auto& stroke:neonStrokes){auto* o=stroke.mesh;if(std::find(prepared.begin(),prepared.end(),o)!=prepared.end())continue;
  prepared.push_back(o);o->invalidatePositions();o->boundingBoxMin=o->boundingBoxMin-Vector3{256,256,256};o->boundingBoxMax=o->boundingBoxMax+Vector3{256,256,256};
 }
}
inline void animateNeon(){
 int32_t cx,sx,cy,sy,cz,sz;camera.getRotationMatrix(cx,sx,cy,sy,cz,sz);
 auto fromView=[&](float x,float y,float length){
  float xx=(x*cz+y*sz)/1024,yy=(-x*sz+y*cz)/1024;
  float wy=yy*cx/1024,wz=-yy*sx/1024;
  return Vector3{int(std::round((xx*cy-wz*sy)*length/1024)),int(std::round(wy*length)),int(std::round((xx*sy+wz*cy)*length/1024))};
 };
 for(auto& stroke:neonStrokes){auto* o=stroke.mesh;auto a=camera.transformDirection(o->position+yawed(stroke.a,o->rotation.y)-camera.position),b=camera.transformDirection(o->position+yawed(stroke.b,o->rotation.y)-camera.position);
  float az=float(std::max(a.z,camera.nearPlane)),bz=float(std::max(b.z,camera.nearPlane));float dx=b.x/bz-a.x/az,dy=b.y/bz-a.y/az,length=std::sqrt(dx*dx+dy*dy);
  if(length<.00001f)continue;
  float nx=-dy/length,ny=dx/length;
  // 3 output pixels leaves room for integer transform/projection rounding;
  // at least one packed sample covers each scanline of a vertical tube.
  auto offset=[&](float z){return yawed(fromView(nx,ny,std::max(stroke.width/2,1.5f*renderScale*z/camera.fovFactor)),-o->rotation.y);};
  auto aa=offset(az),bb=offset(bz);int n=stroke.vertex;
  o->vertices[n].position=stroke.a-aa;o->vertices[n+1].position=stroke.a+aa;
  o->vertices[n+2].position=stroke.b+bb;o->vertices[n+3].position=stroke.b-bb;
 }
}
inline Object* box(int x,int y,int z,int w,int h,int d,Material* m,bool bg=false){auto* o=bank.own(Primitives::createCube(w,h,d,m));o->setPosition(x,y,z);return bank.finish(o,bg);}
inline Object* wall(int x,int y,int z,int w,int h,int d,Material* m){auto* o=box(x,y,z,w,h,d,m);architecture.push_back(o);return o;}
inline Object* panel(Vector3 a,Vector3 b,Vector3 c,Vector3 d,Material* m,bool bg=false){auto* o=bank.object();quad(o,a,b,c,d,m);return bank.finish(o,bg);}
inline Object* disc(int x,int y,int z,int radius,Material* m,bool bg=false){auto* o=bank.object();o->addVertex({{x,y,z}});for(int i=0;i<16;++i){float a=i*2*pi/16;o->addVertex({{x+int(radius*std::cos(a)),y,z+int(radius*std::sin(a))}});}for(int i=0;i<16;++i)o->addTriangle(0,i+1,(i+1)%16+1,m);return bank.finish(o,bg);}
// Combine adjacent static details into one transform/cull operation. Keep
// billboard lamps and background floor layers separate. Materials are shared.
inline Object* batchStaticDetails(size_t first){
 auto* combined=new Object;combined->cullingMode=CullingMode::NO_CULLING;
 std::vector<Object*> removed;
 for(size_t i=first;i<bank.objects.size();++i){auto* source=bank.objects[i].get();if(source->isBillboard||source->noWriteZBuffer)continue;
  uint16_t base=uint16_t(combined->vertices.size());
  for(auto v:source->vertices){v.position=v.position+source->position;combined->addVertex(v);}
  for(auto t:source->triangles)combined->addTriangle(base+t.v1,base+t.v2,base+t.v3,t.material);
  for(auto& stroke:neonStrokes)if(stroke.mesh==source){stroke.mesh=combined;stroke.vertex+=base;stroke.a=stroke.a+source->position;stroke.b=stroke.b+source->position;}
  removed.push_back(source);
 }
 auto& list=scene->getObjects();for(auto* dead:removed)list.erase(std::remove(list.begin(),list.end(),dead),list.end());
 bank.objects.erase(std::remove_if(bank.objects.begin()+first,bank.objects.end(),[&](const auto& o){return std::find(removed.begin(),removed.end(),o.get())!=removed.end();}),bank.objects.end());
 return bank.finish(bank.own(combined));
}
inline Object* reflected(Object* source){auto* o=bank.own(new Object(*source));o->invalidatePositions();for(auto& v:o->vertices){v.position.y=-v.position.y;v.normal.y=-v.normal.y;}for(auto& t:o->triangles)std::swap(t.v2,t.v3);o->position.y=-o->position.y;size_t count=neonStrokes.size();for(size_t i=0;i<count;++i)if(neonStrokes[i].mesh==source){auto copy=neonStrokes[i];copy.mesh=o;copy.a.y=-copy.a.y;copy.b.y=-copy.b.y;neonStrokes.push_back(copy);}return bank.finish(o,true);}
inline void billboardGlow(Vector3 position,int size){auto* m=bank.texture(&glowTex);auto* s=bank.sprite(m,0,0,10);s->blendMode=BlendMode::BLEND_ADD;s->textureFlags=Sprite2D::MIRROR_X|Sprite2D::MIRROR_Y;s->scale=size*renderScale;const auto v=camera.transformDirection(position-camera.position);if(v.z>40){s->x=renderWidth/2+int(v.x*camera.fovFactor/v.z)-16*size*renderScale;s->y=renderHeight/2-int(v.y*camera.fovFactor/v.z)-16*size*renderScale;}else s->enabled=false;}
}

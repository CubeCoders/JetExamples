#pragma once
#include "World.hpp"
namespace Film {
struct Glow {Vector3 position;Sprite2D* sprite;int scale;};
inline std::vector<Glow> glows;
inline std::vector<Material*> flicker;
inline Material* water=nullptr;
struct Searchlight {Object* mesh;int phase;};
inline std::vector<Searchlight> searchlights;
inline std::vector<Object*> holograms;
inline void animateCity(float t){
 for(auto& b:searchlights){b.mesh->rotation.z=int(19*std::sin(t*.31f+b.phase));b.mesh->rotation.y=int(20*std::sin(t*.19f+b.phase));}
 for(size_t i=0;i<holograms.size();++i){holograms[i]->position.y=1000+int(i)*110+int(24*std::sin(t*.7f+i));}
}
inline void glow(Vector3 p,int scale=2){auto* s=bank.sprite(bank.texture(&glowTex),0,0,10);s->blendMode=BlendMode::BLEND_ADD;s->textureFlags=Sprite2D::MIRROR_X|Sprite2D::MIRROR_Y;s->scale=scale;glows.push_back({p,s,scale});}
inline void projectGlows(){for(auto& g:glows){auto v=camera.transformDirection(g.position-camera.position);auto* s=g.sprite;s->enabled=v.z>80 && v.z<camera.farPlane;if(s->enabled){s->x=240+int(v.x*camera.fovFactor/v.z)-16*g.scale;s->y=160-int(v.y*camera.fovFactor/v.z)-16*g.scale;}}}
inline void building(int x,int z,int width,int depth,int height,int style,bool detail=true){
 auto* dark=bank.paint(style%2?0x19243B:0x25213C);
 auto* tower=bank.object();const int bevel=std::min(width,depth)/6;
 const Vector3 ring[]={{-width/2+bevel,0,-depth/2},{width/2-bevel,0,-depth/2},{width/2,0,-depth/2+bevel},{width/2,0,depth/2-bevel},{width/2-bevel,0,depth/2},{-width/2+bevel,0,depth/2},{-width/2,0,depth/2-bevel},{-width/2,0,-depth/2+bevel}};
 for(int j=0;j<8;++j){auto a=ring[j],b=ring[(j+1)%8];int ah=height+(style%3==0?a.x/3:0),bh=height+(style%3==0?b.x/3:0);quad(tower,a,b,b+Vector3{0,bh,0},a+Vector3{0,ah,0},dark);int n=int(tower->vertices.size());tower->addVertex({{0,height,0}});tower->addVertex({a+Vector3{0,ah,0}});tower->addVertex({b+Vector3{0,bh,0}});tower->addTriangle(n,n+1,n+2,dark);}
 for(auto& triangle:tower->triangles)std::swap(triangle.v1,triangle.v3);
 tower->cullingMode=CullingMode::CULL_BACKFACES;
 tower->position={x,0,z};bank.finish(tower);architecture.push_back(tower);
 auto* windows=bank.texture(&facades[style%4]);
 panel({x-width/2+width/6,30,z-depth/2-2},{x+width/2-width/6,30,z-depth/2-2},{x+width/2-width/6,height-65,z-depth/2-2},{x-width/2+width/6,height-65,z-depth/2-2},windows);
 panel({x-width/2-2,30,z+depth/2-depth/6},{x-width/2-2,30,z-depth/2+depth/6},{x-width/2-2,height-65,z-depth/2+depth/6},{x-width/2-2,height-65,z+depth/2-depth/6},windows);
 if(detail){
  auto* trim=bank.paint(style%2?0xEA4BAF:0x37C8E2);
  box(x-width/2-4,height/2,z-depth/2-4,7,height,7,trim);
  if(style%3==1)box(x,height+60,z,width/2,120,depth/2,bank.paint(0x36475D));
  if(style%3==0){box(x,height+80,z,14,160,14,trim);glow({x,height+164,z},2);}
 }
}
inline void skyline(){
 for(int i=0;i<17;++i){int x=(i-8)*440,z=2500+(i%3)*330,h=600+(i*433)%1400;building(x,z,330+(i%3)*70,340,h,i,i%2==0);}
 // Distinct central spire and a lit bridge over the far riverbank.
 building(240,2200,510,480,2200,1);box(240,2280,2200,100,160,100,bank.paint(0xB954CA));glow({240,2370,2200},3);
 auto* beam=bank.paint(0x38B4E9,25,ShadingMode::ADDITIVE);
 for(int x:{-1400,1450}){auto* b=panel({-18,0,0},{18,0,0},{220,1750,0},{-220,1750,0},beam);b->position={x,650,1600};searchlights.push_back({b,x<0?0:3});glow({x,660,1600},2);}
 auto* bridge=bank.paint(0x31CEDB);box(0,90,1900,8000,24,60,bank.paint(0x1F3247));box(0,130,1880,8000,7,7,bridge);
 auto* holo=bank.texture(&holoTex,150);holo->shadingMode=ShadingMode::ADDITIVE;
 for(int i=0;i<3;++i){auto* o=bank.own(Primitives::createBillboard(380,570,holo));o->cullingMode=CullingMode::NO_CULLING;o->setPosition(-1700+i*1680,1000+i*110,1550);bank.finish(o);holograms.push_back(o);}
 water=bank.paint(0x102D4E,100,ShadingMode::WATER_REFLECT);water->specular=30;
 for(int x=-3;x<3;++x)for(int z=-2;z<2;++z)panel({x*2100,0,z*1800-1650},{(x+1)*2100,0,z*1800-1650},{(x+1)*2100,0,(z+1)*1800-1650},{x*2100,0,(z+1)*1800-1650},water,true);
}
inline void street(int halfWidth=320,int blocks=12,bool mirror=false,int detailedBlocks=-1){
 const size_t first=bank.objects.size();
 auto* floor=bank.paint(0x141D2B);
 const int roadEnd=blocks*720+1500;
 panel({-halfWidth-350,0,-roadEnd},{halfWidth+350,0,-roadEnd},{halfWidth+350,0,roadEnd},{-halfWidth-350,0,roadEnd},floor,true);
 auto* stripe=bank.paint(0x72808C);auto* trim=bank.paint(0x45D1D2);auto* pink=bank.paint(0xD94593);auto* curb=bank.paint(0x394353);auto* wallMat=bank.paint(0x1D2638);auto* awning=bank.paint(0x3D2749);auto* pipe=bank.paint(0x4C5667);
 Material* signsShared[6];for(int i=0;i<6;++i){signsShared[i]=bank.texture(&signs[i]);signsShared[i]->perspectiveCorrect=true;flicker.push_back(signsShared[i]);}
 Material* windows[4];Material* fronts[4];for(int i=0;i<4;++i){windows[i]=bank.texture(&facades[i]);fronts[i]=bank.texture(&shops[i]);}
 Material* farWindows[4];for(int i=0;i<4;++i)farWindows[i]=bank.texture(&farFacades[i]);
 for(int i=0;i<blocks;++i){int z=i*720-700;
  if(halfWidth>500)for(int x:{-190,190})panel({x-3,2,z},{x+3,2,z},{x+3,2,z+210},{x-3,2,z+210},stripe,true);
  for(int side:{-1,1}){
   int x=side*(halfWidth+330),h=1400+(i*371+side*157+2000)%1100;
   auto* shell=wall(x,h/2,z,620,h,650,wallMat);
   // The textured shop/window panels ARE the inward face. Keeping a second
   // tall solid quad behind them creates painter-order diagonal occlusion.
   auto& tris=shell->triangles;tris.erase(std::remove_if(tris.begin(),tris.end(),[&](const auto& t){return shell->vertices[t.v1].position.x==-side*310 && shell->vertices[t.v2].position.x==-side*310 && shell->vertices[t.v3].position.x==-side*310;}),tris.end());
   const size_t artFirst=bank.objects.size();
   int faceX=side*(halfWidth+16),k=(i+(side>0?1:0))%4;
   if(detailedBlocks>=0 && i>=detailedBlocks){
    // The descending rain rig never approaches these blocks. Keep their
    // skyline, lit facade and reflection without near-field shop furniture.
    auto* facade=panel({faceX,16,z-300},{faceX,16,z+300},{faceX,h-40,z+300},{faceX,h-40,z-300},farWindows[k]);
    if(side>0)for(auto& v:facade->vertices)v.uv.x=1024-v.uv.x;
    panel({faceX-side,40,z-307},{faceX-side,40,z-300},{faceX-side,h-40,z-300},{faceX-side,h-40,z-307},i%2?pink:trim);
    continue;
   }
   panel({faceX,350,z-300},{faceX,350,z+300},{faceX,h-40,z+300},{faceX,h-40,z-300},windows[k]);
   panel({faceX,16,z-300},{faceX,16,z+300},{faceX,305,z+300},{faceX,305,z-300},fronts[k]);
   panel({side*halfWidth,8,z-340},{side*(halfWidth+100),8,z-340},{side*(halfWidth+100),8,z+340},{side*halfWidth,8,z+340},curb,true);
   // Sloped canopies, angled braces and a projecting sign break the box grid.
   panel({faceX,345,z-308},{faceX,345,z+308},{side*(halfWidth-58),303,z+308},{side*(halfWidth-58),303,z-308},awning);
   panel({side*(halfWidth-59),303,z-309},{side*(halfWidth-59),303,z+309},{side*(halfWidth-59),297,z+309},{side*(halfWidth-59),297,z-309},i%2?pink:trim);
   auto* lamp=bank.object();lamp->isBillboard=true;lamp->setPosition(side*(halfWidth-18),0,z+280);
   quad(lamp,{-3,0,0},{3,0,0},{3,290,0},{-3,290,0},pipe);
   quad(lamp,{-24,290,0},{24,290,0},{24,295,0},{-24,295,0},trim);bank.finish(lamp);lamp->fadeNear=lamp->fadeFar=3000;
   auto* sm=signsShared[(i+(side>0?2:0))%6];
   int sx=side*(halfWidth-24);
   panel({sx,360,z-220},{sx,360,z+85},{sx,462,z+85},{sx,462,z-220},sm);
   if(i%3==0){panel({side*(halfWidth-4),470,z-302},{side*(halfWidth-105),470,z-302},{side*(halfWidth-105),650,z-302},{side*(halfWidth-4),650,z-302},sm);glow({sx,388,z-65},1);}
   panel({side*(halfWidth-4),480,z-316},{side*(halfWidth-4),480,z-309},{side*(halfWidth-4),950,z-309},{side*(halfWidth-4),950,z-316},i%2?pink:trim);
   if(side>0)for(size_t n=artFirst;n<bank.objects.size();++n){auto* art=bank.objects[n].get();if(art->triangles.empty()||!art->triangles[0].material->diffuseMap)continue;for(auto& v:art->vertices)v.uv.x=1024-v.uv.x;}
   if(!mirror){auto* wet=bank.paint(i%2?0xD0398C:0x309CA5,38);int wx=side*(halfWidth-100);panel({wx-20,2,z-210},{wx+20,2,z-210},{wx+10,2,z+240},{wx-10,2,z+240},wet,true);}
   if(!mirror){
    auto* head=batchStaticDetails(artFirst);
    // Keep the same facade planes and colours at range; omit projecting
    // shop furniture and use a filtered DRAM mip for the lit windows.
    bank.lodStorage.emplace_back(new Object);auto* low=bank.lodStorage.back().get();
    quad(low,{faceX,350,z-300},{faceX,350,z+300},{faceX,h-40,z+300},{faceX,h-40,z-300},farWindows[k]);
    quad(low,{faceX,16,z-300},{faceX,16,z+300},{faceX,305,z+300},{faceX,305,z-300},fronts[k]);
    quad(low,{faceX,299,z-309},{faceX,299,z+309},{faceX,305,z+309},{faceX,305,z-309},i%2?pink:trim);
    if(side>0)for(auto& v:low->vertices)v.uv.x=1024-v.uv.x;
    low->calculateBoundingBox();low->cachePositions();head->lodMeshes.push_back(low);
   }
  }
  if(halfWidth<500 && i%3==1){
   // Two sloping suspended cables, safely above every ground-level rig.
   auto* cable=bank.paint(0x46536D);
   panel({-halfWidth,810,z},{0,745,z+30},{0,749,z+30},{-halfWidth,814,z},cable);
   panel({0,745,z+30},{halfWidth,805,z+60},{halfWidth,809,z+60},{0,749,z+30},cable);
  }
 }
 if(mirror){
  std::vector<Object*> subjects;for(size_t i=first;i<bank.objects.size();++i)if(!bank.objects[i]->noWriteZBuffer && bank.objects[i]->vertices.size()<=4)subjects.push_back(bank.objects[i].get());
  std::stable_sort(subjects.begin(),subjects.end(),[](auto* a,auto* b){return a->position.z+a->centreVolume.z>b->position.z+b->centreVolume.z;});
  for(auto* o:subjects)reflected(o);
  panel({-halfWidth,3,-2000},{halfWidth,3,-2000},{halfWidth,3,blocks*720+1300},{-halfWidth,3,blocks*720+1300},bank.paint(0x142637,175),true);
 }
}
inline void boulevard(int blocks=12){
 street(680,blocks,false);
 auto* rail=bank.paint(0x344255);auto* cyan=bank.paint(0x77DAD6);
 for(int i=0;i<8;++i){int z=i*950;box(-660,40,z,28,80,28,rail);box(660,40,z,28,80,28,rail);box(-660,85,z,28,10,28,cyan);box(660,85,z,28,10,28,cyan);}
}
inline void cityGrid(){
 auto* streetMat=bank.paint(0x142438);panel({-8500,0,-8500},{8500,0,-8500},{8500,0,8500},{-8500,0,8500},streetMat,true);
 for(int i=0;i<7;++i)for(int j=0;j<5;++j){int x=(i-3)*1350,z=(j-1)*1450;building(x,z,620,710,700+(i*371+j*529)%1300,i+j,false);}
 auto* lit=bank.paint(0x34AEBE);for(int i=-3;i<=3;++i){int x=i*1350+570;panel({x-5,2,-3000},{x+5,2,-3000},{x+5,2,7800},{x-5,2,7800},lit,true);}
}
}

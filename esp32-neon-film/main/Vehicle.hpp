#pragma once
#include "World.hpp"
namespace Film {
inline float wheelPhase=0;
struct CarPart {Object* object;Vector3 local;int kind;};
// Original ION coupe, modelled from references/car-design.png. Wheel pods are
// the flight hardware: the body gains no wings, no engine and no rear extension.
struct Vehicle {
 std::vector<CarPart> parts;
 Material *metal=nullptr,*wheelGlow=nullptr;
 Vector3 position;float heading=0;
 void add(Object* o,Vector3 local={0,0,0},int kind=0){if(local.x==0&&local.y==0&&local.z==0)local=o->position;parts.push_back({o,local,kind});}
 void face(Object* o,Vector3 a,Vector3 b,Vector3 c,Vector3 d,Material* m,Vector3 outward){
  auto u=b-a,v=c-a;
  int64_t facing=(int64_t(u.y)*v.z-int64_t(u.z)*v.y)*outward.x+(int64_t(u.z)*v.x-int64_t(u.x)*v.z)*outward.y+(int64_t(u.x)*v.y-int64_t(u.y)*v.x)*outward.z;
  if(facing<0)quad(o,d,c,b,a,m);else quad(o,a,b,c,d,m);
 }
 void wheel(int side,int z,bool transformable){
  auto* o=bank.object();auto* rubber=bank.paint(0x111724);auto* alloy=bank.paint(0x8EADCA);auto* spoke=bank.paint(0x34475B);
  constexpr int segments=8;
  auto point=[side](float a,int radius,int x){return Vector3{side*x,int(radius*std::cos(a)),int(radius*std::sin(a))};};
  for(int i=0;i<segments;++i){float a=i*2*pi/segments,b=(i+1)*2*pi/segments;
   quad(o,point(a,44,-17),point(b,44,-17),point(b,44,17),point(a,44,17),rubber);
   quad(o,point(a,44,18),point(b,44,18),point(b,35,19),point(a,35,19),rubber);
   quad(o,point(a,35,20),point(b,35,20),point(b,14,21),point(a,14,21),i%3?alloy:spoke);
   int n=int(o->vertices.size());o->addVertex({{side*22,0,0}});o->addVertex({point(a,14,22)});o->addVertex({point(b,14,22)});o->addTriangle(n,n+1,n+2,spoke);
  }
  if(side<0)for(auto& triangle:o->triangles)std::swap(triangle.v1,triangle.v3);
  o->cullingMode=CullingMode::CULL_BACKFACES;bank.finish(o);add(o,{side*122,46,z},side<0?2:3);
  if(transformable){auto* ring=bank.object();for(int i=0;i<segments;++i){float a=i*2*pi/segments,b=(i+1)*2*pi/segments;quad(ring,point(a,30,23),point(b,30,23),point(b,22,23),point(a,22,23),wheelGlow);}bank.finish(ring);add(ring,{side*122,46,z},side<0?2:3);}
 }
 void heroBody(bool transformable){
  metal=bank.paint(0x7899CB,255,ShadingMode::PHONG);metal->diffuse=165;metal->specular=230;metal->specularExponent=32;
  auto* edge=bank.paint(0x465E85,255,ShadingMode::PHONG);edge->diffuse=175;edge->specular=190;edge->specularExponent=32;
  auto* trim=bank.paint(0x122234);auto* lamps=bank.paint(0xB5F8FF);auto* red=bank.paint(0xFF395D);
  auto* body=bank.object();body->cullingMode=CullingMode::CULL_BACKFACES;
  const int zs[]={-285,-225,-110,95,205,285},widths[]={108,132,128,124,128,110},ys[]={98,117,111,104,94,66};
  for(int i=0;i<5;++i){int z=zs[i],n=zs[i+1],a=widths[i],b=widths[i+1],y=ys[i],v=ys[i+1];
   face(body,{-a+18,y+8,z},{a-18,y+8,z},{b-18,v+8,n},{-b+18,v+8,n},metal,{0,1,0});
   for(int s:{-1,1})face(body,{s*a,y,z},{s*(a-18),y+8,z},{s*(b-18),v+8,n},{s*b,v,n},edge,{s,1,0});
  }
  // Side panels stop at the wheel arches; the tyres are not buried in a box.
  const int stations[]={-285,-226,-213,-175,-137,-124,-85,85,124,137,175,213,226,285};
  const int lower[]={38,38,82,109,82,38,32,32,38,80,101,80,38,38};
  auto section=[&](int z){int i=0;while(i<4&&z>zs[i+1])++i;float q=float(z-zs[i])/(zs[i+1]-zs[i]);return std::pair<int,int>{int(widths[i]+(widths[i+1]-widths[i])*q),int(ys[i]+(ys[i+1]-ys[i])*q)};};
  for(int s:{-1,1})for(int i=0;i<13;++i){auto a=section(stations[i]),b=section(stations[i+1]);face(body,{s*a.first,lower[i],stations[i]},{s*b.first,lower[i+1],stations[i+1]},{s*b.first,b.second,stations[i+1]},{s*a.first,a.second,stations[i]},metal,{s,0,0});}
  face(body,{-108,38,-285},{108,38,-285},{108,98,-285},{-108,98,-285},trim,{0,0,-1});
  face(body,{-110,38,285},{110,38,285},{110,66,285},{-110,66,285},trim,{0,0,1});
  face(body,{-98,32,-267},{98,32,-267},{100,32,270},{-100,32,270},trim,{0,-1,0});
  // Canopy pillars and roof define the wedge silhouette, separate glass below.
  face(body,{-84,157,-92},{84,157,-92},{80,160,30},{-80,160,30},metal,{0,1,0});
  for(int s:{-1,1}){
   face(body,{s*111,112,-176},{s*86,157,-96},{s*74,157,-88},{s*96,114,-169},edge,{s,1,-1});
   face(body,{s*107,112,121},{s*80,160,30},{s*74,160,24},{s*98,112,113},edge,{s,1,1});
   face(body,{s*123,34,-117},{s*123,34,115},{s*126,42,115},{s*126,42,-117},trim,{s,0,0});
   // Door shut line, flush handle, and tiny front/rear amber markers.
   face(body,{s*129,46,-78},{s*129,108,-78},{s*129,108,-75},{s*129,46,-75},trim,{s,0,0});
   face(body,{s*129,93,-61},{s*129,93,-29},{s*129,97,-29},{s*129,97,-61},trim,{s,0,0});
   face(body,{s*108,51,286},{s*43,51,286},{s*43,58,286},{s*108,58,286},lamps,{0,0,1});
  }
  face(body,{-100,83,-286},{100,83,-286},{100,90,-286},{-100,90,-286},red,{0,0,-1});
  body->computeFlatNormals();bank.finish(body);add(body);
  auto* glass=bank.texture(&env);auto* windows=bank.object();
  face(windows,{-98,113,113},{98,113,113},{74,159,29},{-74,159,29},glass,{0,1,1});
  face(windows,{-99,114,-170},{99,114,-170},{77,156,-91},{-77,156,-91},glass,{0,1,-1});
  for(int s:{-1,1})face(windows,{s*110,113,-157},{s*106,113,105},{s*77,155,25},{s*82,153,-87},glass,{s,1,0});
  windows->computeFlatNormals();bank.finish(windows);add(windows,{},1);
  for(int s:{-1,1})for(int z:{-175,175})wheel(s,z,transformable);
 }
 void trafficBody(bool police,int style){
  const unsigned colors[]={0x94776C,0x547789,0xBDC2AA,0x785585,0x355665,0xB88542};
  metal=bank.paint(police?0xD4DCDF:colors[style%6]);auto* trim=bank.paint(0x172532);
  auto* body=bank.object();
  quad(body,{-130,45,-280},{130,45,-280},{118,112,-260},{-118,112,-260},metal);
  quad(body,{130,45,-280},{130,45,280},{119,102,280},{118,112,-260},metal);
  quad(body,{130,45,280},{-130,45,280},{-119,102,280},{119,102,280},metal);
  quad(body,{-130,45,280},{-130,45,-280},{-118,112,-260},{-119,102,280},metal);
  quad(body,{-118,112,-260},{118,112,-260},{119,102,280},{-119,102,280},metal);
  auto* glass=bank.paint(police?0x25495F:0x2E5768);int roof=style%2?204:178;
  quad(body,{-101,113,-170},{101,113,-170},{82,roof,-105},{-82,roof,-105},glass);
  quad(body,{-101,108,165},{-82,roof,12},{82,roof,12},{101,108,165},glass);
  quad(body,{-101,113,-170},{-82,roof,-105},{-82,roof,12},{-101,108,165},glass);
  quad(body,{101,108,165},{82,roof,12},{82,roof,-105},{101,113,-170},glass);
  quad(body,{-82,roof,-105},{82,roof,-105},{82,roof,12},{-82,roof,12},metal);
  // Door belt, B pillars, handles, bumper grilles and paired lamps.
  for(int side:{-1,1}){
   quad(body,{side*126,60,-220},{side*126,60,220},{side*124,70,220},{side*124,70,-220},trim);
   quad(body,{side*100,111,-45},{side*100,111,-33},{side*83,roof,-33},{side*83,roof,-45},trim);
   quad(body,{side*124,89,-74},{side*124,89,-34},{side*124,94,-34},{side*124,94,-74},bank.paint(0xB9CFD5));
   auto* lamp=bank.paint(0xFFE3AA);
   quad(body,{side*70-21,80,282},{side*70+21,80,282},{side*70+21,94,282},{side*70-21,94,282},lamp);
   quad(body,{side*75-20,86,-282},{side*75+20,86,-282},{side*75+20,98,-282},{side*75-20,98,-282},bank.paint(0xF33A50));
  }
  quad(body,{-48,55,283},{48,55,283},{48,77,283},{-48,77,283},trim);
  bank.finish(body);add(body);
  auto* rubber=bank.paint(0x0C141E);auto* rim=bank.paint(0x839BAB);
  for(int side:{-1,1})for(int z:{-170,170}){
   auto* wheel=bank.object();
   for(int i=0;i<6;++i){float a=i*2*pi/6,b=(i+1)*2*pi/6;int first=int(wheel->vertices.size());wheel->addVertex({{side*20,0,0}});wheel->addVertex({{side*20,int(48*std::cos(a)),int(48*std::sin(a))}});wheel->addVertex({{side*20,int(48*std::cos(b)),int(48*std::sin(b))}});wheel->addTriangle(first,first+1,first+2,rubber);}
   quad(wheel,{side*21,-21,-21},{side*21,21,-21},{side*21,21,21},{side*21,-21,21},rim);
   for(int end:{-1,1})quad(wheel,{-18,-24,end*42},{18,-24,end*42},{18,24,end*42},{-18,24,end*42},rubber);
   bank.finish(wheel);add(wheel,{side*126,50,z},side<0?2:3);
  }
  if(police){
   for(int side:{-1,1}){
    auto* door=bank.object();quad(door,{side*125,70,-112},{side*125,70,72},{side*119,110,72},{side*119,110,-112},trim);bank.finish(door);add(door);
    auto* badge=bank.paint(0xA9DCDA);add(panel({side*127,77,-38},{side*127,77,0},{side*124,100,0},{side*124,100,-38},badge));
   }
   add(box(-45,roof+14,-40,80,17,37,bank.paint(0xFF294B)),{-45,roof+14,-40},5);
   add(box(45,roof+14,-40,80,17,37,bank.paint(0x289CFF)),{45,roof+14,-40},6);
  }
 }

 void build(bool police=false,bool lite=false,bool transformable=false,int style=0){
  parts.clear();wheelGlow=bank.paint(0x54EEFF,0,ShadingMode::ADDITIVE);
  if(lite)trafficBody(police,style);else heroBody(transformable);
  auto* shadow=panel({-116,3,-255},{116,3,-255},{116,3,255},{-116,3,255},bank.paint(0x070E1A));add(shadow,{},11);
 }
 void pose(Vector3 pos,float yaw,float hover=0,float fire=0,float blink=0){
  position=pos;heading=yaw;wheelGlow->alpha=uint8_t(std::min(255.f,hover*190+fire*65));
  for(auto& p:parts){auto* o=p.object;Vector3 local=p.local;int roll=0;o->enabled=true;
   // Outboard faces rotate DOWN, with mirrored hinges on opposite sides.
   if(p.kind==2||p.kind==3){roll=int((p.kind==2?90:-90)*hover);local.x+=int((p.kind==2?-28:28)*hover);local.y+=int(10*hover);}
   if(p.kind==5||p.kind==6)o->enabled=(int(blink*8)%2)==(p.kind==5?0:1);
   o->setPosition(pos+yawed(local,yaw));o->setRotation((p.kind==2||p.kind==3)?int(wheelPhase):0,int(yaw),roll);
   if(p.kind==11){o->position.y=0;o->enabled=pos.y<500;}
   if(p.kind==1){for(size_t i=0;i<o->vertices.size();++i){auto& v=o->vertices[i];v.uv=environmentReflectionUV(pos+yawed(v.position,yaw),yawed(v.normal,yaw),camera.position);if(i%4)v.uv.x=unwrapEnvironmentU(v.uv.x,o->vertices[i-i%4].uv.x);}}
  }
 }
};
}

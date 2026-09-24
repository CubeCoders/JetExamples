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
 Vector3 position;float heading=0;int bodyPitch=0,bodyRoll=0;bool suspension=false;
 // Positive pitch raises the nose. Jet's X rotation uses the opposite sign.
 Vector3 direction(Vector3 v)const{if(bodyRoll){float r=bodyRoll*pi/180;v={int(v.x*std::cos(r)-v.y*std::sin(r)),int(v.x*std::sin(r)+v.y*std::cos(r)),v.z};}float a=bodyPitch*pi/180;return yawed({v.x,int(v.y*std::cos(a)+v.z*std::sin(a)),int(-v.y*std::sin(a)+v.z*std::cos(a))},heading);}
 Vector3 worldPoint(Vector3 v)const{Vector3 pivot{0,suspension?70:0,0};return position+pivot+direction(v-pivot);}
 Vector3 wheelRotation(int spin,int roll)const{
  if(!bodyPitch&&!bodyRoll)return {spin,int(heading),roll};
  roll+=bodyRoll;
  // Compose body yaw/pitch with the pod hinge and wheel spin, then decompose
  // to Jet's Rz*Ry*Rx order. Adding Euler angles would twist the hover pods.
  float y=heading*pi/180,p=bodyPitch*pi/180,r=roll*pi/180,x=spin*pi/180;
  float cy=std::cos(y),sy=std::sin(y),cp=std::cos(p),sp=std::sin(p),cr=std::cos(r),sr=std::sin(r),cx=std::cos(x),sx=std::sin(x);
  float m00=cy*cr-sy*sp*sr,m10=cp*sr,m20=-sy*cr-cy*sp*sr;
  float m21=sy*sr*cx-cy*sp*cr*cx+cy*cp*sx,m22=-sy*sr*sx+cy*sp*cr*sx+cy*cp*cx;
  return {int(std::round(std::atan2(m21,m22)*180/pi)),int(std::round(std::asin(std::clamp(-m20,-1.f,1.f))*180/pi)),int(std::round(std::atan2(m10,m00)*180/pi))};
 }
 void add(Object* o,Vector3 local={0,0,0},int kind=0){o->preciseDepthSort=true;if(local.x==0&&local.y==0&&local.z==0)local=o->position;parts.push_back({o,local,kind});}
 void face(Object* o,Vector3 a,Vector3 b,Vector3 c,Vector3 d,Material* m,Vector3 outward){
  auto u=b-a,v=c-a;
  int64_t facing=(int64_t(u.y)*v.z-int64_t(u.z)*v.y)*outward.x+(int64_t(u.z)*v.x-int64_t(u.x)*v.z)*outward.y+(int64_t(u.x)*v.y-int64_t(u.y)*v.x)*outward.z;
  if(facing<0)quad(o,d,c,b,a,m);else quad(o,a,b,c,d,m);
 }
 void triangle(Object* o,Vector3 a,Vector3 b,Vector3 c,Material* m,Vector3 outward){
  auto u=b-a,v=c-a;
  int64_t facing=(int64_t(u.y)*v.z-int64_t(u.z)*v.y)*outward.x+(int64_t(u.z)*v.x-int64_t(u.x)*v.z)*outward.y+(int64_t(u.x)*v.y-int64_t(u.y)*v.x)*outward.z;
  if(facing<0)std::swap(b,c);
  int n=int(o->vertices.size());o->addVertex({a});o->addVertex({b});o->addVertex({c});o->addTriangle(n,n+1,n+2,m);
 }
 void wheel(int side,int z,bool transformable){
  auto* o=bank.object();auto* rubber=bank.paint(0x111724);auto* alloy=bank.paint(0x8EADCA);auto* spoke=bank.paint(0x34475B);
  constexpr int segments=8;
  auto point=[side](float a,int radius,int x){return Vector3{side*x,int(radius*std::cos(a)),int(radius*std::sin(a))};};
  for(int i=0;i<segments;++i){float a=i*2*pi/segments,b=(i+1)*2*pi/segments,mid=(a+b)*.5f;
   Vector3 radial{0,int(1024*std::cos(mid)),int(1024*std::sin(mid))};
   // Every ring shares its boundary with the next: tread, two sidewalls,
   // outer alloy disc and inner cap form a closed wheel even in flight.
   face(o,point(a,44,-14),point(b,44,-14),point(b,44,14),point(a,44,14),rubber,radial);
   face(o,point(a,44,-14),point(a,35,-17),point(b,35,-17),point(b,44,-14),rubber,{-side,0,0});
   triangle(o,{side*-17,0,0},point(a,35,-17),point(b,35,-17),rubber,{-side,0,0});
   face(o,point(a,44,14),point(b,44,14),point(b,35,17),point(a,35,17),rubber,{side,0,0});
   face(o,point(a,35,17),point(b,35,17),point(b,14,19),point(a,14,19),i%3?alloy:spoke,{side,0,0});
   triangle(o,{side*19,0,0},point(a,14,19),point(b,14,19),spoke,{side,0,0});
  }
  o->cullingMode=CullingMode::CULL_BACKFACES;bank.finish(o);add(o,{side*122,46,z},side<0?2:3);
  if(transformable){auto* ring=bank.object();for(int i=0;i<segments;++i){float a=i*2*pi/segments,b=(i+1)*2*pi/segments;quad(ring,point(a,30,23),point(b,30,23),point(b,22,23),point(a,22,23),wheelGlow);}bank.finish(ring);add(ring,{side*122,46,z},side<0?2:3);}
 }
 void heroBody(bool transformable){
  metal=bank.paint(0x7899CB,255,ShadingMode::PHONG);metal->diffuse=165;metal->specular=230;metal->specularExponent=32;
  auto* edge=bank.paint(0x465E85,255,ShadingMode::PHONG);edge->diffuse=175;edge->specular=190;edge->specularExponent=32;
  auto* trim=bank.paint(0x122234);auto* lamps=bank.paint(0xB5F8FF);auto* red=bank.paint(0xFF395D);
  auto* body=bank.object();body->cullingMode=CullingMode::CULL_BACKFACES;
  const int shapeZ[]={-285,-225,-110,95,205,285},shapeW[]={108,132,128,124,128,110},shapeY[]={98,117,111,104,94,66};
  auto shape=[&](int z){int i=0;while(i<4&&z>shapeZ[i+1])++i;float q=float(z-shapeZ[i])/(shapeZ[i+1]-shapeZ[i]);return std::pair<int,int>{int(shapeW[i]+(shapeW[i+1]-shapeW[i])*q),int(shapeY[i]+(shapeY[i+1]-shapeY[i])*q)};};
  const int zs[]={-285,-225,-213,-175,-137,-125,125,137,175,213,225,285};
  const int arches[]={38,38,82,109,82,38,38,80,101,80,38,38};
  constexpr int sections=sizeof(zs)/sizeof(zs[0]);
  auto ring=[&](int i,Vector3* p){auto [w,y]=shape(zs[i]);int z=zs[i],lip=std::min(arches[i],y-5);
   const Vector3 crossSection[]={{-w+18,y+8,z},{w-18,y+8,z},{w,y,z},{w,lip,z},{85,lip,z},{85,26,z},{-85,26,z},{-85,lip,z},{-w,lip,z},{-w,y,z}};
   std::copy(std::begin(crossSection),std::end(crossSection),p);
  };
  // A single closed loft: wheel recesses have ceilings and inner walls,
  // while a continuous central floor closes the underside of the chassis.
  const Vector3 outward[]={{0,1,0},{1,1,0},{1,0,0},{0,-1,0},{1,0,0},{0,-1,0},{-1,0,0},{0,-1,0},{-1,0,0},{-1,1,0}};
  Vector3 a[10],b[10];ring(0,a);
  for(int i=1;i<sections;++i){ring(i,b);for(int j=0;j<10;++j){auto* mat=j==0?metal:(j==1||j==9)?edge:(j==2||j==8)?metal:trim;face(body,a[j],b[j],b[(j+1)%10],a[(j+1)%10],mat,outward[j]);}std::copy(std::begin(b),std::end(b),a);}
  for(int end:{0,sections-1}){ring(end,a);Vector3 center{0,65,zs[end]},normal{0,0,end?1:-1};for(int j=0;j<10;++j)triangle(body,center,a[j],a[(j+1)%10],trim,normal);}
  body->computeFlatNormals();bank.finish(body);add(body);

  // The canopy is another closed volume. All four glass panels are inset
  // into frames derived from the SAME corners, so roof, pillars and glass
  // meet exactly instead of approximating independent silhouettes.
  auto* canopy=bank.object();canopy->cullingMode=CullingMode::CULL_BACKFACES;
  auto* windows=bank.object();windows->cullingMode=CullingMode::CULL_BACKFACES;
  auto* glass=bank.texture(&env);
  const Vector3 rearL{-98,118,-170},rearR{98,118,-170},frontL{-104,106,121},frontR{104,106,121};
  const Vector3 roofRearL{-84,157,-92},roofRearR{84,157,-92},roofFrontL{-80,160,30},roofFrontR{80,160,30};
  auto glazed=[&](Vector3 p0,Vector3 p1,Vector3 p2,Vector3 p3,Vector3 normal){
   Vector3 outer[]={p0,p1,p2,p3},inner[4];
   for(int i=0;i<4;++i){auto p=outer[i],u=outer[(i+1)%4]-p,v=outer[(i+3)%4]-p;inner[i]=p+(u+v).divide(12);}
   for(int i=0;i<4;++i){int j=(i+1)%4;face(canopy,outer[i],outer[j],inner[j],inner[i],edge,normal);}
   face(windows,inner[0],inner[1],inner[2],inner[3],glass,normal);
  };
  glazed(frontL,frontR,roofFrontR,roofFrontL,{0,1,1});
  glazed(rearR,rearL,roofRearL,roofRearR,{0,1,-1});
  glazed(rearL,frontL,roofFrontL,roofRearL,{-1,1,0});
  glazed(frontR,rearR,roofRearR,roofFrontR,{1,1,0});
  face(canopy,roofRearL,roofRearR,roofFrontR,roofFrontL,metal,{0,1,0});
  face(canopy,rearL,rearR,frontR,frontL,trim,{0,-1,0});
  canopy->computeFlatNormals();bank.finish(canopy);add(canopy);
  windows->computeFlatNormals();bank.finish(windows);add(windows,{},1);

  // Intentional surface decals are separate from the closed structural mesh.
  auto* details=bank.object();details->cullingMode=CullingMode::CULL_BACKFACES;
  for(int s:{-1,1}){
   auto sideAt=[&](int z,int y){return Vector3{s*(shape(z).first+1),y,z};};
   face(details,sideAt(-117,38),sideAt(115,38),sideAt(115,44),sideAt(-117,44),trim,{s,0,0});
   face(details,sideAt(-78,46),sideAt(-78,108),sideAt(-75,108),sideAt(-75,46),trim,{s,0,0});
   face(details,sideAt(-61,93),sideAt(-29,93),sideAt(-29,97),sideAt(-61,97),trim,{s,0,0});
   face(details,{s*108,51,286},{s*43,51,286},{s*43,58,286},{s*108,58,286},lamps,{0,0,1});
  }
  face(details,{-100,83,-286},{100,83,-286},{100,90,-286},{-100,90,-286},red,{0,0,-1});
  bank.finish(details);add(details,{},12);
  for(int s:{-1,1})for(int z:{-175,175})wheel(s,z,transformable);
 }
 void trafficBody(bool police,int style){
  const unsigned colors[]={0x94776C,0x547789,0xBDC2AA,0x785585,0x355665,0xB88542};
  metal=bank.paint(police?0xD4DCDF:colors[style%6]);auto* trim=bank.paint(0x172532);
  // Closed, recessed wheel wells. The old uninterrupted side quads passed
  // through the tyres and made their large depth gradients hard to order.
  auto* chassis=bank.object();chassis->cullingMode=CullingMode::CULL_BACKFACES;
  const int zs[]={-280,-225,-202,-138,-115,115,138,202,225,280};
  const int lips[]={40,40,104,104,40,40,104,104,40,40};
  auto ring=[&](int i,Vector3* points){int z=zs[i],lip=lips[i];
   const Vector3 section[]={{-114,118,z},{114,118,z},{130,106,z},{130,lip,z},{96,lip,z},{96,36,z},{-96,36,z},{-96,lip,z},{-130,lip,z},{-130,106,z}};
   std::copy(std::begin(section),std::end(section),points);
  };
  const Vector3 outward[]={{0,1,0},{1,1,0},{1,0,0},{0,-1,0},{1,0,0},{0,-1,0},{-1,0,0},{0,-1,0},{-1,0,0},{-1,1,0}};
  Vector3 a[10],b[10];ring(0,a);
  for(int i=1;i<10;++i){ring(i,b);for(int j=0;j<10;++j)face(chassis,a[j],b[j],b[(j+1)%10],a[(j+1)%10],j<=2||j>=8?metal:trim,outward[j]);std::copy(std::begin(b),std::end(b),a);}
  for(int end:{0,9}){ring(end,a);Vector3 center{0,75,zs[end]};for(int j=0;j<10;++j)triangle(chassis,center,a[j],a[(j+1)%10],trim,{0,0,end?1:-1});}
  bank.finish(chassis);add(chassis);
  auto* body=bank.object();
  auto* glass=bank.paint(police?0x25495F:0x2E5768);int roof=style%2?204:178;
  quad(body,{-101,113,-170},{101,113,-170},{82,roof,-105},{-82,roof,-105},glass);
  quad(body,{-101,108,165},{-82,roof,12},{82,roof,12},{101,108,165},glass);
  quad(body,{-101,113,-170},{-82,roof,-105},{-82,roof,12},{-101,108,165},glass);
  quad(body,{101,108,165},{82,roof,12},{82,roof,-105},{101,113,-170},glass);
  quad(body,{-82,roof,-105},{82,roof,-105},{82,roof,12},{-82,roof,12},metal);
  // Door belt, B pillars, handles, bumper grilles and paired lamps.
  for(int side:{-1,1}){
   quad(body,{side*131,60,-108},{side*131,60,108},{side*131,70,108},{side*131,70,-108},trim);
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
 void pose(Vector3 pos,float yaw,float hover=0,float fire=0,float blink=0,float pitch=0,float lean=0){
  position=pos;heading=yaw;bodyPitch=int(std::round(pitch));bodyRoll=int(std::round(lean));suspension=hover==0&&(bodyPitch||bodyRoll);wheelGlow->alpha=uint8_t(std::min(255.f,hover*190+fire*65));
  for(auto& p:parts){auto* o=p.object;Vector3 local=p.local;int roll=0;o->enabled=true;
   // Outboard faces rotate DOWN, with mirrored hinges on opposite sides.
   if(p.kind==2||p.kind==3){roll=int((p.kind==2?90:-90)*hover);local.x+=int((p.kind==2?-28:28)*hover);local.y+=int(10*hover);}
   if(p.kind==5||p.kind==6)o->enabled=(int(blink*8)%2)==(p.kind==5?0:1);
   bool wheel=p.kind==2||p.kind==3;
   o->setPosition(wheel&&suspension?pos+yawed(local,yaw):worldPoint(local));
   o->setRotation(wheel?(suspension?Vector3{int(wheelPhase),int(yaw),0}:wheelRotation(int(wheelPhase),roll)):(bodyRoll?wheelRotation(0,0):Vector3{-bodyPitch,int(yaw),0}));
   if(p.kind==11){o->setPosition(pos);o->setRotation(0,int(yaw),0);o->position.y=0;o->enabled=pos.y<500;}
   if(p.kind==1){for(size_t i=0;i<o->vertices.size();++i){auto& v=o->vertices[i];v.uv=environmentReflectionUV(worldPoint(v.position),direction(v.normal),camera.position);if(i%4)v.uv.x=unwrapEnvironmentU(v.uv.x,o->vertices[i-i%4].uv.x);}}
  }
 }
};
}

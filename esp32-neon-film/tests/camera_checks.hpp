#pragma once
#include "Film.hpp"
#include <cassert>
inline void checkCameraClearance(float seconds){
 using namespace Film;
 for(auto* object:architecture){
  auto inside=[&](Vector3 point,int margin){
   auto local=yawed(point-object->position,-object->rotation.y);
   return local.x>object->boundingBoxMin.x-margin && local.x<object->boundingBoxMax.x+margin &&
          local.y>object->boundingBoxMin.y-margin && local.y<object->boundingBoxMax.y+margin &&
          local.z>object->boundingBoxMin.z-margin && local.z<object->boundingBoxMax.z+margin;
  };
  if(inside(camera.position,45)){std::printf("Camera inside building at %.2f, camera %d %d %d, building %d %d %d\n",seconds,camera.position.x,camera.position.y,camera.position.z,object->position.x,object->position.y,object->position.z);std::fflush(stdout);assert(false);}
  if(!hero.parts.empty())for(int i=1;i<24;++i)if(inside(lerp(camera.position,hero.position+Vector3{0,80,0},i/24.f),0)){
   std::printf("Building blocks hero at %.2f\n",seconds);std::fflush(stdout);assert(false);
  }
 }
}

// Test the actual street triangles, including projecting signs and awnings
// folded into a combined detail mesh. A 60-unit sphere covers the near plane.
inline void checkStreetFurniture(float seconds){
 using namespace Film;
 struct V{double x,y,z;V operator-(V b)const{return {x-b.x,y-b.y,z-b.z};}V operator+(V b)const{return {x+b.x,y+b.y,z+b.z};}V operator*(double s)const{return {x*s,y*s,z*s};}};
 auto dot=[](V a,V b){return a.x*b.x+a.y*b.y+a.z*b.z;};
 auto cross=[](V a,V b){return V{a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};};
 auto vector=[](Vector3 a){return V{double(a.x),double(a.y),double(a.z)};};
 auto inspect=[&](Object* o){
  if(!o->enabled || o->noWriteZBuffer)return;
  auto local=yawed(camera.position-o->position,-o->rotation.y);constexpr int clearance=60;
  auto lo=o->boundingBoxMin,hi=o->boundingBoxMax;
  if(local.x<lo.x-clearance || local.x>hi.x+clearance || local.y<lo.y-clearance || local.y>hi.y+clearance || local.z<lo.z-clearance || local.z>hi.z+clearance)return;
  V eye=vector(local);
  for(auto& t:o->triangles){V a=vector(o->vertices[t.v1].position),b=vector(o->vertices[t.v2].position),c=vector(o->vertices[t.v3].position),ab=b-a,ac=c-a,n=cross(ab,ac);
   double nn=dot(n,n);if(nn<1)continue;
   double plane=dot(eye-a,n);V p=eye-n*(plane/nn);double distance=1e30;
   double d00=dot(ab,ab),d01=dot(ab,ac),d11=dot(ac,ac),d20=dot(p-a,ab),d21=dot(p-a,ac),den=d00*d11-d01*d01;
   double u=(d11*d20-d01*d21)/den,v=(d00*d21-d01*d20)/den;
   if(u>=0 && v>=0 && u+v<=1)distance=plane*plane/nn;
   V points[]={a,b,c};for(int j=0;j<3;++j){V start=points[j],edge=points[(j+1)%3]-start;double ee=dot(edge,edge);if(ee==0)continue;V delta=eye-(start+edge*std::clamp(dot(eye-start,edge)/ee,0.0,1.0));distance=std::min(distance,dot(delta,delta));}
   if(distance<clearance*clearance){std::fprintf(stderr,"Camera hits street detail at %.3f, camera %d %d %d, local triangle (%g,%g,%g) (%g,%g,%g) (%g,%g,%g), distance %.1f\n",seconds,camera.position.x,camera.position.y,camera.position.z,a.x,a.y,a.z,b.x,b.y,b.z,c.x,c.y,c.z,std::sqrt(distance));assert(false);}
  }
 };
 if(shot==1){for(auto& o:bank.objects)inspect(o.get());}
 else {for(auto& i:road.items)inspect(i.object);if(shot==6)for(auto& i:scrolling)inspect(i.first);}
}

inline bool overlapCars(const Film::Vehicle& a,const Film::Vehicle& b){
 using namespace Film;
 auto axis=[](float angle){float a=angle*pi/180;return std::pair<float,float>{std::cos(a),-std::sin(a)};};
 auto ax=axis(a.heading),az=axis(a.heading-90),bx=axis(b.heading),bz=axis(b.heading-90);
 auto dot=[](auto x,auto y){return x.first*y.first+x.second*y.second;};
 std::pair<float,float> distance{float(b.position.x-a.position.x),float(b.position.z-a.position.z)};
 for(auto normal:{ax,az,bx,bz}){
  float radiusA=125*std::abs(dot(ax,normal))+300*std::abs(dot(az,normal));
  float radiusB=125*std::abs(dot(bx,normal))+280*std::abs(dot(bz,normal));
  if(std::abs(dot(distance,normal))>=radiusA+radiusB)return false;
 }
 return true;
}
inline void checkTraffic(float seconds){
 using namespace Film;
 if(shot!=6&&shot!=7)return;
 for(int i=0;i<6;++i)if(overlapCars(hero,traffic[i])){
  std::printf("Hero clips traffic %d at %.2f, hero %d %d, traffic %d %d\n",i,seconds,hero.position.x,hero.position.z,traffic[i].position.x,traffic[i].position.z);std::fflush(stdout);assert(false);
 }
}

inline void checkVehicleCameras(float seconds){
 using namespace Film;
 auto check=[&](const Vehicle& car){if(car.parts.empty()||!car.parts.front().object->enabled)return;Vector3 pivot{0,car.suspension?70:0,0};auto local=yawed(camera.position-car.position-pivot,-car.heading);float a=car.bodyPitch*pi/180;local={local.x,int(local.y*std::cos(a)-local.z*std::sin(a)),int(local.y*std::sin(a)+local.z*std::cos(a))};
  float r=-car.bodyRoll*pi/180;local={int(local.x*std::cos(r)-local.y*std::sin(r)),int(local.x*std::sin(r)+local.y*std::cos(r)),local.z};local=local+pivot;
  if(std::abs(local.x)<205 && std::abs(local.z)<330 && local.y>-25 && local.y<230){std::fprintf(stderr,"Camera clips vehicle at %.2f: local %d %d %d\n",seconds,local.x,local.y,local.z);assert(false);}
 };
 check(hero);for(auto& car:traffic)check(car);for(auto& car:police)check(car);
 if(shot==7||(shot==6&&shotTime>=exitTrackingStart)){assert(!overlapCars(police[0],police[1]));for(auto& cop:police)assert(!overlapCars(hero,cop));}
 if(shot==7||(shot==6&&shotTime>=exitTrackingStart))for(auto& cop:police)for(auto& car:traffic)if(overlapCars(cop,car)){std::fprintf(stderr,"Police clips traffic at %.2f\n",seconds);assert(false);}
}

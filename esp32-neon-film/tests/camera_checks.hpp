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
 auto check=[&](const Vehicle& car){if(car.parts.empty())return;auto local=yawed(camera.position-car.position,-car.heading);
  if(std::abs(local.x)<165 && std::abs(local.z)<330 && local.y>-25 && local.y<230){std::fprintf(stderr,"Camera clips vehicle at %.2f: local %d %d %d\n",seconds,local.x,local.y,local.z);assert(false);}
 };
 check(hero);for(auto& car:traffic)check(car);for(auto& car:police)check(car);
 if(shot==7){assert(!overlapCars(police[0],police[1]));for(auto& cop:police)assert(!overlapCars(hero,cop));}
 if(shot==7)for(auto& cop:police)for(auto& car:traffic)if(overlapCars(cop,car)){std::fprintf(stderr,"Police clips traffic at %.2f\n",seconds);assert(false);}
}

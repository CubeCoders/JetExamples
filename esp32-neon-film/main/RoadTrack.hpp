#pragma once
#include "City.hpp"
namespace Film {
// Recycle the authored street behind the tracking rig. Cars retain a stable
// local origin; street parallax is driven by distance travelled, not frame rate.
struct RoadTrack {
 struct Item {Object* object;Vector3 origin;int center;};
 std::vector<Item> items;
 std::vector<Vector3> haloOrigins;
 int period=0,rearExtent=2400;
 void clear(){items.clear();haloOrigins.clear();period=0;}
 void capture(int length){
  clear();period=length;
  for(auto& o:bank.objects){
   if(o->boundingBoxMax.z-o->boundingBoxMin.z>length)continue; // continuous road
   items.push_back({o.get(),o->position,o->position.z+o->centreVolume.z});
  }
  for(auto& g:glows)haloOrigins.push_back(g.position);
 }
 int offset(float travel,int center)const{
  int distance=int(travel);return -distance+int(std::floor(float(distance-center+period-rearExtent)/period))*period;
 }
 void advance(float distance,bool lookingBack=false){
  // Reserve the long end of the recycled street for the direction of the shot.
  // The extra 2400 units behind the rig keep nearby geometry clear of recycling.
  rearExtent=lookingBack?period-2400:2400;
  for(auto& i:items)i.object->position=i.origin+Vector3{0,0,offset(distance,i.center)};
  for(size_t i=0;i<haloOrigins.size();++i)glows[i].position=haloOrigins[i]+Vector3{0,0,offset(distance,haloOrigins[i].z)};
 }
};
inline RoadTrack road;
struct LaneChange {float from,to,start,seconds;};
// Easing here describes steering, not camera motion. The derivative drives
// the body's yaw, keeping the wheels and nose aligned with its actual velocity.
inline float lanePosition(float t,const LaneChange& c){float u=clamp((t-c.start)/c.seconds);return c.from+(c.to-c.from)*u*u*(3-2*u);}
inline float laneVelocity(float t,const LaneChange& c){float u=clamp((t-c.start)/c.seconds);return (c.to-c.from)*6*u*(1-u)/c.seconds;}
inline LaneChange chaseLane(float t){
 if(t<4.2f)return {-380,0,.6f,1.2f};
 if(t<8.2f)return {0,380,4.2f,1.4f};
 return {380,0,8.2f,1.4f};
}
inline LaneChange cornerLane(float t){
 if(t<2.f)return {-380,0,.25f,1.15f};
 if(t<3.35f)return {0,380,2.f,1.1f};
 return {380,0,3.35f,.8f};
}
inline float steeringYaw(float lateral,float forward){return std::atan2(lateral,forward)*180/pi;}
}

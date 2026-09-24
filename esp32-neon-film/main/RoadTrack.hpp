#pragma once
#include "City.hpp"
namespace Film {
// Recycle the authored street behind the tracking rig. Cars retain a stable
// local origin; street parallax is driven by distance travelled, not frame rate.
struct RoadTrack {
 struct Item {Object* object;Vector3 origin;int center;};
 std::vector<Item> items;
 std::vector<Vector3> haloOrigins;
 int period=0,rearExtent=2400;size_t firstHalo=0;Vector3 axis{0,0,1};
 void clear(){items.clear();haloOrigins.clear();period=0;}
 void capture(int length,size_t firstObject=0,size_t firstGlow=0,bool eastbound=false){
  clear();period=length;firstHalo=firstGlow;axis=eastbound?Vector3{1,0,0}:Vector3{0,0,1};
  for(size_t n=firstObject;n<bank.objects.size();++n){auto& o=bank.objects[n];
   if(o->boundingBoxMax.z-o->boundingBoxMin.z>length)continue; // continuous road
   auto center=o->position+yawed(o->centreVolume,o->rotation.y);
   items.push_back({o.get(),o->position,eastbound?center.x:center.z});
  }
  for(size_t n=firstGlow;n<glows.size();++n)haloOrigins.push_back(glows[n].position);
 }
 int offset(float travel,int center)const{
  int distance=int(travel);return -distance+int(std::floor(float(distance-center+period-rearExtent)/period))*period;
 }
 void restore(){
  for(auto& i:items)i.object->position=i.origin;
  for(size_t i=0;i<haloOrigins.size();++i)glows[firstHalo+i].position=haloOrigins[i];
 }
 void advance(float distance,bool lookingBack=false){
  // Reserve the long end of the recycled street for the direction of the shot.
  // The extra 2400 units behind the rig keep nearby geometry clear of recycling.
  rearExtent=lookingBack?period-2400:2400;
  for(auto& i:items)i.object->position=i.origin+axis*offset(distance,i.center);
  for(size_t i=0;i<haloOrigins.size();++i)glows[firstHalo+i].position=haloOrigins[i]+axis*offset(distance,axis.x?haloOrigins[i].x:haloOrigins[i].z);
 }
};
inline RoadTrack road;
// The 570-unit coupe represents a 4.5 m car. MPH is converted once to the
// same world units used by road parallax, wheel rotation and flight velocity.
inline constexpr float carLength=570.f,metresPerCar=4.5f;
inline constexpr float unitsPerMetre=carLength/metresPerCar;
inline constexpr float speedFromMph(float mph){return mph*.44704f*unitsPerMetre;}
inline constexpr float roadSpeed=speedFromMph(70),chaseSpeed=speedFromMph(88);
inline constexpr float climbSpeed=650,climbRamp=.8f;
inline float launchHeight(float t){return 55+climbSpeed*(t<climbRamp?t*t/(2*climbRamp):t-climbRamp/2);}
inline float launchPitch(float t){return std::atan2(climbSpeed*clamp(t/climbRamp),chaseSpeed)*180/pi;}
inline constexpr float turnStart=2,turnEnd=3.15f,turnRadius=1030;
struct LaneChange {float from,to,start,seconds;};
// Easing here describes steering, not camera motion. The derivative drives
// the body's yaw, keeping the wheels and nose aligned with its actual velocity.
inline float lanePosition(float t,const LaneChange& c){float u=clamp((t-c.start)/c.seconds);return c.from+(c.to-c.from)*u*u*(3-2*u);}
inline float laneVelocity(float t,const LaneChange& c){float u=clamp((t-c.start)/c.seconds);return (c.to-c.from)*6*u*(1-u)/c.seconds;}
inline LaneChange chaseLane(float t){
 if(t<4.5f)return {-380,0,.6f,1.2f};
 if(t<8.2f)return {0,380,4.5f,1.4f};
 return {380,0,8.2f,1.4f};
}
inline LaneChange cornerLane(float t){
 if(t<1.8f)return {-380,0,.25f,1.15f};
 if(t<3.8f)return {0,380,1.8f,1.f};
 return {380,0,3.8f,.85f};
}
inline float steeringYaw(float lateral,float forward){return std::atan2(lateral,forward)*180/pi;}
}

#include "Vehicle.hpp"
#include <cassert>
#include <map>
#include <tuple>
#include <vector>
#include <cstdio>
using namespace Film;
using Point=std::tuple<int,int,int>;
using Edge=std::pair<Point,Point>;

int main(){
 std::vector<uint16_t> pixels(480*320/4);Scene testScene(pixels.data(),nullptr,480,320);scene=&testScene;scene->setCamera(&camera);
 for(bool flying:{false,true}){
  Vehicle car;car.build(false,false,flying);car.pose({0,0,0},0);
  struct Use{int count=0,direction=0;};std::map<Edge,Use> edges;size_t triangles=0;
  auto point=[](Vector3 p){return Point{p.x,p.y,p.z};};
  for(const auto& part:car.parts){
   if(part.kind==11||part.kind==12)continue; // Shadow and intentional surface decals.
   auto* object=part.object;
   for(const auto& t:object->triangles){
    if(t.material->shadingMode==ShadingMode::ADDITIVE)continue; // Open glow rings.
    Vector3 p[]={object->vertices[t.v1].position+object->position,object->vertices[t.v2].position+object->position,object->vertices[t.v3].position+object->position};
    auto u=p[1]-p[0],v=p[2]-p[0];
    assert(int64_t(u.y)*v.z!=int64_t(u.z)*v.y || int64_t(u.z)*v.x!=int64_t(u.x)*v.z || int64_t(u.x)*v.y!=int64_t(u.y)*v.x);
    for(int i=0;i<3;++i){Point a=point(p[i]),b=point(p[(i+1)%3]);int direction=1;if(b<a){std::swap(a,b);direction=-1;}auto& use=edges[{a,b}];++use.count;use.direction+=direction;}
    ++triangles;
   }
  }
  unsigned bad=0;for(const auto& [edge,use]:edges)if(use.count!=2||use.direction!=0){
   const auto [x,y,z]=edge.first;const auto [a,b,c]=edge.second;
   if(bad<12)std::printf("Open/misoriented edge (%d,%d,%d)-(%d,%d,%d): %d uses, direction %d\n",x,y,z,a,b,c,use.count,use.direction);++bad;
  }
  std::printf("%s: %zu structural triangles, %zu welded edges, %u invalid edges\n",flying?"Hover coupe":"Road coupe",triangles,edges.size(),bad);std::fflush(stdout);assert(!bad);
  bank.clear();
 }
}

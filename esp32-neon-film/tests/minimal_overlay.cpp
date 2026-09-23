#include "PerformanceOverlay.hpp"
#include <cassert>
#include <array>
int main(){
 std::array<uint16_t,480*320/4> pixels{};
 Renderer::Scene scene(pixels.data(),nullptr,480,320);
 PerformanceOverlay hud;hud.attach(scene,480);
 auto* sprite=scene.getSprites().back();auto* texture=sprite->material->diffuseMap;
 assert(sprite->x==463&&sprite->y==0&&texture->width==17&&texture->height==7&&texture->hasAlpha);
 hud.tick(0,0,0);for(int i=1;i<=30;++i)hud.tick(i*16778,123,5000);
 assert(hud.fpsTenths()==596); // 59.6 must round to "60", not truncate to 59.
 // The first glyph is padding. The last glyph is the familiar 5x7 zero.
 const uint8_t zero[]={0x3e,0x51,0x49,0x45,0x3e};
 for(int y=0;y<7;++y){
  for(int x=0;x<6;++x)assert(texture->data[y*17+x]==0);
  for(int x=0;x<5;++x)assert(texture->data[y*17+12+x]==((zero[x]&(1<<y))?0xffff:0));
 }
}

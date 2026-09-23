#pragma once
#include "Scene.hpp"
#include "Display.hpp"
#include "Assets.hpp"
#include "Labels.hpp"
#include <cmath>
#include <cstdio>
namespace Courier {
using namespace Renderer;
inline constexpr float pi=3.14159265359f;
inline Scene* scene=nullptr;
inline Camera camera;
inline float time=0;
inline int stage=-1;
inline uint16_t sky[Display::RENDER_HEIGHT];
inline Texture shipTexture(64,32,const_cast<uint16_t*>(Assets::ship),true,0);
inline Material shipMaterial(0xffff,&shipTexture),trailMaterial(0xffff,&shipTexture);
inline Material black(0,uint8_t(255)),fadeMaterial(0,uint8_t(0));
inline Sprite2D ship,trails[4],top,bottom,fade;
inline Object* stripes[14];
inline Object* fields[14];
inline uint16_t rgb(unsigned c){return uint16_t(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));}
inline Material* paint(unsigned c){auto* m=new Material(rgb(c));m->shadingMode=ShadingMode::UNLIT;return m;}
inline Object* mesh(){auto* o=new Object;o->noWriteZBuffer=true;o->cullingMode=CullingMode::NO_CULLING;return o;}
inline void finish(Object* o){o->calculateBoundingBox();o->cachePositions();scene->addObject(o);}
inline void quad(Object* o,Vector3 a,Vector3 b,Vector3 c,Vector3 d,Material* m){int n=int(o->vertices.size());for(auto v:{a,b,c,d})o->addVertex({v,{0,0},{0,1024,0}});o->addFace(n,n+1,n+2,n+3,m);}
inline void place(Sprite2D& s,float t) {
 const float a=t*2*pi/7;
 s.x=176+int(145*std::sin(a));s.y=143+int(22*std::sin(2*a));
 s.textureFlags=std::cos(a)<0?Sprite2D::FLIP_X:0;
}
inline void update(float dt){
 time=std::fmod(time+dt,28.f);const int next=int(time/7);const float phase=std::fmod(time,7.f);
 if(next!=stage){stage=next;Labels::select(stage);const char* names[]={"DIRECTION / FLIP X","INVERT / FLIP Y","ECHO / COMBINED ALPHA","CINEMA / BARS + FADE"};std::printf("Courier: %s\n",names[stage]);}
 camera.setPosition(int(35*std::sin(time*2*pi/28)),190,-600);camera.lookAt({0,120,900});
 for(int i=0;i<14;++i){const int z=int(std::fmod(i*180.f-time*180.f+7560.f,2520.f))-500;stripes[i]->setPosition(0,0,z);fields[i]->setPosition(0,0,z);}
 place(ship,time);ship.alpha=255;
 if(stage==1 && (int(phase/1.75f)&1))ship.textureFlags|=Sprite2D::FLIP_Y;
 // One group opacity multiplied by each echo's individual opacity.
 trailMaterial.alpha=uint8_t(140+115*(.5f+.5f*std::sin(phase*2*pi/7)));
 for(int i=0;i<4;++i){auto& s=trails[i];s.enabled=stage==2;place(s,time-(4-i)*.23f);s.alpha=uint8_t(35+i*42);}
 const float bars=stage==3?std::min(1.f,std::min(phase,7.f-phase)):0;
 top.height=int(70*bars);bottom.height=int(43*bars);bottom.y=320-bottom.height;
 top.enabled=bottom.enabled=stage==3;
 const float f=stage==3&&phase>4?std::sin((phase-4)*pi/3):0;
 setSolidRectAlpha(fade,uint8_t(255*f*f));
}
inline void init(Scene& target){
 scene=&target;camera.setFOV(58.f,Display::RENDER_WIDTH);camera.nearPlane=40;camera.farPlane=6000;scene->setCamera(&camera);scene->setClearBuffer(true);
 for(int y=0;y<Display::RENDER_HEIGHT;++y)sky[y]=rgb(((91+y*83/320)<<16)|((155+y*57/320)<<8)|(200+y*30/320));
 scene->backgroundGradientColors=sky;
 // A warm daylight sun behind the distant hills.
 auto* sun=mesh();auto* sunPaint=paint(0xFFF1C3);
 for(int y=-210;y<210;y+=20){int x=int(std::sqrt(float(210*210-y*y)));int yy=std::min(210,y+20);int xx=int(std::sqrt(float(210*210-yy*yy)));quad(sun,{-x,560+y,2600},{x,560+y,2600},{xx,560+yy,2600},{-xx,560+yy,2600},sunPaint);}finish(sun);
 auto* mountains=mesh();auto* dark=paint(0x567E75);auto* light=paint(0x84A08A);
 for(int i=0;i<12;++i){int x=-2400+i*400,h=230+(i*137)%420,n=int(mountains->vertices.size());for(auto v:{Vector3{x,0,1950},Vector3{x+260,h,1950},Vector3{x+520,0,1950},Vector3{x+260,0,1870}})mountains->addVertex({v});mountains->addTriangle(n,n+1,n+3,dark);mountains->addTriangle(n+1,n+2,n+3,light);}finish(mountains);
 auto* floor=mesh();quad(floor,{-4000,0,-500},{4000,0,-500},{4000,0,4000},{-4000,0,4000},paint(0x73934D));finish(floor);
 // Rectangular crop plots separated by earthen access tracks.
 Material* crops[]={paint(0x8EA35B),paint(0xA7AA61),paint(0x628547),paint(0xC0AF71)};
 for(int i=0;i<14;++i){fields[i]=mesh();for(int x=-1600,j=0;x<1600;x+=200,++j)quad(fields[i],{x+9,1,8},{x+191,1,8},{x+191,1,172},{x+9,1,172},crops[(i*3+j)%4]);finish(fields[i]);}
 auto* grid=paint(0xB4B56E);auto* rails=mesh();for(int x=-1600;x<=1600;x+=200)quad(rails,{x-6,1,-500},{x+6,1,-500},{x+6,1,2500},{x-6,1,2500},grid);finish(rails);
 for(int i=0;i<14;++i){stripes[i]=mesh();quad(stripes[i],{-1800,2,-3},{1800,2,-3},{1800,2,3},{-1800,2,3},grid);finish(stripes[i]);}
 for(int i=0;i<4;++i){auto& s=trails[i];s.material=&trailMaterial;s.scale=2;s.zOrder=10+i;scene->addSprite(&s);}
 ship.material=&shipMaterial;ship.scale=2;ship.zOrder=20;scene->addSprite(&ship);
 top=makeLetterboxBar(480,0,0,true,&black);bottom=makeLetterboxBar(480,0,0,false,&black);top.zOrder=bottom.zOrder=40;scene->addSprite(&top);scene->addSprite(&bottom);
 fade=makeFullScreenFade(480,320,0,&fadeMaterial);fade.zOrder=45;scene->addSprite(&fade);
 Labels::add(target);update(0);
}
}

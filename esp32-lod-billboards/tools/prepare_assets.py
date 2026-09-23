"""Generate original tree meshes, a matching billboard and immutable captions."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import math,argparse
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--font');a=p.parse_args();r=Path(__file__).resolve().parents[1]
fontpath=a.font or ('C:/Windows/Fonts/consola.ttf' if Path('C:/Windows/Fonts/consola.ttf').exists() else 'DejaVuSansMono.ttf')
def rgb(c):return ((c[0]>>3)<<11)|((c[1]>>2)<<5)|(c[2]>>3)
def array(name,im):
 vals=[rgb(c) for c in im.get_flattened_data()]
 return f'inline const uint16_t {name}[]={{\n'+'\n'.join(','.join(map(str,vals[i:i+24]))+',' for i in range(0,len(vals),24))+'\n};\n'
def tree(lats,lons):
 faces=[]
 def add(a,b,c,base):
  u=[b[i]-a[i] for i in range(3)];v=[c[i]-a[i] for i in range(3)];n=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]];length=math.sqrt(sum(x*x for x in n))
  if length<1:return
  brightness=.57+.43*max(0,sum(n[i]*(-.45,.75,-.48)[i] for i in range(3))/length)
  colour=tuple(int(x*brightness) for x in base)
  faces.append((a,b,c,colour))
 # Trunk: winding is outward, caps omitted beneath canopy and ground.
 for i in range(lons):
  a=i*2*math.pi/lons;b=(i+1)*2*math.pi/lons
  p0=(int(23*math.cos(a)),0,int(23*math.sin(a)));p1=(int(23*math.cos(b)),0,int(23*math.sin(b)))
  p2=(int(14*math.cos(b))+8,330,int(14*math.sin(b)));p3=(int(14*math.cos(a))+8,330,int(14*math.sin(a)))
  add(p0,p2,p1,(133,96,61));add(p0,p3,p2,(133,96,61))
 for cx,cy,cz,rx,ry,rz,base in [(0,355,0,155,150,135,(122,157,60)),(-120,285,-5,120,105,100,(115,152,61)),(130,305,10,110,110,105,(144,167,69)),(15,455,0,110,105,105,(145,175,78)),(0,325,85,140,100,120,(100,143,65))]:
  def vertex(j,i):
   t=j*math.pi/lats;ph=i*2*math.pi/lons
   return (cx+round(rx*math.sin(t)*math.cos(ph)),cy+round(ry*math.cos(t)),cz+round(rz*math.sin(t)*math.sin(ph)))
  for j in range(lats):
   for i in range(lons):
    aa,bb,cc,dd=vertex(j,i),vertex(j+1,i),vertex(j+1,i+1),vertex(j,i+1)
    add(aa,cc,bb,base);add(aa,dd,cc,base)
 return faces
high=tree(6,12);low=tree(3,8)
s='#pragma once\n#include <cstdint>\nnamespace TreeData {\nstruct Face {int16_t p[9];uint16_t colour;};\n'
for name,faces in [('high',high),('low',low)]:
 s+=f'inline const Face {name}[]={{\n'
 for aa,bb,cc,c in faces:s+='{{'+','.join(str(v) for point in (aa,bb,cc) for v in (point[0],point[1]-280,point[2]))+'},'+str(rgb(c))+'},\n'
 s+='};\n'
(r/'main/TreeData.hpp').write_text(s+'}\n')
# Orthographic front-view bake. Matches the exact mesh's triangle colours.
im=Image.new('RGB',(256,320));d=ImageDraw.Draw(im)
for aa,bb,cc,c in sorted(high,key=lambda f:sum(v[2] for v in f[:3]),reverse=True):
 d.polygon([(round(128+v[0]/2),round(160-(v[1]-280)/2)) for v in (aa,bb,cc)],fill=c)
im=im.resize((128,160),Image.Resampling.NEAREST);im.save(r/'assets/oak-billboard.png')
(r/'main/Assets.hpp').write_text('#pragma once\n#include <cstdint>\nnamespace Assets {\n'+array('oak',im.transpose(Image.Transpose.FLIP_TOP_BOTTOM))+'}\n')
labels=[('title','WOODLAND / LOD',310,27,24,(250,240,210)),('high',f'FULL MESH / {len(high)} TRIANGLES',340,19,15,(215,235,186)),('low',f'SIMPLE MESH / {len(low)} TRIANGLES',340,19,15,(246,221,147)),('fade','TRANSITION / MESH + BILLBOARD',340,19,15,(246,221,147)),('billboard','BILLBOARD / 2 TRIANGLES',340,19,15,(174,229,231)),('reference',f'REFERENCE / FULL {len(high)} TRIANGLES',340,19,15,(246,211,185)),('autoDesc','Distance LOD: full mesh > simple mesh > billboard',455,19,13,(232,238,219)),('referenceDesc','Same camera path, full mesh kept at every distance',455,19,13,(232,238,219))]
s='#pragma once\n#include "Scene.hpp"\nnamespace Labels {\n'
for name,text,w,h,size,c in labels:
 im=Image.new('RGB',(w,h));ImageDraw.Draw(im).text((0,-2),text,font=ImageFont.truetype(fontpath,size),fill=c)
 s+=array(name+'Pixels',im)+f'inline Renderer::Texture {name}Texture({w},{h},const_cast<uint16_t*>({name}Pixels),true,0);\n'
s+='''inline Renderer::Material titleMat(0xffff,&titleTexture),modeMat(0xffff,&highTexture),descMat(0xffff,&autoDescTexture),barMat(0x0842,nullptr,nullptr,false,145);
inline Renderer::Sprite2D title,mode,desc,top,bottom;
inline void select(int stage,bool reference){Renderer::Texture* names[]={&highTexture,&lowTexture,&fadeTexture,&billboardTexture,&referenceTexture};modeMat.diffuseMap=names[stage];descMat.diffuseMap=reference?&referenceDescTexture:&autoDescTexture;}
inline void add(Renderer::Scene& scene){
 top=Renderer::makeSolidRect(0,0,480,65,&barMat);top.zOrder=50;scene.addSprite(&top);
 bottom=Renderer::makeSolidRect(0,289,480,31,&barMat);bottom.zOrder=50;scene.addSprite(&bottom);
 title.x=18;title.y=13;title.material=&titleMat;title.zOrder=100;scene.addSprite(&title);
 mode.x=18;mode.y=44;mode.material=&modeMat;mode.zOrder=100;scene.addSprite(&mode);
 desc.x=18;desc.y=299;desc.material=&descMat;desc.zOrder=100;scene.addSprite(&desc);
}
}
'''
(r/'main/Labels.hpp').write_text(s)
print('Tree:',len(high),'high triangles;',len(low),'low triangles; billboard 2 triangles, 40960 bytes')

"""Generate original cafe artwork, quarter-image halos and immutable labels."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import argparse,math
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--font');a=p.parse_args();r=Path(__file__).resolve().parents[1]
fontpath=a.font or ('C:/Windows/Fonts/consola.ttf' if Path('C:/Windows/Fonts/consola.ttf').exists() else 'DejaVuSansMono.ttf')
def font(n):return ImageFont.truetype(fontpath,n)
def rgb(c):return ((c[0]>>3)<<11)|((c[1]>>2)<<5)|(c[2]>>3)
def array(name,im):
 vals=[rgb(c) for c in im.get_flattened_data()]
 return f'inline const uint16_t {name}[]={{\n'+'\n'.join(','.join(map(str,vals[i:i+24]))+',' for i in range(0,len(vals),24))+'\n};\n'
im=Image.new('RGB',(128,48),(22,28,47));d=ImageDraw.Draw(im)
d.rounded_rectangle((1,1,126,46),radius=6,outline=(246,124,182),width=2)
d.line((8,8,20,8),fill=(106,229,214),width=2);d.line((108,39,120,39),fill=(106,229,214),width=2)
d.text((29,-3),'LATE',font=font(29),fill=(255,179,212));d.text((28,28),'CAFE / 24H',font=font(12),fill=(136,236,227))
im.save(r/'assets/cafe-sign.png');s='#pragma once\n#include <cstdint>\nnamespace Assets {\n'+array('sign',im)
for name,tint in [('warmGlow',(255,176,94)),('coolGlow',(75,196,255))]:
 im=Image.new('RGB',(16,16));vals=[]
 for y in range(16):
  for x in range(16):
   distance=math.hypot(x-15.5,y-15.5)/15.5;v=max(0,1-distance)**2*.58
   vals.append(tuple(int(c*v) for c in tint))
 im.putdata(vals);im.save(r/f'assets/{name}-quarter.png');s+=array(name,im)
(r/'main/Assets.hpp').write_text(s+'}\n')
labels=[('title','AFTER HOURS',310,27,25,(235,235,242)),('full','A SMALL NEON COURTYARD',335,19,15,(149,226,223)),('bare','01 / THE UNLIT STAGE',335,19,15,(161,191,210)),('mirror','02 / A FLOOR THAT LIES',335,19,15,(167,218,238)),('light','03 / PAINTING WITH LIGHT',335,19,15,(255,204,147)),('halo','04 / THE FINISHING GLOW',335,19,15,(244,166,213)),('fullDesc','Simple meshes. Borrowed light. A quiet night.',450,19,14,(214,224,235)),('bareDesc','No lights, reflections or halos',450,19,14,(214,224,235)),('mirrorDesc','Mirrored meshes beneath a translucent floor',455,19,14,(214,224,235)),('lightDesc','Additive discs and cones pretend to be spotlights',455,19,13,(235,219,198)),('haloDesc','Mirrored quarter-sprites soften the lamp heads',455,19,13,(235,215,229))]
s='#pragma once\n#include "Scene.hpp"\nnamespace Labels {\n'
for name,text,w,h,size,c in labels:
 im=Image.new('RGB',(w,h));ImageDraw.Draw(im).text((0,-2),text,font=font(size),fill=c)
 s+=array(name+'Pixels',im)+f'inline Renderer::Texture {name}Texture({w},{h},const_cast<uint16_t*>({name}Pixels),true,0);\n'
s+='''inline Renderer::Material titleMat(0xffff,&titleTexture),modeMat(0xffff,&fullTexture),descMat(0xffff,&fullDescTexture);
inline Renderer::Material barMat(0x0842,nullptr,nullptr,false,110);
inline Renderer::Sprite2D title,mode,desc,top,bottom;
inline void select(int stage) {
 Renderer::Texture* names[]={&fullTexture,&bareTexture,&mirrorTexture,&lightTexture,&haloTexture};
 Renderer::Texture* descriptions[]={&fullDescTexture,&bareDescTexture,&mirrorDescTexture,&lightDescTexture,&haloDescTexture};
 modeMat.diffuseMap=names[stage];descMat.diffuseMap=descriptions[stage];
}
inline void add(Renderer::Scene& scene) {
 top=Renderer::makeSolidRect(0,0,480,65,&barMat);top.zOrder=50;scene.addSprite(&top);
 bottom=Renderer::makeSolidRect(0,289,480,31,&barMat);bottom.zOrder=50;scene.addSprite(&bottom);
 title.x=18;title.y=13;title.material=&titleMat;title.zOrder=100;scene.addSprite(&title);
 mode.x=18;mode.y=44;mode.material=&modeMat;mode.zOrder=100;scene.addSprite(&mode);
 desc.x=18;desc.y=299;desc.material=&descMat;desc.zOrder=100;scene.addSprite(&desc);
}
}
'''
(r/'main/Labels.hpp').write_text(s)

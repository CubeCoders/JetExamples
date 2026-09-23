"""Generate immutable display labels for the cel-shading comparison."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import math,argparse
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--font');a=p.parse_args()
r=Path(__file__).resolve().parents[1]
fontpath=a.font or ('C:/Windows/Fonts/consola.ttf' if Path('C:/Windows/Fonts/consola.ttf').exists() else 'DejaVuSansMono.ttf')
def font(n):return ImageFont.truetype(fontpath,n)
def rgb(c):return ((c[0]>>3)<<11)|((c[1]>>2)<<5)|(c[2]>>3)
def array(name,im):
 vals=[rgb(v) for v in im.get_flattened_data()]
 return f'inline const uint16_t {name}[]={{\n'+'\n'.join(','.join(map(str,vals[i:i+24]))+',' for i in range(0,len(vals),24))+'\n};\n'
labels=[('title','CEL / TEAPOT',310,25,24,(226,249,255)),('off','SMOOTH / PHONG',330,22,19,(119,214,249)),('on','CEL / FOUR BANDS',330,22,19,(255,151,197)),('note','LIGHTING BANDS  /  NO EXTRA IMAGE BUFFER',450,19,14,(225,237,249)),('detail','SAME MOTION  |  7s PER MODE',450,18,14,(168,216,224))]
s='#pragma once\n#include "Scene.hpp"\nnamespace Labels {\n'
for name,text,w,h,size,c in labels:
 im=Image.new('RGB',(w,h));ImageDraw.Draw(im).text((0,-2),text,font=font(size),fill=c)
 s+=array(name+'Pixels',im)+f'inline Renderer::Texture {name}Texture({w},{h},const_cast<uint16_t*>({name}Pixels),true,0);\n'
s+='''inline Renderer::Material titleMat(0xffff,&titleTexture),modeMat(0xffff,&offTexture),noteMat(0xffff,&noteTexture),detailMat(0xffff,&detailTexture);
inline Renderer::Sprite2D title,mode,note,detail;
inline void select(int on) {modeMat.diffuseMap=on?&onTexture:&offTexture;}
inline void add(Renderer::Scene& scene) {
 title.x=18;title.y=14;title.material=&titleMat;scene.addSprite(&title);
 mode.x=18;mode.y=45;mode.material=&modeMat;scene.addSprite(&mode);
 detail.x=18;detail.y=271;detail.material=&detailMat;scene.addSprite(&detail);
 note.x=18;note.y=295;note.material=&noteMat;scene.addSprite(&note);
}
}
'''
(r/'main/Labels.hpp').write_text(s)

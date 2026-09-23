"""Generate the CRT showcase's original arcade test card and display labels."""
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
im=Image.new('RGB',(256,128));d=ImageDraw.Draw(im)
for y in range(128):d.line((0,y,255,y),fill=(23+int(y*.3),22+int(y*.14),61+int(y*.4)))
d.ellipse((91,10,165,84),fill=(255,151,98))
for y in range(50,88,7):d.rectangle((90,y,166,y+2),fill=(82,42,92))
d.polygon([(0,84),(24,62),(49,80),(73,55),(111,89),(142,69),(169,89),(198,56),(231,84),(255,64),(255,127),(0,127)],fill=(20,32,69))
d.line([(0,84),(24,62),(49,80),(73,55),(111,89),(142,69),(169,89),(198,56),(231,84),(255,64)],fill=(112,123,215),width=1)
d.rectangle((0,92,255,127),fill=(21,38,64))
for y in [93,97,104,115,127]:d.line((0,y,255,y),fill=(37,170,184))
for x in range(-256,513,48):d.line((128+(x-128)*.1,92,x,127),fill=(37,170,184))
d.text((13,11),'JET',font=font(27),fill=(240,252,255));d.text((14,40),'ARCADE',font=font(14),fill=(60,244,227))
d.text((14,62),'RGB / 565',font=font(9),fill=(225,230,251))
cols=[(247,250,255),(248,217,54),(46,231,223),(55,231,123),(247,83,164),(242,79,61),(90,125,247)]
for i,c in enumerate(cols):d.rectangle((13+i*10,79,22+i*10,90),fill=c)
for x in range(14,87):d.line((x,101,x,112),fill=(235,246,252) if (x//2)%2 else (14,22,34))
(r/'assets/arcade-card.png').parent.mkdir(exist_ok=True);im.save(r/'assets/arcade-card.png')
s='#pragma once\n#include <cstdint>\nnamespace Assets {\n'+array('backdrop',im)
for name,c in [('cyan',(47,230,217)),('pink',(250,95,158)),('gold',(255,199,76))]:
 tile=Image.new('RGB',(32,32),tuple(int(v*.4) for v in c));d=ImageDraw.Draw(tile)
 d.rectangle((1,1,30,30),outline=c,width=2);d.rectangle((6,6,25,25),outline=c,width=1)
 d.polygon([(16,9),(23,16),(16,23),(9,16)],fill=c);d.rectangle((14,14,17,17),fill=(240,249,255))
 s+=array(name,tile);tile.save(r/f'assets/{name}-tile.png')
(r/'main/Assets.hpp').write_text(s+'}\n')
labels=[('title','CRT / ARCADE',310,25,24,(226,249,255)),('off','SCANLINES OFF',330,22,19,(119,214,249)),('on','SCANLINES ON',330,22,19,(255,151,197)),('note','IN-PLACE   /   NO EXTRA IMAGE BUFFER',450,19,15,(225,237,249)),('detail','SAME MOTION  |  7s PER MODE',450,18,14,(168,216,224))]
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

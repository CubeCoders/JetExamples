"""Generate original mail-plane pixel artwork and immutable captions."""
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
im=Image.new('RGB',(64,32));d=ImageDraw.Draw(im)
# Original side-view mail plane, facing right. Asymmetry makes both flips obvious.
d.polygon([(10,18),(12,5),(18,5),(23,16),(48,13),(57,17),(57,22),(24,25),(11,23)],fill=(91,43,42))
d.polygon([(12,17),(14,6),(17,6),(23,18),(48,15),(55,18),(55,21),(23,23),(12,21)],fill=(207,59,49))
d.polygon([(19,19),(49,17),(54,19),(48,21),(19,22)],fill=(248,220,165))
d.polygon([(29,14),(32,10),(40,10),(45,15)],fill=(44,66,70))
d.polygon([(32,13),(34,11),(39,11),(42,14)],fill=(164,215,226))
d.polygon([(26,18),(33,17),(42,28),(33,28)],fill=(133,41,37))
d.polygon([(25,17),(33,16),(40,26),(33,26)],fill=(225,82,59))
d.rectangle((55,17,59,20),fill=(61,67,65));d.line((59,9,59,28),fill=(248,245,210),width=2)
d.line((23,24,23,27),fill=(60,63,60));d.ellipse((20,25,25,30),fill=(39,44,43))
d.rectangle((15,8,17,10),fill=(249,225,178))
im.save(r/'assets/courier.png')
(r/'main/Assets.hpp').write_text('#pragma once\n#include <cstdint>\nnamespace Assets {\n'+array('ship',im)+'}\n')
labels=[('title', 'AIR MAIL', 310, 27, 25, (255, 242, 207)), ('direction', '01 / CHANGE DIRECTION', 335, 19, 15, (186, 225, 231)), ('invert', '02 / FLY INVERTED', 335, 19, 15, (255, 209, 146)), ('echo', '03 / LEAVE AN ECHO', 335, 19, 15, (247, 212, 163)), ('cinema', '04 / CINEMATIC EXIT', 335, 19, 15, (181, 214, 245)), ('directionDesc', 'One plane texture. Flip X to fly back.', 450, 19, 14, (216, 226, 245)), ('invertDesc', 'Flip Y turns the same artwork upside down.', 450, 19, 14, (216, 226, 245)), ('echoDesc', 'Shared material alpha x individual sprite alpha', 455, 19, 13, (216, 226, 245)), ('cinemaDesc', 'Animated letterbox bars and a fade to black', 455, 19, 14, (216, 226, 245))]
s='#pragma once\n#include "Scene.hpp"\nnamespace Labels {\n'
for name,text,w,h,size,c in labels:
 im=Image.new('RGB',(w,h));ImageDraw.Draw(im).text((0,-2),text,font=font(size),fill=c)
 s+=array(name+'Pixels',im)+f'inline Renderer::Texture {name}Texture({w},{h},const_cast<uint16_t*>({name}Pixels),true,0);\n'
s+='''inline Renderer::Material titleMat(0xffff,&titleTexture),modeMat(0xffff,&directionTexture),descMat(0xffff,&directionDescTexture);
inline Renderer::Material barMat(0x0842,nullptr,nullptr,false,110);
inline Renderer::Sprite2D title,mode,desc,top,bottom;
inline void select(int stage) {
 Renderer::Texture* names[]={&directionTexture,&invertTexture,&echoTexture,&cinemaTexture};
 Renderer::Texture* descriptions[]={&directionDescTexture,&invertDescTexture,&echoDescTexture,&cinemaDescTexture};
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

"""Generate small diagnostic textures, indexed lava and immutable display labels."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import math,argparse
parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--font');args=parser.parse_args()
r=Path(__file__).resolve().parents[1]
font_path=args.font or ('C:/Windows/Fonts/consola.ttf' if Path('C:/Windows/Fonts/consola.ttf').exists() else 'DejaVuSansMono.ttf')
def font(size):
 try:return ImageFont.truetype(font_path,size)
 except OSError:
  if args.font:raise
  return ImageFont.load_default(size=size)
def rgb(c):return ((c[0]>>3)<<11)|((c[1]>>2)<<5)|(c[2]>>3)
def array(name,values,ctype='uint16_t'):
 return f'inline const {ctype} {name}[]={{\n'+'\n'.join(','.join(map(str,values[i:i+24]))+',' for i in range(0,len(values),24))+'\n};\n'
s='#pragma once\n#include <cstdint>\nnamespace Assets {\n'
tile=Image.new('RGB',(64,64),(15,43,64));d=ImageDraw.Draw(tile)
d.rectangle((0,0,63,5),fill=(250,90,150));d.rectangle((0,58,63,63),fill=(255,192,50))
d.rectangle((0,6,5,57),fill=(35,230,205));d.rectangle((58,6,63,57),fill=(115,95,240))
d.rectangle((9,9,54,54),outline=(73,129,155),width=1)
d.text((13,15),'JET',font=font(20),fill=(238,250,255))
d.polygon([(17,42),(38,42),(38,37),(49,46),(38,54),(38,49),(17,49)],fill=(35,230,205))
s+=array('tile',[rgb(p) for p in tile.get_flattened_data()]);tile.save(r/'assets/address-tile.png')
checker=Image.new('RGB',(32,32));checker.putdata([(28,56,76) if (x//8+y//8)%2 else (49,85,103) for y in range(32) for x in range(32)])
s+=array('checker',[rgb(p) for p in checker.get_flattened_data()])
leaf=Image.new('RGB',(96,128),(255,0,255));d=ImageDraw.Draw(leaf)
d.polygon([(39,123),(48,123),(58,48),(51,48)],fill=(174,110,54))
d.line([(44,123),(49,91),(55,51)],fill=(238,176,78),width=4)
for dx,dy in [(-45,-17),(-47,6),(-38,28),(-17,-32),(13,-35),(38,-26),(40,2),(32,25)]:
 x,y=54+dx,48+dy
 d.polygon([(54,48),(x+5,y-6),(x,y),(x-7,y+17),(x+12,y+8)],fill=(45,182,109))
 d.line([(54,48),(x,y)],fill=(134,231,113),width=2)
d.polygon([(30,112),(62,112),(57,127),(35,127)],fill=(232,121,66))
d.rectangle((29,110,63,115),fill=(255,177,100))
s+=array('foliage',[rgb(p) for p in leaf.get_flattened_data()]);leaf.save(r/'assets/keyed-foliage.png')
indices=[]
for y in range(64):
 for x in range(64):
  v=12*math.sin(x*.18)+9*math.cos(y*.22)+7*math.sin((x+y)*.14)+math.hypot(x-32,y-32)*1.1
  indices.append(int(v)%64)
palette=[]
for i in range(64):
 glow=(1+math.sin(i*2*math.pi/64))*.5
 palette.append((int(65+190*glow),int(5+225*glow**2),int(12+138*glow**5)))
s+=array('lavaIndices',indices,'uint8_t')+array('lavaPalette',[rgb(c) for c in palette])
lava=Image.new('RGB',(64,64));lava.putdata([palette[i] for i in indices]);lava.save(r/'assets/indexed-lava.png')
(r/'main/Assets.hpp').write_text(s+'}\n')
labels=[('title','TEXTURE LAB',310,25,24,(226,249,255)),
 ('wrap','01  WRAP / REPEAT',330,22,19,(73,233,209)),
 ('clamp','02  CLAMP / STRETCH',330,22,19,(255,199,86)),
 ('zero','03  ZERO / TRANSPARENT EDGES',334,22,17,(246,133,194)),
 ('key','04  COLOUR-KEY TRANSPARENCY',334,22,17,(134,237,167)),
 ('palette','05  PALETTE CYCLING',334,22,19,(255,179,93)),
 ('lodoff','06  TEXTURE LOD / OFF',334,22,19,(128,207,250)),
 ('lodon','07  TEXTURE LOD / ON',334,22,19,(128,207,250)),
 ('wrapDesc','UVs outside 0..1 repeat the same image',450,19,15,(204,228,239)),
 ('clampDesc','Outside UVs stretch the edge texels',450,19,15,(204,228,239)),
 ('zeroDesc','Outside UVs return black; black is keyed out',450,19,14,(204,228,239)),
 ('keyDesc','One shared image: magenta is the colour key',450,19,14,(204,228,239)),
 ('paletteDesc','64 colours move; the 4 KiB index image stays fixed',450,19,13,(255,212,160)),
 ('offDesc','Same distance sweep; texture always sampled',450,19,14,(204,228,239)),
 ('onDesc','Texture fades to a flat colour with distance',450,19,14,(204,228,239)),
 ('keyOff','KEY OFF',120,19,17,(255,162,226)),('keyOn','KEY ON',120,19,17,(113,243,178)),
 ('full','FULL TEXTURE',230,18,15,(88,239,210)),('fade','FADING TO COLOUR',230,18,15,(255,197,95)),('flat','FLAT COLOUR / NO SAMPLING',260,18,14,(252,148,198))]
s='#pragma once\n#include "Scene.hpp"\nnamespace Labels {\n'
for name,text,w,h,size,col in labels:
 im=Image.new('RGB',(w,h));ImageDraw.Draw(im).text((0,-2),text,font=font(size),fill=col)
 s+=array(name+'Pixels',[rgb(p) for p in im.get_flattened_data()])
 s+=f'inline Renderer::Texture {name}Texture({w},{h},const_cast<uint16_t*>({name}Pixels),true,0);\n'
s+='''inline Renderer::Material titleMaterial(0xffff,&titleTexture),modeMaterial(0xffff,&wrapTexture),descMaterial(0xffff,&wrapDescTexture);
inline Renderer::Material offMaterial(0xffff,&keyOffTexture),onMaterial(0xffff,&keyOnTexture),statusMaterial(0xffff,&fullTexture);
inline Renderer::Sprite2D title,mode,desc,left,right,status;
inline void select(int stage,int lodState=0) {
 Renderer::Texture* modes[]={&wrapTexture,&clampTexture,&zeroTexture,&keyTexture,&paletteTexture,&lodoffTexture,&lodonTexture};
 Renderer::Texture* descriptions[]={&wrapDescTexture,&clampDescTexture,&zeroDescTexture,&keyDescTexture,&paletteDescTexture,&offDescTexture,&onDescTexture};
 Renderer::Texture* states[]={&fullTexture,&fadeTexture,&flatTexture};
 modeMaterial.diffuseMap=modes[stage];descMaterial.diffuseMap=descriptions[stage];
 left.enabled=right.enabled=stage==3;status.enabled=stage==6;
 statusMaterial.diffuseMap=states[lodState];
}
inline void add(Renderer::Scene& scene) {
 title.x=18;title.y=14;title.material=&titleMaterial;scene.addSprite(&title);
 mode.x=18;mode.y=45;mode.material=&modeMaterial;scene.addSprite(&mode);
 desc.x=18;desc.y=294;desc.material=&descMaterial;scene.addSprite(&desc);
 left.x=92;left.y=265;left.material=&offMaterial;scene.addSprite(&left);
 right.x=277;right.y=265;right.material=&onMaterial;scene.addSprite(&right);
 status.x=112;status.y=267;status.material=&statusMaterial;scene.addSprite(&status);
}
}
'''
(r/'main/Labels.hpp').write_text(s)

"""Rebuild embedded source text, indexed livery and the procedural room panorama."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import argparse
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--font', help='Optional TrueType font for the bitmap labels')
args=parser.parse_args()
r=Path(__file__).resolve().parents[1]
def array(name,values,ctype='uint16_t'):
 return f'inline const {ctype} {name}[] = {{\n'+'\n'.join(','.join(map(str,values[i:i+24]))+',' for i in range(0,len(values),24))+'\n};\n'
def rgb565(rgb):
 rr,gg,bb=rgb;return ((rr>>3)<<11)|((gg>>2)<<5)|(bb>>3)
s='#pragma once\n#include <cstdint>\nnamespace Assets {\n'
for name,ext in [('obj','obj'),('mtl','mtl')]:
 data=(r/'assets'/f'Nascar Intel - Separate Windows.{ext}').read_text()
 s+=f'inline const char {name}[] = R"JETASSET({data})JETASSET";\n'
# Palette indices preserve a 256x256 livery in 64 KiB; all materials share it.
im=Image.open(r/'assets/outrun.png').convert('RGB').transpose(Image.Transpose.FLIP_TOP_BOTTOM).resize((256,256),Image.Resampling.BOX)
im=im.quantize(colors=256,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
s+=array('liveryIndices',list(im.get_flattened_data()),'uint8_t')
pal=im.getpalette();s+=array('liveryPalette',[rgb565(pal[i:i+3]) for i in range(0,768,3)])
im.convert('RGB').save(r/'assets/livery-256.png')
# Distant-room panorama, not a captured framebuffer. Light positions match the
# eight workshop ribs; latitude places ceiling bars above the horizon.
env=Image.new('RGB',(128,64));pixels=[]
for y in range(64):
 v=(y+.5)/64
 for x in range(128):
  u=(x+.5)/128;col=[10,18,36] if v<.5 else [5,10,20]
  for i in range(8):
   center=(.5+i/8)%1;du=abs((u-center+.5)%1-.5)
   strength=max(0,1-du/.016)*max(0,1-abs(v-.34)/.24)
   broad=max(0,1-du/.055)*.28*max(0,1-abs(v-.35)/.32)
   tint=(55,225,255) if i%2==0 else (240,45,195)
   for c in range(3):col[c]+=tint[c]*(strength+broad)
  # Broad white ceiling panels yield recognisable glossy stripes in the glass.
  panel=max(0,1-abs(v-.22)/.024)
  if int(u*16)%2==0:
   for c in range(3):col[c]+=panel*205
  floor=max(0,1-abs(v-.62)/.028)
  for c in range(3):col[c]+=floor*(30,50,85)[c]
  pixels.append(tuple(min(255,int(c)) for c in col))
env.putdata(pixels);env.save(r/'assets/workshop-environment.png')
s+=array('environment',[rgb565(p) for p in pixels]);s+='}\n'
(r/'main/Assets.hpp').write_text(s)
font_path=args.font or ('C:/Windows/Fonts/consola.ttf' if Path('C:/Windows/Fonts/consola.ttf').exists() else 'DejaVuSansMono.ttf')
def label_font(size):
 try:
  return ImageFont.truetype(font_path,size)
 except OSError:
  if args.font:
   raise
  return ImageFont.load_default(size=size)
s='#pragma once\n#include "Scene.hpp"\nnamespace Labels {\n'
for name,text,w,h,size,col in [('title','NEON MOTORWORKS',320,26,23,(222,245,255)),('caption','OBJ / PHONG PAINT / CHROME GLASS',420,17,14,(142,213,237))]:
 im=Image.new('RGB',(w,h));ImageDraw.Draw(im).text((0,-2),text,font=label_font(size),fill=col)
 s+=array(name+'Pixels',[rgb565(p) for p in im.get_flattened_data()])
 s+=f'inline Renderer::Texture {name}Texture({w},{h},const_cast<uint16_t*>({name}Pixels),true,0);\ninline Renderer::Material {name}Material(0xffff,&{name}Texture);\ninline Renderer::Sprite2D {name};\n'
s+='''inline void add(Renderer::Scene& scene) {
 title.x=20;title.y=17;title.material=&titleMaterial;scene.addSprite(&title);
 caption.x=20;caption.y=292;caption.material=&captionMaterial;scene.addSprite(&caption);
}
}
''';(r/'main/Labels.hpp').write_text(s)

"""Generate original neon-film facades, signs, dashboard and reflection artwork."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import math,random
r=Path(__file__).resolve().parents[1]
fontpath='C:/Windows/Fonts/consola.ttf' if Path('C:/Windows/Fonts/consola.ttf').exists() else 'DejaVuSansMono.ttf'
def font(n):return ImageFont.truetype(fontpath,n)
def rgb(c):return ((c[0]>>3)<<11)|((c[1]>>2)<<5)|(c[2]>>3)
def array(name,im):
 vals=[rgb(c) for c in im.get_flattened_data()]
 return f'inline const uint16_t {name}[]={{\n'+'\n'.join(','.join(map(str,vals[i:i+24]))+',' for i in range(0,len(vals),24))+'\n};\n'
s='#pragma once\n#include <cstdint>\nnamespace Assets {\n';rng=random.Random(2387)
for k in range(4):
 im=Image.new('RGB',(64,128),(13+k*3,19,35+k*4));d=ImageDraw.Draw(im)
 for y in range(5,125,9):
  for x in range(4,62,8):
   c=[(60,139,176),(220,165,103),(148,92,172),(45,70,97)][k]
   if rng.random()<.62:d.rectangle((x,y,x+3,y+4),fill=tuple(int(v*(.55+rng.random()*.45)) for v in c))
 d.line((0,0,0,127),fill=(54,97,127),width=2)
 if k%2:d.line((60,0,60,127),fill=(214,47,146),width=2)
 s+=array('facade'+str(k),im.transpose(Image.Transpose.FLIP_TOP_BOTTOM));im.save(r/f'assets/facade-{k}.png')
# Four graphic shopfronts: glass, a recessed door, shutters and lit interiors.
for k in range(4):
 im=Image.new('RGB',(128,64),(17,24,35));d=ImageDraw.Draw(im)
 glow=[(194,106,49),(67,140,174),(154,65,128),(59,127,106)][k]
 d.rectangle((3,6,78,54),fill=glow);d.rectangle((6,9,75,48),fill=tuple(v//2 for v in glow))
 for x in range(8,77,9):
  d.rectangle((x,24+(x%3)*4,x+5,44),fill=glow)
 d.rectangle((3,48,79,54),fill=(53,58,65));d.line((40,7,40,48),fill=(16,25,34),width=3)
 d.rectangle((87,4,120,63),fill=(7,15,24));d.rectangle((92,9,115,47),fill=tuple(v//3 for v in glow));d.line((94,32,94,38),fill=(164,184,184),width=2)
 d.rectangle((0,0,127,4),fill=glow)
 if k==2:
  d.rectangle((3,7,80,52),fill=(43,46,63))
  for y in range(10,53,4):d.line((3,y,80,y),fill=(79,66,88))
 for x in (0,81,123):d.line((x,4,x,63),fill=(73,69,77),width=3)
 s+=array('shop'+str(k),im.transpose(Image.Transpose.FLIP_TOP_BOTTOM));im.save(r/f'assets/shop-{k}.png')
signs=[('NOVA','RAMEN',(255,86,166)),('ORBIT','MOTEL',(67,224,244)),('KAIRO','ARCADE',(250,170,59)),('SYNTH','CLINIC',(122,241,164)),('AFTER','HOURS',(196,104,255)),('ION 88','AUTONOMOUS',(78,229,235))]
for i,(a,b,c) in enumerate(signs):
 im=Image.new('RGB',(128,48),(8,14,28));d=ImageDraw.Draw(im);d.rounded_rectangle((1,1,126,46),4,outline=c,width=2)
 d.text((7,0),a,font=font(28),fill=c);d.text((8,30),b,font=font(12),fill=(215,231,240));im=im.resize((128,64),Image.Resampling.NEAREST);s+=array('sign'+str(i),im.transpose(Image.Transpose.FLIP_TOP_BOTTOM));im.save(r/f'assets/sign-{i}.png')
im=Image.new('RGB',(128,64),(11,18,37));d=ImageDraw.Draw(im)
for x in range(0,128,8):
 c=(42,227,236) if x%24==0 else (226,48,173) if x%16==0 else (54,82,132)
 d.rectangle((x,8+(x*7)%24,x+3,58),fill=c)
d.line((0,40,127,40),fill=(165,91,198),width=4);s+=array('environment',im);im.save(r/'assets/environment.png')
im=Image.new('RGB',(64,96));d=ImageDraw.Draw(im)
for y in range(8,90,3):
 width=int(24*(.5+.5*math.sin((y-7)*math.pi/82)))+4
 d.line((32-width,y,32+width,y),fill=(26,132+(y%7)*12,173))
d.ellipse((22,12,42,32),outline=(103,250,255),width=2);d.polygon([(21,38),(43,38),(49,75),(15,75)],outline=(92,230,238));s+=array('hologram',im.transpose(Image.Transpose.FLIP_TOP_BOTTOM));im.save(r/'assets/hologram.png')
im=Image.new('RGB',(16,16));im.putdata([tuple(int(c*max(0,1-math.hypot(x-15.5,y-15.5)/15.5)**2) for c in (37,123,155)) for y in range(16) for x in range(16)]);s+=array('glow',im)
im=Image.new('RGB',(256,96),(6,11,20));d=ImageDraw.Draw(im)
d.rounded_rectangle((1,1,254,94),8,outline=(64,101,127),width=2)
d.text((10,6),'ION // AUTONOMOUS',font=font(20),fill=(92,245,215));d.text((12,39),'NO DRIVER REQUIRED',font=font(15),fill=(252,177,65));d.text((12,65),'NIGHT FLIGHT / 2387',font=font(14),fill=(231,79,124))
im=im.resize((256,128),Image.Resampling.NEAREST);s+=array('dashboard',im.transpose(Image.Transpose.FLIP_TOP_BOTTOM));im.save(r/'assets/dashboard.png')
(r/'main/Assets.hpp').write_text(s+'}\n')

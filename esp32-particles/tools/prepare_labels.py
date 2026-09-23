"""Generate immutable labels and digit textures for the particle showcase."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import argparse
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--font');a=p.parse_args();r=Path(__file__).resolve().parents[1]
fontpath=a.font or ('C:/Windows/Fonts/consola.ttf' if Path('C:/Windows/Fonts/consola.ttf').exists() else 'DejaVuSansMono.ttf')
def array(name,im):
 vals=[((c[0]>>3)<<11)|((c[1]>>2)<<5)|(c[2]>>3) for c in im.get_flattened_data()]
 return f'inline const uint16_t {name}Pixels[]={{\n'+'\n'.join(','.join(map(str,vals[i:i+24]))+',' for i in range(0,len(vals),24))+'\n};\n'
labels=[('title','PARTICLE LAB',310,25,24,(224,246,255)),('spark','01 / ADDITIVE SPARKS',330,22,18,(91,225,255)),('water','02 / WATER SPRAY',330,22,18,(218,170,255)),('pool','03 / POOL LIMIT: 200',330,22,18,(255,202,100)),('distance','04 / DISTANCE CULL',330,22,18,(128,220,233)),('sparkDesc','White-hot bursts cool to blue and fade',450,19,14,(216,236,246)),('waterDesc','Gravity + velocity / ordinary alpha fades',450,19,14,(216,236,246)),('poolDesc','260 requested per burst / 200 shared slots',450,19,14,(255,214,148)),('distanceDesc','Beyond 1500 units: alive, but not drawn',450,19,14,(216,236,246)),('live','LIVE',40,18,15,(163,218,232)),('cap','/ 200',65,18,15,(163,218,232)),('drawn','DRAWN',60,18,15,(163,218,232))]
labels += [('digit'+str(i),str(i),12,18,16,(245,250,255)) for i in range(10)]
s='#pragma once\n#include "Scene.hpp"\nnamespace Labels {\n'
for name,text,w,h,size,c in labels:
 im=Image.new('RGB',(w,h));ImageDraw.Draw(im).text((0,-2),text,font=ImageFont.truetype(fontpath,size),fill=c)
 s+=array(name,im)+f'inline Renderer::Texture {name}Texture({w},{h},const_cast<uint16_t*>({name}Pixels),true,0);\n'
s+='''inline Renderer::Material titleMat(0xffff,&titleTexture),modeMat(0xffff,&sparkTexture),descMat(0xffff,&sparkDescTexture);
inline Renderer::Material liveMat(0xffff,&liveTexture),capMat(0xffff,&capTexture),drawnMat(0xffff,&drawnTexture),digits[6];
inline Renderer::Sprite2D title,mode,desc,live,cap,drawn,number[6];
inline Renderer::Texture* numeral[]={&digit0Texture,&digit1Texture,&digit2Texture,&digit3Texture,&digit4Texture,&digit5Texture,&digit6Texture,&digit7Texture,&digit8Texture,&digit9Texture};
inline void counts(unsigned active,unsigned rendered) {
 unsigned values[]={active,rendered};
 for(int group=0;group<2;++group)for(int i=0;i<3;++i){unsigned div=i==0?100:i==1?10:1;digits[group*3+i].diffuseMap=numeral[(values[group]/div)%10];}
}
inline void select(int n) {
 Renderer::Texture* modes[]={&sparkTexture,&waterTexture,&poolTexture,&distanceTexture};
 Renderer::Texture* descriptions[]={&sparkDescTexture,&waterDescTexture,&poolDescTexture,&distanceDescTexture};
 modeMat.diffuseMap=modes[n];descMat.diffuseMap=descriptions[n];
}
inline void add(Renderer::Scene& s) {
 title.x=18;title.y=14;title.material=&titleMat;s.addSprite(&title);
 mode.x=18;mode.y=45;mode.material=&modeMat;s.addSprite(&mode);
 desc.x=18;desc.y=295;desc.material=&descMat;s.addSprite(&desc);
 live.x=18;live.y=271;live.material=&liveMat;s.addSprite(&live);
 cap.x=103;cap.y=271;cap.material=&capMat;s.addSprite(&cap);
 drawn.x=220;drawn.y=271;drawn.material=&drawnMat;s.addSprite(&drawn);
 for(int i=0;i<6;++i){number[i].x=(i<3?62:280)+(i%3)*12;number[i].y=271;number[i].material=&digits[i];s.addSprite(&number[i]);}
 counts(0,0);
}
}
'''
(r/'main/Labels.hpp').write_text(s)

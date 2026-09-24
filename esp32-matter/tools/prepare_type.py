"""Bake original exhibition typography; only one-bit masks are embedded."""
from PIL import Image,ImageDraw,ImageFont
from pathlib import Path
import argparse
p=argparse.ArgumentParser();p.add_argument('--font',default='C:/Windows/Fonts/arial.ttf');p.add_argument('--bold',default='C:/Windows/Fonts/arialbd.ttf');p.add_argument('--output',type=Path,default=Path(__file__).resolve().parents[1]/'main/Typography.hpp');a=p.parse_args()
labels=['MATTER','01 / PRESSURE','02 / COUNTERWEIGHT','03 / PAPER WEATHER','04 / QUICKSILVER','05 / COLOUR IN SPACE','06 / BOTANICA','07 / INTERFERENCE','08 / RELIQUARY','09 / THE GYRE','10 / MATTER','MATTER']
out=['#pragma once','#include <cstdint>','namespace Matter::Type {']
sizes=[]
for i,s in enumerate(labels):
    w,h=(360,79) if i==0 else (410,84) if i==11 else (360,25)
    im=Image.new('1',(w,h));d=ImageDraw.Draw(im)
    d.text((0,-7 if i==0 else -4),s,font=ImageFont.truetype(a.bold,54 if i==0 else 30 if i==11 else 18),fill=1)
    if i==0:d.text((3,57),'AN EXHIBITION IN MOTION',font=ImageFont.truetype(a.font,13),fill=1)
    if i==11:
        d.text((1,34),'A demo by CubeCoders  /  Powered by Jet',font=ImageFont.truetype(a.font,14),fill=1)
        d.text((1,56),'Software 3D on ESP32-S3   ·   github.com/cubecoders/jet',font=ImageFont.truetype(a.font,12),fill=1)
    pixels=list(im.getdata());data=bytearray((w*h+7)//8)
    for j,v in enumerate(pixels):
        if v:data[j//8]|=0x80>>(j%8)
    out.append('inline constexpr uint8_t mask%d[]={%s};'%(i,','.join(str(v) for v in data)));sizes.append((w,h))
out+=['inline constexpr int count=12;','inline constexpr int widths[]={'+','.join(str(s[0]) for s in sizes)+'};','inline constexpr int heights[]={'+','.join(str(s[1]) for s in sizes)+'};','inline constexpr const uint8_t* data[]={'+','.join('mask%d'%i for i in range(12))+'};','}']
a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text('\n'.join(out)+'\n');print(a.output)

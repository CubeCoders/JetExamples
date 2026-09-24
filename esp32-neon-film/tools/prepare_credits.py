from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import argparse
p=argparse.ArgumentParser(description='Bake closing-credit text into one-bit masks with Pillow')
p.add_argument('--font',required=True,type=Path)
p.add_argument('--bold-font',required=True,type=Path)
a=p.parse_args()
root=Path(__file__).resolve().parents[1]
# Baked coverage masks retain smooth desktop typography without a runtime font
# library. The embedded build includes only its 7.5 KiB, one-bit mask.
font=a.font
bold=a.bold_font
def mask(scale):
    im=Image.new('1',(360*scale,168*scale));d=ImageDraw.Draw(im)
    for text,y,size,f in [('ESP 88',0,40,bold),('A demo by CubeCoders',51,17,font),('Powered by Jet',96,19,bold),('Software 3D Renderer',124,16,font),('https://github.com/cubecoders/jet',151,14,font)]:
        ft=ImageFont.truetype(str(f),size*scale)
        d.text((180*scale,y*scale),text,font=ft,fill=1,anchor='mt',stroke_width=0)
    return im.tobytes()
header='#pragma once\n#include <cstdint>\nnamespace Film {\n'
for scale in (6,1):
    header+=('#if FILM_RENDER_SCALE == 6\n' if scale==6 else '#else\n')
    data=mask(scale)
    header+='inline constexpr uint8_t creditMask[]={\n'+''.join(','.join(str(x) for x in data[i:i+32])+',\n' for i in range(0,len(data),32))+'};\n'
header+='#endif\n}\n'
(root/'main/CreditMask.hpp').write_text(header,newline='\n')

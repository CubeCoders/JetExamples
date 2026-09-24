"""Pipe Jet's packed-field or full-frame RGB24 render into ffmpeg without frame files."""
import argparse,subprocess,os
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('output',type=Path)
p.add_argument('--ffmpeg',default='ffmpeg')
p.add_argument('--renderer',type=Path,default=None)
p.add_argument('--quality',action='store_true',help='Use the full-frame 2880x1920 renderer and downsample to 1920x1280')
p.add_argument('--start',type=float,default=0)
p.add_argument('--seconds',type=float,default=102)
a=p.parse_args();a.output.parent.mkdir(parents=True,exist_ok=True)
if a.renderer is None:
 a.renderer=Path(__file__).resolve().parents[1]/(('build-quality/film_quality' if a.quality else 'build-preview/film_video')+('.exe' if os.name=='nt' else ''))
with a.output.with_suffix('.render.log').open('w') as log:
 renderer=subprocess.Popen([str(a.renderer),str(a.start),str(a.seconds)],stdout=subprocess.PIPE,stderr=log)
 encoder=subprocess.run([a.ffmpeg,'-progress',str(a.output.with_suffix('.encode.log')),'-hide_banner','-loglevel','warning','-y','-f','rawvideo','-pixel_format','rgb24','-video_size','2880x1920' if a.quality else '480x320','-framerate','60','-i','pipe:0','-vf','scale=1920:1280:flags=lanczos' if a.quality else 'scale=960:640:flags=neighbor','-c:v','libx264','-preset','medium' if a.quality else 'fast','-threads','2','-crf','15' if a.quality else '18','-pix_fmt','yuv420p','-movflags','+faststart',str(a.output)],stdin=renderer.stdout)
 renderer.stdout.close();rc=renderer.wait()
 if rc or encoder.returncode:raise SystemExit(f'render={rc}, encode={encoder.returncode}')
print(a.output)

#!/usr/bin/env python3
"""Explicit Hyprland desktop check using an owned, disposable headless output."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import signal
import subprocess
import time

p=argparse.ArgumentParser()
p.add_argument('--desktop',action='store_true')
p.add_argument('--app',required=True,type=Path)
p.add_argument('--output',required=True,type=Path)
p.add_argument('--gpu',default='')
a=p.parse_args()
if not a.desktop:p.error('--desktop required: creates and removes a headless output')
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
name=f'WALLPAPER-LAB-TEST-{os.getpid()}'
env=os.environ|{f'XDG_{k}_HOME':str(out/k.lower()) for k in ('CONFIG','STATE','DATA','CACHE')}
renderer=None;started=None;created=False;log=(out/'renderer.log').open('w')
def run(*args):return subprocess.check_output(list(map(str,args)),env=env,text=True,timeout=30)
def wait(fn):
 end=time.monotonic()+20
 while time.monotonic()<end:
  value=fn()
  if value:return value
  time.sleep(.1)
 raise AssertionError('Headless output condition timed out')
def listed():return any(m['name']==name for m in json.loads(run('hyprctl','-j','monitors')))
def layer():return 'wallpaper-lab-background' in json.dumps(json.loads(run('hyprctl','-j','layers')).get(name,{}))
def frame(label):
 path=out/(label+'.png');run('grim','-o',name,path)
 return hashlib.sha256(path.read_bytes()).hexdigest()
def moving(label):
 time.sleep(1);first=frame(label+'-a');time.sleep(1.2);return first!=frame(label+'-b')
try:
 project=out/'project';project.mkdir(exist_ok=True);(out/'assets').mkdir(exist_ok=True)
 subprocess.run(['ffmpeg','-v','error','-f','lavfi','-i','testsrc2=size=320x180:rate=15','-t','5','-c:v','libx264','-pix_fmt','yuv420p','-y',str(project/'media.mp4')],check=True)
 (project/'project.json').write_text(json.dumps({'type':'video','title':'Output reconnect fixture','file':'media.mp4'}))
 run('hyprctl','output','create','headless',name);created=True;wait(listed)
 ready=out/'ready'
 renderer=subprocess.Popen([str(app),'--render',*(['--lab-gpu',a.gpu] if a.gpu else []),'--ready-file',str(ready),'--assets-dir',str(out/'assets'),'--fps','15','--volume','0','--screen-root',name,'--bg',str(project)],env=env,stdout=log,stderr=log,start_new_session=True)
 started=Path(f'/proc/{renderer.pid}/stat').read_text().rsplit(')',1)[1].split()[19]
 wait(lambda:ready.exists() or renderer.poll() is not None);assert renderer.poll() is None,'Renderer exited before readiness'
 wait(layer);assert moving('before'),'Initial video is frozen'
 run('hyprctl','output','remove',name);created=False;wait(lambda:not listed());time.sleep(.4)
 assert renderer.poll() is None,'Renderer exited on disconnect'
 run('hyprctl','output','create','headless',name);created=True;wait(listed);wait(layer)
 assert renderer.poll() is None and moving('after'),'Video did not resume after reconnect'
 result={'output_disconnect_survived':True,'layer_restored':True,'video_frames_resumed':True,'same_renderer_pid':True}
 if a.gpu:
  result['gpu_request']=a.gpu
  result['drm_devices']=sorted({line for fd in Path(f'/proc/{renderer.pid}/fdinfo').iterdir() for line in fd.read_text().splitlines() if line.startswith(('drm-driver:', 'drm-pdev:'))})
 (out/'results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
finally:
 if renderer and renderer.poll() is None:
  proc=Path(f'/proc/{renderer.pid}')
  if proc.joinpath('exe').resolve()==app and proc.joinpath('stat').read_text().rsplit(')',1)[1].split()[19]==started:
   os.killpg(renderer.pid,signal.SIGTERM)
   try:renderer.wait(timeout=10)
   except subprocess.TimeoutExpired:os.killpg(renderer.pid,signal.SIGKILL);renderer.wait()
 if created:run('hyprctl','output','remove',name)
 log.close()

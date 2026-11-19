#!/usr/bin/env python3
"""Explicit desktop check: real keyboard input, isolated settings and fake awww."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import time
from PIL import Image

p=argparse.ArgumentParser()
p.add_argument('--desktop',action='store_true')
p.add_argument('--app',required=True,type=Path)
p.add_argument('--output',required=True,type=Path)
a=p.parse_args()
if not a.desktop:p.error('--desktop required: opens test carousel windows and injects keys')
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True)
bin=out/'bin';bin.mkdir(exist_ok=True)
(bin/'awww').write_text('''#!/usr/bin/python3
import json,os,sys
from pathlib import Path
r=Path(os.environ['PERSISTENT_TEST_ROOT']);f=r/'daemon.json';s=json.loads(f.read_text()) if f.exists() else {};a=sys.argv[1:]
if a[0]=='img':
 for o in a[a.index('--outputs')+1].split(','):s[o]=a[1]
elif a[0]=='query':
 for o,i in s.items():print(f': {o}: 1920x1080, scale: 1, currently displaying: image: {i}')
elif a[0]=='clear':
 for o in a[a.index('--outputs')+1].split(','):s.pop(o,None)
f.write_text(json.dumps(s))
''');(bin/'awww').chmod(0o755)
ydotool=shutil.which('ydotool');ydotoold=shutil.which('ydotoold');assert ydotool and ydotoold
socket=out/'input.sock';socket.unlink(missing_ok=True)
env=os.environ|{f'XDG_{kind}_HOME':str(out/kind.lower()) for kind in ('CONFIG','STATE','CACHE','DATA')}
env.update(PATH=str(bin)+':'+os.environ['PATH'],PERSISTENT_TEST_ROOT=str(out),YDOTOOL_SOCKET=str(socket),WALLPAPER_LAB_POPUP_STATE=str(out/'popup.json'))
log=(out/'test.log').open('w');hosts=[];results={};session=0
personal_launcher=Path.home()/'.local/bin/wallpaper-lab'
personal_status=subprocess.check_output([str(personal_launcher),'--status'],text=True)
personal_image=subprocess.check_output(['awww','query'],text=True)
def run(*args):return subprocess.check_output(list(map(str,args)),env=env,text=True,stderr=log)
def wait(fn,timeout=15):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  try:
   value=fn()
   if value:return value
  except (FileNotFoundError,json.JSONDecodeError):pass
  time.sleep(.06)
 raise AssertionError('desktop condition timed out')
def trace():return json.loads((out/'popup.json').read_text())
def status():return json.loads(run(app,'--status'))
def window(pid):return next((w for w in json.loads(run('hyprctl','clients','-j')) if w['pid']==pid),None)
def start(action='commit'):
 global session
 session+=1;(out/'popup.json').unlink(missing_ok=True)
 host=subprocess.Popen([str(app),'--carousel-event','persistent-test',str(session),'1',action],env=env,stdout=log,stderr=log);hosts.append(host)
 wait(lambda:window(host.pid));wait(lambda:trace().get('hits'))
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{host.pid}"}}))');time.sleep(.2)
 return host

def key(code):run(ydotool,'key',f'{code}:1',f'{code}:0');time.sleep(.15)
def event(number,steps,action):run(app,'--carousel-event','persistent-test',number,steps,action)
daemon=subprocess.Popen([ydotoold,'--socket-path='+str(socket)],stdout=log,stderr=log)
try:
 wait(socket.exists);time.sleep(1.5)
 images=[]
 for name,color in [('AAA','#3443dd'),('BBB','#df4858'),('CCC','#39b77c')]:
  image=out/(name+'.png');Image.new('RGB',(640,360),color).save(image);images.append(json.loads(run(app,'--import',image))[0])
 prefs={'wallpaper_backend':1,'awww_position':1,'mode':'single','screen':0,'caelestia_enabled':False,'carousel_persistent':True,'reduce_motion':True,'carousel_layout':0}
 (config/'settings.json').write_text(json.dumps(prefs));(config/'carousel.json').write_text(json.dumps(images))
 for layout in range(4):
  run(app,'--ui-option','carousel_layout',layout);run(app,'--apply',images[0])
  host=start();assert host.poll() is None and trace()['position']==0
  key(106);wait(lambda:trace()['position']==1)
  # A second tap/release of the global shortcut advances the same window.
  previous=session;session+=1;event(session,1,'commit');wait(lambda:trace()['position']==2)
  assert host.poll() is None
  event(previous,99,'commit');time.sleep(.15);assert trace()['position']==2
  key(15);wait(lambda:trace()['position']==0)
  key(105);wait(lambda:trace()['position']==2)
  key(103);wait(lambda:trace()['position']==(0 if layout==0 else 1))
  key(108);wait(lambda:trace()['position']==2)
  key(28);host.wait(timeout=20);assert host.returncode==0 and status()['path']==images[2]
  before=status();cancel=start();key(106);wait(lambda:trace()['position']==0);key(1);cancel.wait(timeout=10);assert status()==before
  results[str(layout)]={'release_stays_open':True,'same_window_shortcut_taps':True,'arrows_and_tab':True,'enter_applies':True,'escape_cancels':True}
 run(app,'--ui-option','carousel_persistent',0)
 host=start('step');event(session,2,'commit');host.wait(timeout=20)
 assert host.returncode==0 and status()['path']==images[0]
 results['hold_mode_release_applies']=True
finally:
 for host in hosts:
  if host.poll() is None:
   run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.close({{window="pid:{host.pid}"}}))')
   try:host.wait(timeout=10)
   except subprocess.TimeoutExpired:host.terminate();host.wait()
 run(app,'--stop');daemon.terminate();daemon.wait();socket.unlink(missing_ok=True);log.close()
assert personal_status==subprocess.check_output([str(personal_launcher),'--status'],text=True)
assert personal_image==subprocess.check_output(['awww','query'],text=True)
results['personal_wallpaper_preserved']=True
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))

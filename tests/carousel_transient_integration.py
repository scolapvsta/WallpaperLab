#!/usr/bin/env python3
"""Isolated live carousel: active anchor, saved order, no-op, non-favorite and grid bounds."""
import argparse,json,pathlib,subprocess,os,time
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);capture=out/'capture';capture.mkdir(exist_ok=True);env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'XDG_DATA_HOME':str(out/'data'),'XDG_CACHE_HOME':str(out/'cache'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps']),'WALLPAPER_LAB_POPUP_STATE':str(out/'popup.json'),'WALLPAPER_LAB_POPUP_CAPTURE':str(capture)};log=open(out/'log','w');hosts=[];results={}
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,text=True,stderr=log)
def status():return json.loads(run(app,'--status'))
def wait(fn,timeout=30):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  try:v=fn()
  except(FileNotFoundError,json.JSONDecodeError):v=None
  if v:return v
  time.sleep(.05)
 raise AssertionError('timeout')
def trace():return json.loads((out/'popup.json').read_text())
def start(session):
 (out/'popup.json').unlink(missing_ok=True);child=subprocess.Popen([str(app),'--carousel-event','anchor',str(session),'1','step'],env=env,stdout=log,stderr=log);hosts.append(child);wait(lambda:trace()['steps']==1);return child

def event(session,steps,action):run(app,'--carousel-event','anchor',session,steps,action)

personal=json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));paths=[]
try:
 for name,color in [('AAA','blue'),('BBB','red'),('CCC','green'),('DDD','purple')]:
  file=out/(name+'.png');Image.new('RGB',(1280,720),color).save(file);paths.append(json.loads(run(app,'--import',file))[0])
 (config/'settings.json').write_text(json.dumps({'mode':'single','screen':0,'fps':30,'muted':True,'workshop_path':str(steam),'carousel_layout':0,'theme_catalog':True,'ui_theme':0,'transition_type':0}));favorites=[paths[2],paths[0],paths[1]];(config/'carousel.json').write_text(json.dumps(favorites))
 for mode in range(4):
  run(app,'--ui-option','carousel_layout',mode);run(app,'--apply',paths[3]);h=start(10+mode);assert trace()['selected']==3;time.sleep(.7);frame=Image.open(capture/'step-1-2.png').convert('RGB');t=trace();hits=[x for x in t['hits']if x[0]==3];assert hits;hit=min(hits,key=lambda r:abs(r[1]+r[3]/2-frame.width/2)+abs(r[2]+r[4]/2-frame.height/2));color=frame.getpixel((int(hit[1]+hit[3]/2),int(hit[2]+hit[4]/2)));assert color[0]>90 and color[2]>90 and color[1]<20,(mode,color)
  for step in range(2,12):event(10+mode,step,'step');wait(lambda:trace()['steps']==step);assert trace()['selected']==[2,0,1][(step-2)%3],(mode,trace())
  event(10+mode,11,'commit');h.wait(35);assert h.returncode==0 and status()['path']==paths[2];assert json.loads((config/'carousel.json').read_text())==favorites;results[str(mode)]={'active_thumbnail_visible':True,'three_wraps_skip_nonfavorite':True,'favorites_unchanged':True}
finally:
 for h in hosts:
  if h.poll()is None:
   try:subprocess.run(['hyprctl','eval',f'hl.dispatch(hl.dsp.window.close({{window="pid:{h.pid}"}}))'],stdout=log,stderr=log);h.wait(40)
   except Exception:pass
 run(app,'--stop');(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));print(json.dumps(results,indent=2))

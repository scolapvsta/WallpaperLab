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
 (config/'settings.json').write_text(json.dumps({'mode':'single','screen':0,'fps':30,'muted':True,'workshop_path':str(steam),'carousel_layout':0,'theme_catalog':True,'ui_theme':0,'transition_type':1}));favorites=[paths[2],paths[0],paths[1]];(config/'carousel.json').write_text(json.dumps(favorites));run(app,'--apply',paths[0]);baseline=status()
 h=start(1);assert trace()['baseline']==1 and trace()['selected']==0 and trace()['position']==1,trace();wait(lambda:(capture/'step-1-2.png').exists());frame=Image.open(capture/'step-1-2.png');bounds=frame.getchannel('A').getbbox();t=trace();assert bounds[0]>=0 and bounds[1]>=0 and bounds[2]<=frame.width and bounds[3]<=frame.height,bounds;assert bounds[2]>t['padding']+t['width'] or bounds[0]<t['padding'],'selection zoom missing' ;event(1,1,'commit');h.wait(35);assert h.returncode==0 and status()==baseline;results['open_on_active_and_no_restart']=True;results['grid_zoom_contained_in_native_surface']=True
 h=start(2);event(2,2,'step');wait(lambda:trace()['steps']==2);assert trace()['selected']==1 and trace()['position']==2,trace();event(2,3,'step');wait(lambda:trace()['steps']==3);assert trace()['selected']==2 and trace()['position']==0,trace();event(2,3,'commit');h.wait(35);assert h.returncode==0 and status()['path']==paths[2];results['saved_order_not_catalog_sort']=True
 stable=status();run(app,'--carousel-event','anchor',3,1,'commit');assert status()==stable;results['early_release_on_active_is_noop']=True
 run(app,'--apply',paths[3]);h=start(4);assert trace()['selected']==3 and trace()['position']==0,trace();wait(lambda:(capture/'step-1-2.png').stat().st_mtime>time.time()-3);time.sleep(.55);frame=Image.open(capture/'step-1-2.png').convert('RGB');t=trace();color=frame.getpixel((int(t['padding']+t['tile']/2),int(t['padding']+t['tile']/2)));assert color[0]>90 and color[2]>90 and color[1]<20,('transient active thumbnail missing',color);
 for step in range(2,12):
  event(4,step,'step');wait(lambda:trace()['steps']==step);assert trace()['selected']==[2,0,1][(step-2)%3],('non-favorite re-entered rotation',trace())
 event(4,11,'commit');h.wait(35);assert h.returncode==0 and status()['path']==paths[2];assert json.loads((config/'carousel.json').read_text())==favorites;results['nonfavorite_current_thumbnail_loaded_and_skipped_on_three_wraps']=True
finally:
 for h in hosts:
  if h.poll()is None:
   try:subprocess.run(['hyprctl','eval',f'hl.dispatch(hl.dsp.window.close({{window="pid:{h.pid}"}}))'],stdout=log,stderr=log);h.wait(40)
   except Exception:pass
 run(app,'--stop');(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));print(json.dumps(results,indent=2))

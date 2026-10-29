#!/usr/bin/env python3
"""Native carousel: grid hover, click confirmation, selection zoom and active no-op."""
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
sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-mouse-{os.getpid()}.sock';env['YDOTOOL_SOCKET']=str(sock);daemon=subprocess.Popen(['ydotoold','--socket-path='+str(sock)],stdout=log,stderr=log)
def window(pid):return next((w for w in json.loads(run('hyprctl','clients','-j'))if w['pid']==pid),None)
def move_to(pid,point):
 w=window(pid);run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{pid}"}}))');run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={round(w["at"][0]+point[0])},y={round(w["at"][1]+point[1])}}}))');time.sleep(.2)
def target(index):
 t=trace();hits=[h for h in t['hits']if h[0]==index and 0<h[1]+h[3]/2<t['width']+2*t['padding']and 0<h[2]+h[4]/2<t['height']+2*t['padding']];assert hits,(index,t)
 h=max(hits,key=lambda h:h[3]*h[4]);return(h[1]+h[3]/2,h[2]+h[4]/2)
try:
 wait(sock.exists);time.sleep(1.2)
 for name,color in [('AAA','blue'),('BBB','red'),('CCC','green')]:
  file=out/(name+'.png');Image.new('RGB',(1280,720),color).save(file);paths.append(json.loads(run(app,'--import',file))[0])
 prefs={'mode':'single','screen':0,'fps':30,'muted':True,'workshop_path':str(steam),'carousel_layout':0,'theme_catalog':True,'ui_theme':0,'transition_type':0,'switcher_zoom':140,'switcher_size':120,'wave_speed':140,'wave_height':30}
 favorites=[paths[2],paths[0],paths[1]];(config/'carousel.json').write_text(json.dumps(favorites))
 for mode in range(4):
  prefs['carousel_layout']=mode;(config/'settings.json').write_text(json.dumps(prefs));run(app,'--apply',paths[0]);h=start(10+mode);wait(lambda:window(h.pid));wait(lambda:trace().get('hits'));time.sleep(.5);assert trace()['selected']==0
  move_to(h.pid,target(1));
  if mode==0:wait(lambda:trace()['selected']==1)
  wait(lambda:trace()['focus']>.9);assert max(hit[3]for hit in trace()['hits']if hit[0]==(1 if mode==0 else 0))>trace()['tile']*1.3
  move_to(h.pid,target(1))
  w=window(h.pid);run('grim','-g',f'{w["at"][0]},{w["at"][1]} {w["size"][0]}x{w["size"][1]}',out/f'mode-{mode}.png');run('ydotool','click','0xC0');h.wait(35);assert h.returncode==0 and status()['path']==paths[1];assert json.loads((config/'carousel.json').read_text())==favorites;results[str(mode)]={'mouse_hover_zoom':mode==0,'selection_zoom':True,'mouse_click_applies':True,'favorite_order_preserved':True}
 stable=status();h=start(20);wait(lambda:window(h.pid));wait(lambda:trace().get('hits'));time.sleep(.5);move_to(h.pid,target(1));run('ydotool','click','0xC0');h.wait(35);assert status()==stable;results['click_current_is_noop']=True
finally:
 for h in hosts:
  if h.poll()is None:
   try:run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.close({{window="pid:{h.pid}"}}))');h.wait(40)
   except Exception:pass
 run(app,'--stop');daemon.terminate();daemon.wait(5);sock.unlink(missing_ok=True);(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));print(json.dumps(results,indent=2))

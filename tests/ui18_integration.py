#!/usr/bin/env python3
"""Native UI: direct mirroring toggles, atomic image resets and all locale menus."""
import argparse,json,pathlib,subprocess,os,time,shlex,shutil
from PIL import Image,ImageChops,ImageStat
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);source=out/'originals';source.mkdir(exist_ok=True);bin_=out/'bin';bin_.mkdir(exist_ok=True);sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-ui18-{os.getpid()}.sock'
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'XDG_DATA_HOME':str(out/'data'),'XDG_CACHE_HOME':str(out/'cache'),'WALLPAPER_LAB_UI_STATE':str(out/'ui.json'),'YDOTOOL_SOCKET':str(sock),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps']),'PATH':str(bin_)+':'+os.environ['PATH']}
log=open(out/'log','w');daemon=subprocess.Popen(['ydotoold','--socket-path='+str(sock)],stdout=log,stderr=log);ui=None;results={}
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,text=True)
def wait(fn,timeout=25):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  try:value=fn()
  except(FileNotFoundError,json.JSONDecodeError,StopIteration,KeyError):value=None
  if value:return value
  assert not ui or ui.poll()is None,(out/'log').read_text()[-2000:];time.sleep(.1)
 raise AssertionError('condition timeout')
def read_state(file):
 for _ in range(20):
  try:return json.loads(file.read_text())
  except json.JSONDecodeError:time.sleep(.01)
 raise AssertionError('state file stayed incomplete')
def g():return read_state(out/'ui.json.geometry')
def s():return read_state(out/'ui.json.style')
def state():return read_state(out/'ui.json')
def w():return next(w for w in json.loads(run('hyprctl','clients','-j'))if w['pid']==ui.pid)
def move(x,y):
 q=w();target=(round(q['at'][0]+x),round(q['at'][1]+y))
 for _ in range(3):
  run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={target[0]},y={target[1]}}}))');time.sleep(.12)
  pos=json.loads(run('hyprctl','cursorpos','-j'))
  if abs(pos['x']-target[0])+abs(pos['y']-target[1])<3:break
 assert abs(pos['x']-target[0])+abs(pos['y']-target[1])<5,('pointer moved unexpectedly',pos,target)
def click(x,y):
 for _ in range(5):
  active=json.loads(run('hyprctl','activewindow','-j'))
  if active.get('pid')!=ui.pid:run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.4)
  move(x,y)
  if json.loads(run('hyprctl','activewindow','-j')).get('pid')==ui.pid:run('ydotool','click','0xC0');time.sleep(.3);return
 raise AssertionError('test window cannot acquire focus safely')
def esc():run('ydotool','key','1:1','1:0');time.sleep(.25)
def shot(name):
 q=w();file=out/(name+'.png');run('grim','-g',f'{q["at"][0]},{q["at"][1]} {q["size"][0]}x{q["size"][1]}',file);return Image.open(file)
def choose(id,index):
 for attempt in range(3):
  click(*g()['choices'][str(id)]['control']);time.sleep(.2)
  if g()['choice_open']==id:break
 wait(lambda:g()['choice_open']==id)
 for _ in range(25):
  pt=g()['choices'][str(id)]['options'][index]
  if pt[0]>0:
   click(*pt);wait(lambda:g()['choice_open']!=id);return
  pts=[pt for pt in g()['choices'][str(id)]['options']if pt[0]>0];move(*pts[-1]);run('ydotool','mousemove','--wheel','--','0','3'if index<g()['choice_scroll']else'-3');time.sleep(.2)
 raise AssertionError(('choice',id,index))
def sample(im,rect):
 x,y,width,height=rect;return im.crop((round(x+width*.45),round(y+height*.65),round(x+width*.45)+10,round(y+height*.65)+10))
def status():return json.loads(run(app,'--status'))



personal=json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));file=None
try:
 fixture=source/'Reset.png';Image.new('RGB',(1280,720),(70,120,170)).save(fixture);file=json.loads(run(app,'--import',fixture))[0];prefs={'workshop_path':str(steam),'mode':'single','screen':0,'fps':30,'muted':True,'fullscreen_policy':0,'language':'it','ui_theme':4,'theme_catalog':True};(config/'settings.json').write_text(json.dumps(prefs));original={'image':{'brightness':.2,'contrast':1.4,'saturation':.7,'temperature':.5,'hue':30,'gamma':1.6,'vignette':.3,'rotation':45,'flip_x':1,'flip_y':1},'position':{'x':.3,'y':-.2,'zoom':1.3},'playback':{'audio':False}};(config/'wallpaper_profiles.json').write_text(json.dumps({file:original}));run(app,'--apply',file);stable=status();wait(sock.exists);time.sleep(1);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log);wait(w);wait(lambda:'library_cards'in g());m=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']=='DP-6');run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.5);card=g()['library_cards'][0];click(card[0]+card[2]/2,card[1]+card[3]/2);wait(lambda:state()['detail'])
 for _ in range(24):
  point=g()['image_resets'][0]
  if 250<point[1]<w()['size'][1]-235:break
  move(w()['size'][0]-150,550);run('ydotool','mousemove','--wheel','--','0','-3');time.sleep(.3)
 time.sleep(.5);assert len(g()['image_resets'])==1;before=shot('single-red-reset');click(*g()['image_resets'][0]);profile=lambda:json.loads((config/'wallpaper_profiles.json').read_text())[file];wait(lambda:profile()['image']=={});assert profile()['position']==original['position']and profile()['playback']==original['playback'];assert status()==stable;results['one_red_reset_all_image_values_preserves_crop_playback']=True
 click(*g()['header']['settings']);wait(lambda:state()['settings']);click(*g()['settings_tabs'][3]);wait(lambda:s()['settings_tab']==3);point=g()['choices']['38']['control'];assert point[1]>g()['settings_tabs'][4][1]-6;shot('collection-top-unclipped');counts=[5,6,9,3,3,3];seen=[]
 for group,count in enumerate(counts):
  choose(38,group);pref=json.loads((config/'settings.json').read_text());assert pref['theme_collection']==group;assert len(g()['choices']['30']['options'])==count,(group,g()['choices']['30']);
  for variant in range(count):
   choose(30,variant);pref=json.loads((config/'settings.json').read_text());assert pref['theme_collection']==group;seen.append(pref['ui_theme'])
  shot('collection-'+str(group))
 assert len(set(seen))==29;results['six_collections_filter_all_29_themes_and_persist']=True;assert status()==stable
finally:
 if __import__('sys').exc_info()[0]and ui and ui.poll()is None:
  try:shot('failure')
  except Exception:pass
 try:run('ydotool','click','0x80')
 except Exception:pass
 if ui and ui.poll()is None:ui.terminate();ui.wait(15)
 run(app,'--stop');daemon.terminate();daemon.wait(5);sock.unlink(missing_ok=True);(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));print(json.dumps(results,indent=2))

#!/usr/bin/env python3
"""Native UI: direct mirroring toggles, atomic image resets and all locale menus."""
import argparse,json,pathlib,subprocess,os,time,shlex,shutil
from PIL import Image,ImageChops,ImageStat
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);source=out/'originals';source.mkdir(exist_ok=True);bin_=out/'bin';bin_.mkdir(exist_ok=True);sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-ui17-{os.getpid()}.sock'
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
 active=json.loads(run('hyprctl','activewindow','-j'))
 if active.get('pid')!=ui.pid:
  run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.4)
 move(x,y);run('ydotool','click','0xC0');time.sleep(.3)
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


personal=json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True))
def profile():return json.loads((config/'wallpaper_profiles.json').read_text())[file]
def inspect_visible(key,index):
 for _ in range(24):
  point=g()[key][index]
  if 260<point[1]<w()['size'][1]-235:break
  move(w()['size'][0]-150,550);run('ydotool','mousemove','--wheel','--','0','-3');time.sleep(.3)
 time.sleep(.5);point=g()[key][index];assert 230<point[1]<w()['size'][1]-215,(key,point);return point
try:
 fixture=source/'Reset fixture.png';Image.new('RGB',(1280,720),(60,115,170)).save(fixture);file=json.loads(run(app,'--import',fixture))[0]
 prefs={'workshop_path':str(steam),'mode':'single','screen':0,'fps':30,'muted':True,'fullscreen_policy':0,'language':'it','ui_theme':0,'theme_catalog':True};(config/'settings.json').write_text(json.dumps(prefs));profile_file=config/'wallpaper_profiles.json';original={'position':{'x':.2,'y':-.1,'zoom':1.3},'playback':{'audio':False},'properties':{'creator-test':'keep'},'image':{'brightness':.15,'contrast':1.2,'saturation':.75,'temperature':.4,'hue':35,'gamma':1.7,'vignette':.4,'rotation':35,'flip_x':0,'flip_y':0}};profile_file.write_text(json.dumps({file:original}));run(app,'--apply',file);stable=status();wait(sock.exists);time.sleep(1.1);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log);wait(w);wait(lambda:'library_cards'in g());m=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']=='DP-6');run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.5)
 card=g()['library_cards'][0];click(card[0]+card[2]/2,card[1]+card[3]/2);wait(lambda:state()['detail']);assert status()==stable
 for index,key in enumerate(['flip_x','flip_y']):
  point=inspect_visible('mirror_toggles',index);click(*point);wait(lambda:profile()['image'].get(key)==1);assert g()['choice_open']<0;assert status()==stable
 shot('toggles-before-reset');results['both_direct_toggles_persist_no_restart']=True
 point=inspect_visible('image_resets',1);click(*point);wait(lambda:set(profile()['image'])=={'brightness','contrast','saturation'});after=profile();assert after['image']=={k:original['image'][k]for k in ['brightness','contrast','saturation']};assert all(after[k]==original[k]for k in ['position','playback','properties']);assert status()==stable;shot('additional-effects-reset');results['reset_additional_preserves_basic_position_playback_creator']=True
 click(*inspect_visible('image_resets',0));wait(lambda:profile()['image']=={});assert all(profile()[k]==original[k]for k in ['position','playback','properties']);assert status()==stable;results['reset_all_image_preserves_other_sections_no_restart']=True
 # An already neutral image must remain valid on repeated reset.
 click(*inspect_visible('image_resets',1));assert profile()['image']=={};results['neutral_reset_idempotent']=True
 click(*g()['header']['settings']);wait(lambda:state()['settings']);click(*g()['settings_tabs'][4]);wait(lambda:s()['settings_tab']==4)
 codes=['it','en','de','es','fr','pt','nl','pl','ro','sv','zh-CN','zh-TW','ja','ko','hi','ru','ar','bn','id','tr','vi','ur'];catalog=json.loads((pathlib.Path(__file__).resolve().parents[1]/'src/translations.json').read_text())
 for index,code in enumerate(codes):
  choose(40,index);wait(lambda:s()['language']==index);assert json.loads((config/'settings.json').read_text())['language']==code;shot('language-'+code)
  for tab in [2,3,7]:click(*g()['settings_tabs'][tab]);wait(lambda:s()['settings_tab']==tab)
  shot('integration-'+code);click(*g()['settings_tabs'][4]);wait(lambda:s()['settings_tab']==4)
 results['22_languages_live_switch_persistence_and_four_settings_tabs']=True;assert status()==stable
 assert 'Failed to compile'not in(out/'log').read_text()
finally:
 if __import__('sys').exc_info()[0]and ui and ui.poll()is None:
  try:shot('failure')
  except Exception:pass
 try:run('ydotool','click','0x80')
 except Exception:pass
 if ui and ui.poll()is None:ui.terminate();ui.wait(15)
 run(app,'--stop');daemon.terminate();daemon.wait(5);sock.unlink(missing_ok=True);(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));print(json.dumps(results,indent=2))

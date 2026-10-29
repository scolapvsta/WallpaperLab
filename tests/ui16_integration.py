#!/usr/bin/env python3
"""Native UI: map targets, compact animation controls, footer and live image settings."""
import argparse,json,pathlib,subprocess,os,time,shlex,shutil
from PIL import Image,ImageChops,ImageStat
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);source=out/'originals';source.mkdir(exist_ok=True);bin_=out/'bin';bin_.mkdir(exist_ok=True);sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-ui16-{os.getpid()}.sock'
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

personal=json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));files=[]
def slider(id,fraction):
 time.sleep(.45);rect=g()['number_tracks'][str(id)];click(rect[0]+rect[2]*fraction,rect[1]+2);time.sleep(.2)
def preview_close(kind,point):
 click(*point);wait(lambda:s()[kind]);shot(kind)
 esc();wait(lambda:not s()[kind]);click(*point);wait(lambda:s()[kind]);click(25,290);wait(lambda:not s()[kind]);click(*point);wait(lambda:s()[kind]);q=w();click((q['size'][0]-720)/2+687,(q['size'][1]-460)/2+29);wait(lambda:not s()[kind])
try:
 for name,color in [('AAA',(45,115,185)),('BBB',(185,70,50)),('CCC',(60,175,105))]:
  file=source/(name+'.png');fixture=Image.new('RGB',(1280,720),color)
  from PIL import ImageDraw
  draw=ImageDraw.Draw(fixture)
  for x in range(1280):draw.line((x,0,x,719),fill=((color[0]+x//8)%255,(color[1]+x//14)%255,color[2]))
  fixture.save(file);files.append(json.loads(run(app,'--import',file))[0])
 prefs={'theme_catalog':True,'ui_theme':2,'window_transparency':0,'workshop_path':str(steam),'mode':'single','screen':0,'fps':30,'muted':True,'carousel_layout':0};(config/'settings.json').write_text(json.dumps(prefs));(config/'carousel.json').write_text(json.dumps([files[2],files[0],files[1]]));run(app,'--apply',files[0]);stable=status();wait(sock.exists);time.sleep(1.2);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log);wait(w);wait(lambda:'library_cards'in g());m=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']=='DP-6');run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.6)
 card=g()['library_cards'][0];click(card[0]+card[2]/2,card[1]+card[3]/2);wait(lambda:state()['detail']);time.sleep(1);assert status()['pid']==stable['pid'];assert '69'not in g()['choices'];results['active_noop_and_no_inspector_monitor_control']=True
 before=shot('ui-smoothed-icons-text');card=g()['library_cards'][0];preview=g()['preview_rect'];a1=sample(before,card);a2=sample(before,preview);run(app,'--ui-option','window_transparency','30');wait(lambda:s()['transparency']==30);time.sleep(.4);after=shot('transparent');assert ImageChops.difference(a1,sample(after,card)).getbbox()is None;assert ImageChops.difference(a2,sample(after,preview)).getbbox()is None;run(app,'--ui-option','window_transparency','0');results['opaque_media_regression']=True
 assert g()['choices']['57']['control'][1]>w()['size'][1]-100;choose(57,2);assert json.loads((config/'settings.json').read_text())['page_size']==200;results['page_size_in_footer']=True
 click(*g()['header']['display']);wait(lambda:s()['display_popup']);choose(1,2);click(*g()['display_cards'][1]);wait(lambda:g()['screen']==1);shot('monitor-target');esc();wait(lambda:not s()['display_popup']);card=g()['library_cards'][1];click(card[0]+card[2]/2,card[1]+card[3]/2);wait(lambda:not s()['applying']);wait(lambda:len(status().get('outputs',{}))==2);assert status()['outputs']['DP-6']['pid']==stable['pid'];assert status()['outputs']['DP-5']['path']==files[1];stable=status();results['map_highlight_selects_destination_without_apply']=True
 click(*g()['header']['carousel']);wait(lambda:state()['page']==1);preview_close('switcher_preview',g()['animation_controls'][0]);preview_close('transition_preview',g()['animation_controls'][1]);assert status()==stable;assert not any(q['title']=='Wallpaper Lab Carousel'for q in json.loads(run('hyprctl','clients','-j')));results['both_in_app_previews_x_escape_outside']=True
 click(*g()['animation_controls'][2]);wait(lambda:s()['customize']==1);slider(100,.6);slider(101,.5);slider(102,.3);slider(103,.7);slider(104,.65);shot('switcher-customization');esc();wait(lambda:s()['customize']==0);assert json.loads((config/'settings.json').read_text())['switcher_size']>100
 choose(20,3);click(*g()['animation_controls'][2]);wait(lambda:s()['customize']==1);slider(105,.7);slider(106,.5);choose(75,1);shot('wave-customization');esc();results['switcher_and_wave_settings_persist']=True
 choose(22,5);click(*g()['animation_controls'][3]);wait(lambda:s()['customize']==2);slider(107,.4);choose(76,1);choose(77,2);shot('transition-customization');esc();pref=json.loads((config/'settings.json').read_text());assert pref['transition_direction']==2 and pref['transition_curve']==1 and pref['transition_duration']>900;results['transition_duration_direction_curve_persist']=True
 click(*g()['header']['library']);wait(lambda:state()['page']==0);card=g()['library_cards'][1];click(card[0]+card[2]/2,card[1]+card[3]/2);wait(lambda:state()['detail'])
 for _ in range(12):
  track=g()['number_tracks'].get('120',[0,9999,0])
  if 260<track[1]<w()['size'][1]-190:break
  move(w()['size'][0]-170,520);run('ydotool','mousemove','--wheel','--','0','-3');time.sleep(.25)
 slider(120,.65);profile=json.loads((config/'wallpaper_profiles.json').read_text());assert profile[files[1]]['image']['temperature']>.2;results['image_temperature_live_slider_persists']=True
 for _ in range(12):
  point=g()['choices'].get('81',{}).get('control',[0,9999])
  if 260<point[1]<w()['size'][1]-190:break
  move(w()['size'][0]-170,520);run('ydotool','mousemove','--wheel','--','0','-3');time.sleep(.25)
 time.sleep(.45);choose(81,1);profile=json.loads((config/'wallpaper_profiles.json').read_text());assert profile[files[1]]['image']['flip_x']==1;shot('image-transforms');results['mirror_control_persists_without_restart']=True;assert status()==stable
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

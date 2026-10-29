#!/usr/bin/env python3
"""Private-XDG pointer demo: selection no-op, opaque media, themes, import and dismissals."""
import argparse,json,pathlib,subprocess,os,time,shlex,shutil
from PIL import Image,ImageChops,ImageStat
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);source=out/'originals';source.mkdir(exist_ok=True);bin_=out/'bin';bin_.mkdir(exist_ok=True);sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-ui15-{os.getpid()}.sock'
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
 q=w();run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={int(q["at"][0]+x)},y={int(q["at"][1]+y)}}}))');time.sleep(.08)
def click(x,y):
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.12);move(x,y);run('ydotool','click','0xC0');time.sleep(.25)
def esc():run('ydotool','key','1:1','1:0');time.sleep(.25)
def shot(name):
 q=w();file=out/(name+'.png');run('grim','-g',f'{q["at"][0]},{q["at"][1]} {q["size"][0]}x{q["size"][1]}',file);return Image.open(file)
def choose(id,index):
 click(*g()['choices'][str(id)]['control']);wait(lambda:g()['choice_open']==id)
 for _ in range(25):
  pt=g()['choices'][str(id)]['options'][index]
  if pt[0]>0:click(*pt);return
  pts=[pt for pt in g()['choices'][str(id)]['options']if pt[0]>0];move(*pts[-1]);run('ydotool','mousemove','--wheel','--','0','3'if index<g()['choice_scroll']else'-3');time.sleep(.2)
 raise AssertionError(('choice',id,index))
def sample(im,rect):
 x,y,width,height=rect;return im.crop((round(x+width*.45),round(y+height*.65),round(x+width*.45)+10,round(y+height*.65)+10))
def status():return json.loads(run(app,'--status'))
personal=json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));files=[]
try:
 for name,color in [('AAA',(45,115,185)),('BBB',(185,70,50)),('CCC',(60,175,105))]:
  file=source/(name+'.png');fixture=Image.new('RGB',(1280,720),color)
  if name=='AAA':
   # A spatial gradient makes accidental whole-window alpha observable even when
   # the same wallpaper is behind the window: desktop and preview UVs differ.
   from PIL import ImageDraw
   draw=ImageDraw.Draw(fixture)
   for x in range(1280):draw.line((x,0,x,719),fill=(35+x*170//1280,95+x*60//1280,190-x*120//1280))
  fixture.save(file);files.append(json.loads(run(app,'--import',file))[0])
 extra=source/'Imported via file picker.png';Image.new('RGB',(1024,512),(120,70,180)).save(extra);picker=bin_/'zenity';picker.write_text('#!/bin/sh\nprintf "%s\\n" '+shlex.quote(str(extra))+'\n');picker.chmod(0o700)
 prefs={'theme_catalog':True,'ui_theme':0,'window_transparency':0,'workshop_path':str(steam),'mode':'single','screen':0,'fps':30,'muted':True,'carousel_layout':0};(config/'settings.json').write_text(json.dumps(prefs));(config/'carousel.json').write_text(json.dumps([files[2],files[0],files[1]]));run(app,'--apply',files[0]);stable=status();wait(sock.exists);time.sleep(1.2);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log);wait(w);wait(lambda:'library_cards'in g());m=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']=='DP-6');run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.6)
 card=g()['library_cards'][0];click(card[0]+card[2]/2,card[1]+card[3]/2);wait(lambda:state()['detail']);time.sleep(1.2);assert status()['pid']==stable['pid'] and not s()['applying'];results['active_selection_no_renderer_restart']=True
 before=shot('opaque');card=g()['library_cards'][0];preview=g()['preview_rect'];a1=sample(before,card);a2=sample(before,preview);run(app,'--ui-option','window_transparency','30');wait(lambda:s()['transparency']==30);time.sleep(.4);after=shot('transparent-ui');assert ImageChops.difference(a1,sample(after,card)).getbbox()is None,'grid media faded';assert ImageChops.difference(a2,sample(after,preview)).getbbox()is None,'live scene faded';results['grid_and_live_preview_remain_opaque']=True
 # Single-monitor target selector disabled in span, enabled in single mode.
 pref=json.loads((config/'settings.json').read_text());pref['mode']='span';(config/'settings.json').write_text(json.dumps(pref));time.sleep(1.3);point=g()['choices']['69']['control'];click(*point);assert g()['choice_open']!=69;pref['mode']='single';(config/'settings.json').write_text(json.dumps(pref));time.sleep(1.3);click(*g()['choices']['69']['control']);wait(lambda:g()['choice_open']==69);esc();before_outputs=status()['outputs'];choose(69,1);wait(lambda:not s()['applying']);wait(lambda:len(status().get('outputs',{}))==2);assert status()['outputs']['DP-6']['pid']==before_outputs['DP-6']['pid'];stable=status();results['target_monitor_gated_by_single_mode']=True;results['inspector_assigns_second_monitor_independently']=True
 click(*g()['header']['import']);wait(lambda:g()['library_total']==4);shot('imported-local-files');assert status()['pid']==stable['pid'];results['native_picker_import_and_auto_refresh']=True
 click(*g()['header']['filter']);wait(lambda:'58'in g()['choices']);choose(58,2);assert g()['library_total']==4;choose(58,1);wait(lambda:g()['library_total']==0);choose(58,0);wait(lambda:g()['library_total']==4);results['origin_filter']=True
 click(*g()['header']['settings']);click(*g()['settings_tabs'][3]);run(app,'--ui-option','window_transparency','0');wait(lambda:s()['transparency']==0)
 dark=[0,2,3,4,6,7,8,9,10,14,15];light=[1,5,11,12,13]
 for family,ids in [(0,dark),(1,light)]:
  choose(38,family)
  for index,id in enumerate(ids):choose(30,index);wait(lambda:s()['theme']==id);assert s()['light']==bool(family);shot('theme-'+str(id))
 # Transparency slider width is contained within the same right-hand entry column.
 track=g()['opacity_track'];field=g()['choices']['30']['control'];assert track[2]<292 and track[0]>field[0]-160;results['sixteen_independent_themes_and_compact_slider']=True
 click(*g()['settings_tabs'][6]);shot('library-paths');click(*g()['settings_tabs'][7]);shot('caelestia-settings');esc();wait(lambda:not state()['settings']);click(*g()['header']['carousel']);wait(lambda:state()['page']==1)
 click(708,214);wait(lambda:s()['transition_preview']);esc();wait(lambda:not s()['transition_preview']);assert state()['page']==1
 click(708,214);wait(lambda:s()['transition_preview']);click(35,300);wait(lambda:not s()['transition_preview']);assert status()['pid']==stable['pid'];results['transition_escape_and_outside_click']=True
 for cancel in ['escape','outside']:
  click(708,170);child=wait(lambda:next((r for r in json.loads(run('hyprctl','clients','-j'))if r['title']=='Wallpaper Lab Carousel'and r['pid']!=ui.pid),None));time.sleep(.3)
  if cancel=='escape':run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{child["pid"]}"}}))');esc()
  else:click(35,300)
  wait(lambda:not any(r['pid']==child['pid']for r in json.loads(run('hyprctl','clients','-j'))));assert status()['pid']==stable['pid']
 results['switcher_escape_and_outside_click']=True
 assert 'Failed to compile'not in(out/'log').read_text()
finally:
 try:run('ydotool','click','0x80')
 except Exception:pass
 if ui and ui.poll()is None:ui.terminate();ui.wait(15)
 run(app,'--stop');daemon.terminate();daemon.wait(5);sock.unlink(missing_ok=True);(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));print(json.dumps(results,indent=2))

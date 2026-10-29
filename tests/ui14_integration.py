#!/usr/bin/env python3
"""Opt-in Hyprland pointer regression: paging, group filters, image sliders and overlays.
Personal assets and the desktop renderer are untouched; all changes use private XDG files.
"""
import argparse,json,pathlib,subprocess,os,time,shutil
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
lib=out/'library';lib.mkdir(exist_ok=True)
for n in range(205):
 d=lib/str(1000+n);d.mkdir(exist_ok=True);(d/'project.json').write_text(json.dumps({'type':'web','title':f'Test wallpaper {n:03d}','preview':'preview.png','file':'index.html','tags':['1920 x 1080']}));(d/'index.html').write_text('<style>html{background:#704020}</style>');im=Image.new('RGB',(320,180),'#245183');im.paste('#f5bc80',(0,0,160,90));im.save(d/'preview.png')
config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);(config/'settings.json').write_text(json.dumps({'fps':30,'page_size':100,'ui_theme':4,'light_theme':False}));(config/'carousel.json').write_text(json.dumps([str(lib/'1000'),str(lib/'1001')]))
sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab14-{os.getpid()}.sock';env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'WALLPAPER_LAB_LIBRARY':str(lib),'WALLPAPER_LAB_UI_STATE':str(out/'ui.json'),'WALLPAPER_LAB_UI_READ_ONLY':'1','YDOTOOL_SOCKET':str(sock),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','deps','lib'])};log=open(out/'log','w');daemon=subprocess.Popen(['ydotoold','--socket-path='+str(sock)],stdout=log,stderr=log);ui=None;results={}
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,text=True)
def wait(fn,timeout=20):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  try:v=fn()
  except(FileNotFoundError,json.JSONDecodeError,StopIteration,KeyError):v=None
  if v:return v
  assert not ui or ui.poll()is None,(out/'log').read_text()[-2000:];time.sleep(.1)
 raise AssertionError('timeout')
def g():return json.loads((out/'ui.json.geometry').read_text())
def s():return json.loads((out/'ui.json.style').read_text())
def state():return json.loads((out/'ui.json').read_text())
def w():return next(w for w in json.loads(run('hyprctl','clients','-j'))if w['pid']==ui.pid)
def move(x,y):
 q=w();run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={int(q["at"][0]+x)},y={int(q["at"][1]+y)}}}))');time.sleep(.08)
def click(x,y):
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.1);move(x,y);run('ydotool','click','0xC0');time.sleep(.25)
def shot(n):
 q=w();run('grim','-g',f'{q["at"][0]},{q["at"][1]} {q["size"][0]}x{q["size"][1]}',out/(n+'.png'))
def choose(id,index):
 point=g()['choices'][str(id)]['control'];click(*point);wait(lambda:g()['choice_open']==id)
 for _ in range(20):
  pt=g()['choices'][str(id)]['options'][index]
  if pt[0]>0:click(*pt);return
  pts=[p for p in g()['choices'][str(id)]['options']if p[0]>0];move(*pts[-1]);run('ydotool','mousemove','--wheel','--','0','3'if index<g()['choice_scroll']else'-3');time.sleep(.2)
 raise AssertionError('choice not found')
def visible_track(key,index=None):
 for _ in range(35):
  q=g()[key]if index is None else g()[key][index];h=w()['size'][1]
  if q[2]>0 and 235<q[1]<h-195:return q
  move(w()['size'][0]-120,h/2);run('ydotool','mousemove','--wheel','--','0','-3'if q[1]>=h-195 else'3');time.sleep(.3)
 raise AssertionError(('offscreen',key,q))
try:
 wait(sock.exists);time.sleep(1.2);run('ydotool','click','0x80');ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log);wait(w);wait(lambda:'choices'in g());m=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']=='DP-6');run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.8)
 assert state()['results']==100 and g()['library_total']==205,(state(),g()['library_total']);click(w()['size'][0]-64,w()['size'][1]-66);wait(lambda:g()['library_page']==2);assert state()['results']==100;click(64,w()['size'][1]-66);wait(lambda:g()['library_page']==1);results['library_paging']=True
 click(*g()['header']['filter']);wait(lambda:'57'in g()['choices']);choose(57,2);wait(lambda:state()['results']==200);choose(57,0);wait(lambda:state()['results']==50)
 group=g()['resolution_groups'][0];click(*group);wait(lambda:state()['results']==50);assert g()['library_total']==205;shot('resolution-groups');results['whole_group_works_with_AND']=True
 click(*g()['choices']['52']['control']);fixed=g()['choices']['57']['control'];time.sleep(.2);assert fixed==g()['choices']['57']['control'];shot('dropdown-overlay');run('ydotool','key','1:1','1:0');time.sleep(.2)
 # Select a wallpaper and adjust its image controls, without launching its renderer.
 click(450,290);wait(lambda:state()['detail']);time.sleep(.8)
 for n,value in enumerate([.7,.8,.2]):
  track=visible_track('image_tracks',n);click(track[0]+track[2]*value,track[1]);time.sleep(.2)
 profiles=config/'wallpaper_profiles.json';wait(profiles.exists);data=json.loads(profiles.read_text());image=next(iter(data.values()))['image'];assert .35<image['brightness']<.45 and 1.5<image['contrast']<1.7 and .3<image['saturation']<.5,image;shot('image-controls');results['image_sliders_persist']=image
 # The five local playback dropdowns persist independently, even under global veto.
 for control,key in zip(range(64,69),['audio','reactive','mouse','parallax','particles']):
  for _ in range(25):
   pt=g()['choices'][str(control)]['control'];h=w()['size'][1]
   if 235<pt[1]<h-195:break
   move(w()['size'][0]-120,h/2);run('ydotool','mousemove','--wheel','--','0','-3'if pt[1]>=h-195 else'3');time.sleep(.25)
  choose(control,0);wait(lambda:key in next(iter(json.loads(profiles.read_text()).values())).get('playback',{}))
 assert all(not v for v in next(iter(json.loads(profiles.read_text()).values()))['playback'].values());results['local_playback_dropdowns_saved']=True
 # Resize filters via the same splitter interaction as the inspector.
 old=g()['filter_width'];move(old,430);run('ydotool','click','0x40');time.sleep(.15);run('ydotool','mousemove','--','70','0');time.sleep(.25);run('ydotool','click','0x80');wait(lambda:g()['filter_width']>old+40);results['resizable_filters']=True
 # Appearance overlay and independent opacity control.
 click(*g()['header']['settings']);click(*g()['settings_tabs'][3]);wait(lambda:'30'in g()['choices']);track=g()['opacity_track'];click(track[0]+track[2]*.4,track[1]);wait(lambda:json.loads((config/'settings.json').read_text()).get('window_opacity',100)<90);shot('appearance-opacity');results['opacity_saved']=True
 choose(30,5);shot('pastel');choose(30,3);shot('pink-neon');choose(30,4);shot('pastel-dark')
 click(*g()['settings_tabs'][4]);
 for locale in range(22):
  choose(40,locale);wait(lambda:s()['language']==locale);shot(f'locale-{locale}')
 choose(40,0);run('ydotool','key','1:1','1:0');time.sleep(.2)
 results['locale_switching']=True
 click(*g()['header']['carousel']);wait(lambda:'22'in g()['choices']);choose(22,1);shot('carousel');click(708,214);time.sleep(.4);shot('transition-preview');results['transition_preview_rendered']=True
 assert 'Failed to compile'not in(out/'log').read_text();results['shader_compilation']=True
finally:
 try:run('ydotool','click','0x80')
 except Exception:pass
 if ui and ui.poll()is None:ui.terminate();ui.wait(10)
 daemon.terminate();daemon.wait(5);sock.unlink(missing_ok=True);(out/'results.json').write_text(json.dumps(results,indent=2))
print(json.dumps(results,indent=2))

#!/usr/bin/env python3
"""Theme-scoped palettes, dropdown themes, native backgrounds and visual smoke tests."""
import argparse,json,pathlib,subprocess,os,time,shlex,shutil
from PIL import Image,ImageChops,ImageStat
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);source=out/'originals';source.mkdir(exist_ok=True);bin_=out/'bin';bin_.mkdir(exist_ok=True);sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-ui22-{os.getpid()}.sock'
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
  run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={target[0]},y={target[1]},relative=false}}))');time.sleep(.12)
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




personal=json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True))
env['WALLPAPER_LAB_UI_READ_ONLY']='1'
root=pathlib.Path(__file__).resolve().parents[1]
env['LD_LIBRARY_PATH']+=':'+str(root/'local/sysroot/usr/lib')+':'+str(root/'build-engine/output')
fixture=out/'library/1000';fixture.mkdir(parents=True,exist_ok=True);env['WALLPAPER_LAB_LIBRARY']=str(fixture.parent)
(fixture/'project.json').write_text(json.dumps({'type':'web','title':'Theme / layout fixture','file':'index.html','preview':'preview.png'}))
(config/'carousel.json').write_text(json.dumps([str(fixture)]))
(fixture/'index.html').write_text('<body>UI fixture</body>');Image.new('RGB',(640,360),(100,122,168)).save(fixture/'preview.png')
(config/'settings.json').write_text(json.dumps({'theme_catalog':True,'theme_variants':True,'theme_ui_v2':True,'ui_design':1,'ui_theme':29,'light_theme':True,'font_family':'DejaVu Sans','window_transparency':30,'language':'it','carousel_layout':1}))
selection=out/'selection.txt';selection.write_text('')
picker=bin_/'zenity';picker.write_text('#!/bin/sh\ncat '+shlex.quote(str(selection))+'\n');picker.chmod(0o755)
background=source/"App's pastel backdrop.png";im=Image.new('RGB',(1600,900),(112,158,193));__import__('PIL.ImageDraw',fromlist=['ImageDraw']).Draw(im).rectangle((800,0,1600,900),fill=(193,123,153));im.save(background)
bad=source/'broken.png';bad.write_text('not an image')
def pref():return json.loads((config/'settings.json').read_text())
def toggle(id):
 assert g()['choices'][str(id)]['kind']=='toggle';old=g()['choices'][str(id)]['value'];click(*g()['choices'][str(id)]['control']);wait(lambda:g()['choices'][str(id)]['value']!=old)
def launch():
 global ui
 ui=subprocess.Popen([str(app),'--appearance-settings'],env=env,stdout=log,stderr=log);wait(w)
 monitors=json.loads(run('hyprctl','monitors','-j'));m=next((m for m in monitors if m['name']=='DP-6'),monitors[-1])
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.5)
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.float({{action="on",window="pid:{ui.pid}"}})); hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x=1440,y=900,relative=false}})); hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}})); hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))')
 time.sleep(.8);wait(lambda:'38'in g()['choices']);time.sleep(.6)
def close():ui.terminate();ui.wait(15)
try:
 wait(sock.exists);launch()
 assert s()['design']==1 and s()['light']is True and '31'not in g()['choices']
 choose(38,1);wait(lambda:s()['design']==2);assert s()['light']is False
 assert len(g()['choices']['30']['options'])==6
 for i in range(6):
  choose(30,i);wait(lambda:s()['theme']==3+i);shot('2077-palette-'+str(i))
 # Theme font list is curated; its last action expands it to installed families.
 small=len(g()['choices']['33']['options']);choose(33,small-1);wait(lambda:len(g()['choices']['33']['options'])>small)
 all_count=len(g()['choices']['33']['options']);choose(33,1);assert pref()['font_family']=='Rajdhani'
 choose(33,all_count-1);wait(lambda:len(g()['choices']['33']['options'])==5)
 for index,name in [(2,'Oxanium'),(3,'Orbitron'),(1,'Rajdhani')]:
  choose(33,index);wait(lambda:pref()['font_family']==name);shot('font-'+name)
 click(*g()['choices']['33']['control']);wait(lambda:g()['choice_open']==33);shot('font-menu-single-label');esc()
 results['theme_fonts_curated_show_all_and_single_render']=True
 assert g()['background_button_size'][0]<160 and g()['background_button_size'][1]==34
 for preset in range(4):
  choose(39,preset);wait(lambda:pref().get('cyber_background',0)==preset);im=shot('2077-background-'+str(preset))
  px,py=g()['app_background_controls'][0];thumb=im.crop((round(px-157),round(py-50),round(px-93),round(py+3)))
  assert max(ImageStat.Stat(thumb).stddev)>.8,('blank background preview',preset)
 results['compact_controls_four_presets_and_preview']=True
 selection.write_text(str(background)+'\n');click(*g()['app_background_controls'][0]);wait(lambda:pref().get('app_background'))
 copied=pathlib.Path(pref()['app_background']);assert copied.is_file() and copied!=background;background.unlink();close();launch();assert copied.is_file();shot('2077-custom-background')
 selection.write_text(str(bad)+'\n');click(*g()['app_background_controls'][0]);time.sleep(2);assert pref()['app_background']==str(copied)
 click(*g()['app_background_controls'][1]);wait(lambda:not pref().get('app_background')and pref().get('cyber_background',0)==0)
 results['background_import_restore_and_invalid_preserves_previous']=True
 choose(30,5);close();launch();assert s()['theme']==8 and s()['light']is False and pref()['font_family']=='Rajdhani'
 choose(38,0);wait(lambda:s()['design']==1);assert s()['light']is True
 choose(38,1);wait(lambda:s()['design']==2);assert s()['theme']==8 and s()['light']is False
 results['theme_owns_mode_and_restores_palette']=True
 click(*g()['settings_tabs'][4]);toggle(41);assert pref()['reduce_motion'];click(*g()['settings_tabs'][3]);esc();wait(lambda:not state()['settings']);shot('2077-library')
 # The real Workshop read-only path is optional when Steam has no connection.
 click(*g()['header']['workshop']);wait(lambda:state()['page']==3)
 deadline=time.monotonic()+35
 while time.monotonic()<deadline and (g()['workshop']['busy']or not g()['workshop']['count']):time.sleep(.3)
 if g()['workshop']['count']>=12:
  time.sleep(2);before=g()['text_rasterizations'];time.sleep(1);idle=g()['text_rasterizations']-before;fps=[]
  for n in range(12):
   move(w()['size'][0]*.55,w()['size'][1]*.6);run('ydotool','mousemove','--wheel','--','0','-2'if n<6 else'2');time.sleep(.15);fps.append(s()['fps'])
  time.sleep(.8);shot('workshop-smooth-scroll');results['workshop_scroll']={'items':g()['workshop']['count'],'median_fps':sorted(fps)[len(fps)//2],'idle_text_uploads':idle};assert idle<10 and sorted(fps)[len(fps)//2]>=40
 else:results['workshop_scroll']='Steam/network unavailable; CPU/GPU text-cache regression tested separately'
 click(*g()['header']['library']);click(*g()['header']['carousel']);wait(lambda:state()['page']==1)
 for layout in range(4):
  choose(20,layout);click(*g()['animation_controls'][0]);wait(lambda:s()['switcher_preview']);time.sleep(.5);shot('2077-layout-'+str(layout));esc();wait(lambda:not s()['switcher_preview'])
 results['four_layouts']=True
finally:
 if __import__('sys').exc_info()[0] and ui and ui.poll()is None:
  try:shot('failure')
  except Exception:pass
 if ui and ui.poll()is None:close()
 daemon.terminate();daemon.wait(5);sock.unlink(missing_ok=True);(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True))
print(json.dumps(results,indent=2))

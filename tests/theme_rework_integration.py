#!/usr/bin/env python3
"""Theme-scoped palettes, dropdown themes, native backgrounds and visual smoke tests."""
import argparse,json,pathlib,subprocess,os,time,shlex,shutil
from PIL import Image,ImageChops,ImageStat
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);source=out/'originals';source.mkdir(exist_ok=True);bin_=out/'bin';bin_.mkdir(exist_ok=True);sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-ui21-{os.getpid()}.sock'
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
(config/'settings.json').write_text(json.dumps({'theme_catalog':True,'theme_variants':True,'theme_ui_v2':True,'ui_design':1,'ui_theme':29,'light_theme':True,'font_family':'DejaVu Sans','language':'it','carousel_layout':1}))
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
 assert s()['theme']==0 and s()['design']==1 and s()['light']is True
 assert g()['choices']['38']['kind']=='dropdown' and len(g()['choices']['38']['options'])==2
 assert len(g()['choices']['30']['options'])==3
 for i in range(3):
  choose(30,i);wait(lambda:s()['theme']==i);shot('strawberry-palette-'+str(i))
 toggle(31);shot('strawberry-dark');choose(30,1)
 choose(38,1);wait(lambda:s()['design']==2);assert s()['theme']==3 and s()['light']is False
 assert len(g()['choices']['30']['options'])==5
 for i in range(5):
  choose(30,i);wait(lambda:s()['theme']==3+i);shot('2077-palette-'+str(i))
 choose(30,3);toggle(31);assert s()['theme']==6;shot('2077-light')
 choose(38,0);wait(lambda:s()['design']==1);assert s()['theme']==1 and s()['light']is False
 choose(38,1);wait(lambda:s()['design']==2);assert s()['theme']==6 and s()['light']is True
 close();launch();assert s()['design']==2 and s()['theme']==6 and s()['light']is True
 assert pref()['font_family']=='DejaVu Sans';results['dropdown_scoped_palettes_modes_font_independence_restart']=True
 # Import through the same asynchronous file picker used by the appearance button.
 selection.write_text(str(background)+'\n');click(*g()['app_background_controls'][0]);wait(lambda:pref().get('app_background'))
 copied=pathlib.Path(pref()['app_background']);assert copied.is_file() and copied!=background and 'wallpapers'not in copied.parts
 background.unlink();close();launch();assert pathlib.Path(pref()['app_background']).is_file();shot('2077-custom-background')
 selection.write_text(str(bad)+'\n');click(*g()['app_background_controls'][0]);time.sleep(2);assert pref()['app_background']==str(copied)
 click(*g()['app_background_controls'][1]);wait(lambda:not pref().get('app_background'));results['background_copy_survives_source_removal_and_restart_invalid_preserves_previous_clear']=True
 # Freeze effects, then inspect the unobstructed city backdrop and all preview layouts.
 click(*g()['settings_tabs'][4]);toggle(41);assert pref()['reduce_motion'];click(*g()['settings_tabs'][3]);toggle(31)
 esc();wait(lambda:not state()['settings']);first=shot('2077-library');time.sleep(.5);second=shot('2077-reduced-motion');assert ImageChops.difference(first.crop((500,500,900,720)),second.crop((500,500,900,720))).getbbox()is None
 click(*g()['header']['carousel']);wait(lambda:state()['page']==1)
 for layout in range(4):
  choose(20,layout);assert pref()['carousel_layout']==layout
  click(*g()['animation_controls'][0]);wait(lambda:s()['switcher_preview']);time.sleep(.8);shot('2077-layout-'+str(layout));esc();wait(lambda:not s()['switcher_preview'])
 results['four_switcher_layout_previews_and_reduce_motion']=True
 click(*g()['header']['settings']);wait(lambda:state()['settings']);click(*g()['settings_tabs'][3]);choose(38,0);toggle(31);esc();wait(lambda:not state()['settings']);click(*g()['header']['library']);shot('strawberry-library')
finally:
 if __import__('sys').exc_info()[0] and ui and ui.poll()is None:
  try:shot('failure')
  except Exception:pass
 if ui and ui.poll()is None:close()
 daemon.terminate();daemon.wait(5);sock.unlink(missing_ok=True);(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True))
print(json.dumps(results,indent=2))

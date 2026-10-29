#!/usr/bin/env python3
"""Theme-scoped palettes, dropdown themes, native backgrounds and visual smoke tests."""
import argparse,json,pathlib,subprocess,os,time,shlex,shutil
from PIL import Image,ImageChops,ImageStat
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);source=out/'originals';source.mkdir(exist_ok=True);bin_=out/'bin';bin_.mkdir(exist_ok=True);sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-ui23-{os.getpid()}.sock'
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'XDG_DATA_HOME':str(out/'data'),'XDG_CACHE_HOME':str(out/'cache'),'WALLPAPER_LAB_UI_STATE':str(out/'ui.json'),'YDOTOOL_SOCKET':str(sock),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])+(':'+os.environ['LD_LIBRARY_PATH'] if os.environ.get('LD_LIBRARY_PATH') else ''),'PATH':str(bin_)+':'+os.environ['PATH']}
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
 for _ in range(6):
  run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={target[0]},y={target[1]},relative=false}}))');time.sleep(.035)
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
env['WALLPAPER_LAB_UI_READ_ONLY']='1';root=pathlib.Path(__file__).resolve().parents[1]
env['LD_LIBRARY_PATH']+=':'+str(root/'local/sysroot/usr/lib')+':'+str(root/'build-engine/output')
library=out/'library';library.mkdir(exist_ok=True);env['WALLPAPER_LAB_LIBRARY']=str(library);fixtures=[]
for i,color in enumerate([(55,95,145),(175,92,120),(60,135,100)]):
 fixture=library/str(1000+i);fixture.mkdir(exist_ok=True);fixtures.append(str(fixture))
 (fixture/'project.json').write_text(json.dumps({'type':'web','title':'Theme fixture '+str(i),'file':'index.html','preview':'preview.png'}));(fixture/'index.html').write_text('<body>UI fixture</body>');Image.new('RGB',(640,360),color).save(fixture/'preview.png')
(config/'carousel.json').write_text(json.dumps(fixtures))
(config/'settings.json').write_text(json.dumps({'theme_scoped_v3':True,'ui_design':0,'ui_theme':14,'font_family':'DejaVu Sans','general_font_enabled':False,'language':'it','carousel_layout':0,'reduce_motion':True}))
def pref():return json.loads((config/'settings.json').read_text())
def launch():
 global ui
 ui=subprocess.Popen([str(app),'--appearance-settings'],env=env,stdout=log,stderr=log);wait(w)
 monitors=json.loads(run('hyprctl','monitors','-j'));m=next((m for m in monitors if m['name']=='DP-6'),monitors[-1])
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}));hl.dispatch(hl.dsp.window.float({{action="on",window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x=1440,y=900,relative=false}}));hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.8)
 wait(lambda:'38'in g()['choices'])
def close():ui.terminate();ui.wait(15)
try:
 wait(sock.exists);launch();assert len(g()['choices']['38']['options'])==4
 palette_counts=[3,3,11,4];background_hashes={}
 for design in range(4):
  choose(38,design);wait(lambda:s()['design']==design);assert len(g()['choices']['30']['options'])==palette_counts[design]
  assert s()['light']==(design==1);assert not pref()['general_font_enabled'];expected=['Sans','Sans','Oxanium','Silkscreen'][design];assert g()['effective_font']==expected
  hashes=[]
  for preset in range(4):
   choose(39,preset);time.sleep(.15);im=shot('theme-'+str(design)+'-background-'+str(preset));px,py=g()['app_background_controls'][0];crop=im.crop((int(px-157),int(py-50),int(px-93),int(py+3)));assert max(ImageStat.Stat(crop).stddev)>.4;hashes.append(__import__('hashlib').sha256(crop.tobytes()).hexdigest())
  assert len(set(hashes))==4; background_hashes[design]=hashes
 results['four_themes_palettes_distinct_backgrounds_fonts']=True
 # General Fonts is a genuine opt-in override over the suggested family.
 choose(38,2);choose(33,1);wait(lambda:g()['effective_font']=='Rajdhani');click(*g()['general_font_check']);wait(lambda:g()['general_font_enabled']);assert g()['effective_font']=='DejaVu Sans'
 choose(33,2);assert g()['effective_font']=='DejaVu Sans';choose(35,2);wait(lambda:g()['effective_font']=='Oxanium');click(*g()['general_font_check']);wait(lambda:not g()['general_font_enabled']);assert g()['effective_font']=='Orbitron'
 results['suggested_and_general_font_checkbox_override']=True
 for palette in range(6,11):choose(30,palette)
 shot('cyber-extra-palettes');choose(30,1);choose(33,0)
 choose(38,3);choose(39,0);click(*g()['settings_tabs'][4])
 def motion_toggle():
  old=g()['choices']['41']['value'];click(*g()['choices']['41']['control']);wait(lambda:g()['choices']['41']['value']!=old)
 motion_toggle();esc();wait(lambda:not state()['settings']);a1=shot('pixel-animation-a');time.sleep(.35);a2=shot('pixel-animation-b');area=(800,600,1400,850);assert sum(max(px[:3])>3 for px in ImageChops.difference(a1.crop(area),a2.crop(area)).get_flattened_data())>20
 click(*g()['header']['settings']);wait(lambda:state()['settings']);motion_toggle();esc();wait(lambda:not state()['settings']);a1=shot('pixel-reduced-motion-a');time.sleep(.35);a2=shot('pixel-reduced-motion-b');assert max(channel[1]for channel in ImageChops.difference(a1.crop(area),a2.crop(area)).getextrema())<=1
 results['pixel_animation_and_reduce_motion']=True
 click(*g()['header']['settings']);wait(lambda:state()['settings']);click(*g()['settings_tabs'][3])
 choose(38,1);choose(39,0);esc();wait(lambda:not state()['settings'])
 move(700,780);time.sleep(.2);before=shot('berry-cookie-idle');pt=g()['header']['refresh'];move(*pt);time.sleep(.2);after=shot('berry-cookie-hover');box=(int(pt[0]-18),int(pt[1]-18),int(pt[0]+18),int(pt[1]+18));assert ImageChops.difference(before.crop(box),after.crop(box)).getbbox()is not None
 results['cookie_hover_feedback']=True
 # All sixteen preview combinations use the same shared skin as the popup.
 for design in range(4):
  click(*g()['header']['settings']);wait(lambda:state()['settings']);choose(38,design);esc();wait(lambda:not state()['settings']);click(*g()['header']['carousel']);wait(lambda:state()['page']==1)
  for layout in range(4):
   choose(20,layout);click(*g()['animation_controls'][0]);wait(lambda:s()['switcher_preview']);time.sleep(.5);shot('preview-theme-'+str(design)+'-layout-'+str(layout));esc();wait(lambda:not s()['switcher_preview'])
 close();ui=None;results['sixteen_switcher_previews']=True
 # Capture the actual compositor popups using the synthetic library, then cancel.
 for design,palette,rgb in [(0,14,(156,203,189)),(1,1,(172,49,91)),(2,4,(255,105,198)),(3,17,(255,133,200))]:
  run(app,'--ui-option','ui_design',design);run(app,'--ui-option','ui_theme',palette);run(app,'--ui-option','general_font_enabled',0)
  for layout in range(4):
   if design==2 and layout==3:run(app,'--ui-option','font',0);run(app,'--ui-option','general_font_enabled',1)
   run(app,'--ui-option','carousel_layout',layout);capture=out/('popup-'+str(design)+'-'+str(layout));capture.mkdir(exist_ok=True);trace=capture/'trace.json';session=design*10+layout+1
   pe=env|{'WALLPAPER_LAB_POPUP_CAPTURE':str(capture),'WALLPAPER_LAB_POPUP_STATE':str(trace)}
   child=subprocess.Popen([str(app),'--carousel-event','theme-test',str(session),'1','step'],env=pe,stdout=log,stderr=log)
   try:
    deadline=time.monotonic()+15
    while not(capture/'step-1-2.png').exists():assert child.poll()is None;assert time.monotonic()<deadline;time.sleep(.05)
    state_=json.loads(trace.read_text());assert(state_['design'],state_['theme'])==(design,palette)
    assert state_['font']==('Sans'if design==2 and layout==3 else['Sans','Sans','Oxanium','Silkscreen'][design]);assert state_['general_font_override']==(design==2 and layout==3)
    im=Image.open(capture/'step-1-2.png').convert('RGBA');matching=sum(1 for px in im.get_flattened_data()if px[3]>200 and sum(abs(px[i]-rgb[i])for i in range(3))<35);assert matching>15,(design,layout,matching)
    client=next(c for c in json.loads(run('hyprctl','clients','-j'))if c['pid']==child.pid);run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.close({{window="pid:{child.pid}"}}))');child.wait(8)
   finally:
    if child.poll()is None:
     proc=pathlib.Path('/proc',str(child.pid));assert pathlib.Path(os.readlink(proc/'exe')).resolve()==app;child.terminate();child.wait(8)
 results['sixteen_actual_popups_match_theme_palette']=True
 assert not(out/'state/wallpaper-lab/running.json').exists()
finally:
 if __import__('sys').exc_info()[0]and ui and ui.poll()is None:
  try:shot('failure')
  except Exception:pass
 if ui and ui.poll()is None:close()
 daemon.terminate();daemon.wait(5);sock.unlink(missing_ok=True);(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));print(json.dumps(results,indent=2))

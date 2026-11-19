"""Opt-in regression: Cyberpunk wordmark stays above the toolbar while resizing."""
import argparse,json,os,subprocess,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--app',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True)
library=out/'library';library.mkdir(exist_ok=True)
helper=out/'caelestia-fixture.py'
# Only dependency preparation is stubbed; this isolated visual check has no renderer.
helper.write_text('import json,sys\nprint(json.dumps({"python":sys.executable}))\n')
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'XDG_DATA_HOME':str(out/'data'),'XDG_CACHE_HOME':str(out/'cache'),'WALLPAPER_LAB_LIBRARY':str(library),'WALLPAPER_LAB_UI_STATE':str(out/'ui.json'),'WALLPAPER_LAB_UI_READ_ONLY':'1','WALLPAPER_LAB_CAELESTIA_HELPER':str(helper)}
(config/'settings.json').write_text(json.dumps({'theme_scoped_v3':True,'ui_design':2,'ui_theme':24,'language':'en','reduce_motion':False,'caelestia_enabled':False}))
log=(out/'launcher.log').open('w');ui=None

def run(*args):return subprocess.check_output([str(a)for a in args],env=env,text=True)
def geom():return json.loads((out/'ui.json.geometry').read_text())
def style():return json.loads((out/'ui.json.style').read_text())
def window():return next(w for w in json.loads(run('hyprctl','clients','-j')) if w['pid']==ui.pid)
def wait(fn):
 end=time.monotonic()+20
 while time.monotonic()<end:
  assert ui.poll()is None,(out/'launcher.log').read_text()[-2000:]
  try:
   result=fn()
   if result:return result
  except(OSError,ValueError,KeyError,StopIteration):pass
  time.sleep(.1)
 raise AssertionError('layout timeout')
def overlap(a,b):return a[2]>0 and a[3]>0 and b[2]>0 and b[3]>0 and a[0]<b[0]+b[2] and b[0]<a[0]+a[2] and a[1]<b[1]+b[3] and b[1]<a[1]+a[3]
def check():
 g=geom();r=g['header_decoration'];w,h=window()['size']
 if r[2]>0:
  assert r[0]>=0 and r[0]+r[2]<=w and r[1]>=0
  assert r[1]+r[3]<89,('decoration reaches toolbar',r,w)
  controls=[g['power_rect']]
  for name in ['settings','filter','refresh','display','import']:
   x,y=g['header'][name];controls.append([x-22,y-23,44,46])
  for box in controls:assert not overlap(r,box),(r,box,w)
 return r
results=[]
try:
 ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log);wait(window);wait(lambda:'header_decoration'in geom())
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.float({{action="on",window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))')
 for integrations in [False,True]:
  run(app,'--ui-option','caelestia_enabled',int(integrations))
  wait(lambda:style()['integrations_visible']==integrations)
  # Activation may prepare dependencies; no renderer exists in this isolated state.
  for width in [1000,1264,1400,1900]:
   run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x={width},y=900,relative=false}}));hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}}))')
   wait(lambda:abs(window()['size'][0]-width)<5);time.sleep(1.2)
   r=check()
   if width>=1264:assert r[2]>0
   if width==1000 and integrations:assert r[2]==0
   results.append({'width':width,'integrations':integrations,'decoration_visible':r[2]>0,'bounds':r})
   w=window();x,y=w['at'];sw,sh=w['size'];run('grim','-g',f'{x},{y} {sw}x{sh}',out/f'header-{width}-{int(integrations)}.png')
   # Sample multiple animation phases; glitch geometry is included in the bounds.
   for _ in range(12):check();time.sleep(.1)
 # A font override must also honor the first-row bounds.
 run(app,'--ui-option','general_font_enabled',1);time.sleep(1.2);assert check()[2]>0
 (out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
finally:
 if ui and ui.poll()is None:ui.terminate();ui.wait(timeout=15)
 log.close()

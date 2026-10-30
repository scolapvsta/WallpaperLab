#!/usr/bin/env python3
"""Capture native popup canvases without mapping windows or using desktop input."""
import argparse,json,os,pathlib,re,subprocess,time
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
root=pathlib.Path(__file__).resolve().parents[1];config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);library=out/'library';library.mkdir(exist_ok=True)
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'XDG_DATA_HOME':str(out/'data'),'XDG_CACHE_HOME':str(out/'cache'),'WALLPAPER_LAB_LIBRARY':str(library),'WALLPAPER_LAB_UI_READ_ONLY':'1','WALLPAPER_LAB_POPUP_HIDDEN':'1','LD_LIBRARY_PATH':':'.join([str(root/'local/sysroot/usr/lib'),str(root/'build-engine/output'),os.environ.get('LD_LIBRARY_PATH','')])}
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,text=True)
def read(file):
 for _ in range(25):
  try:return json.loads(file.read_text())
  except(FileNotFoundError,json.JSONDecodeError):time.sleep(.02)
 raise AssertionError(('incomplete state',file))
installed=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=subprocess.check_output([installed,'--status'],text=True)
fixtures=[]
for i,color in enumerate([(55,95,145),(175,92,120),(60,135,100)]):
 d=library/str(1000+i);d.mkdir(exist_ok=True);fixtures.append(str(d));(d/'project.json').write_text(json.dumps({'type':'web','title':'Color fixture '+str(i),'file':'index.html','preview':'preview.png'}));(d/'index.html').write_text('<body>Read-only fixture</body>');Image.new('RGB',(320,180),color).save(d/'preview.png')
(config/'carousel.json').write_text(json.dumps(fixtures));(config/'settings.json').write_text(json.dumps({'theme_scoped_v3':True,'ui_design':3,'ui_theme':17,'switcher_theme':3,'switcher_palette':19,'reduce_motion':True,'language':'it'}))
# Palette IDs are stable: three original dark colors, seven new dark, ten light.
ids=[14,15,16,*range(21,38)];entries=[]
for line in(root/'src/theme_catalog.h').read_text().splitlines():
 if re.match(r'\s*\{"',line):entries.append(re.findall(r'0x([0-9a-f]{6})',line))
assert len(ids)==20
results={'palettes':20,'native_popup_captures':0,'hidden_surfaces':True};log=(out/'log').open('w')
for n,palette in enumerate(ids):
 run(app,'--ui-option','switcher_palette',palette);run(app,'--ui-option','ui_design',n%4)
 saved=read(config/'settings.json');assert saved['switcher_palette']==palette and 'switcher_theme'not in saved
 for layout in(range(4)if palette in[14,28,24,31]else[0]):
  run(app,'--ui-option','carousel_layout',layout);capture=out/f'palette-{palette}-layout-{layout}';capture.mkdir(exist_ok=True);trace=capture/'trace.json';pe=env|{'WALLPAPER_LAB_POPUP_CAPTURE':str(capture),'WALLPAPER_LAB_POPUP_STATE':str(trace)}
  child=subprocess.Popen([app,'--carousel-event','colors-test',str(1+n*4+layout),'1','step'],env=pe,stdout=log,stderr=log)
  proc=pathlib.Path('/proc',str(child.pid));start=(proc/'stat').read_text().rsplit(')',1)[1].split()[19]
  try:
   deadline=time.monotonic()+15
   while not(capture/'step-1-2.png').exists():
    assert child.poll()is None,log.name
    assert time.monotonic()<deadline,('capture timeout',palette,layout)
    time.sleep(.05)
   state=read(trace);assert(state['design'],state['theme'])==(0,palette),state
   im=Image.open(capture/'step-1-2.png').convert('RGBA');rgb=tuple(bytes.fromhex(entries[palette][-1]));matching=sum(px[3]>200 and sum(abs(px[i]-rgb[i])for i in range(3))<35 for px in im.get_flattened_data());assert matching>15,(palette,layout,matching)
   assert max(px[3]for px in im.crop((0,0,im.width,10)).get_flattened_data())==0,('decorations survived',palette,layout)
   results['native_popup_captures']+=1
  finally:
   if child.poll()is None:
    assert os.readlink(proc/'exe')==str(app)and(proc/'stat').read_text().rsplit(')',1)[1].split()[19]==start
    child.terminate();child.wait(8)
assert not(out/'state/wallpaper-lab/running.json').exists();assert before==subprocess.check_output([installed,'--status'],text=True)
results.update({'independent_colors':True,'plain_default_style':True,'transparent_margins':True,'personal_renderer_preserved':True});(out/'results.json').write_text(json.dumps(results,indent=2));print(json.dumps(results,indent=2))

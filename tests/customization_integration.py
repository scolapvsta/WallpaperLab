#!/usr/bin/env python3
"""Native settings/refresh checks with isolated preferences and a fake bind API.
Real pointer input is used. Only cursor/window operations reach the compositor;
shortcut persistence and validation target a disposable Hyprland configuration.
"""
import argparse,json,os,pathlib,re,shutil,subprocess,tempfile,time
from PIL import Image,ImageDraw
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--refresh-only',action='store_true');a=p.parse_args()
if not a.desktop:p.error('--desktop required')
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);results={}
def run(*args,env=None):return subprocess.check_output([str(x) for x in args],text=True,env=env)
def wait(fn,timeout=15):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  try:value=fn()
  except (FileNotFoundError,json.JSONDecodeError):value=None
  if value:return value
  time.sleep(.06)
 raise AssertionError('UI condition timed out')
installed=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=json.loads(run(installed,'--status'));realhypr=shutil.which('hyprctl')
with tempfile.TemporaryDirectory(prefix='customization-',dir=out) as tmp,(out/'ui.log').open('w') as log:
 root=pathlib.Path(tmp).resolve();config=root/'config';library=root/'library';(config/'hypr').mkdir(parents=True);(root/'bin').mkdir();(config/'hypr/hyprland.lua').write_text('-- disposable test configuration\n')
 def project(number,title,color):
  folder=library/str(number);folder.mkdir(parents=True)
  im=Image.new('RGB',(320,200),color);draw=ImageDraw.Draw(im);draw.ellipse((100,25,220,145),fill=(180,220,235));im.save(folder/'preview.png')
  (folder/'index.html').write_text('<!doctype html><html><style>html,body{margin:0;width:100%;height:100%;background:linear-gradient(45deg,#216ca0,#c5ebad)}</style></html>')
  (folder/'project.json').write_text(json.dumps({'title':title,'type':'web','file':'index.html','preview':'preview.png'}))
 project(1,'B Wallpaper',(30,58,80));project(2,'C Wallpaper',(62,32,78))
 fake=root/'bin/hyprctl';fake.write_text('''#!/usr/bin/env python3
import os,sys,json,pathlib,re
args=sys.argv[1:];root=pathlib.Path(os.environ['XDG_CONFIG_HOME'])
if args[0]=='systeminfo':print('configProvider: lua')
elif args[0]=='reload':print('ok')
elif args[0]=='binds':
 file=root/'wallpaper-lab/shortcut.lua';text=file.read_text() if file.exists() else '';match=re.search(r'hl.bind\\("([^"\\n]+)"',text);result=[]
 if match:
  parts=match[1].split('+');bits={'SUPER':64,'CTRL':4,'ALT':8,'SHIFT':1};mask=sum(bits.get(x,0) for x in parts[:-1]);result.append({'modmask':mask,'key':parts[-1],'description':'Wallpaper Lab carousel'})
  for token,name in [('SUPER','Super'),('CTRL','Control'),('ALT','Alt'),('SHIFT','Shift')]:
   if token in parts:
    for side in ['_L','_R']:result.append({'modmask':0,'key':name+side,'description':'Wallpaper Lab carousel release '+name+side,'release':True,'non_consuming':True})
 print(json.dumps(result))
elif args[0]=='eval' and 'hl.unbind' in args[1]:print('ok')
else:os.execv(os.environ['REAL_HYPRCTL'],[os.environ['REAL_HYPRCTL']]+args)
''');fake.chmod(0o755)
 socket=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'wallpaper-style-{os.getpid()}.sock';statefile=root/'state.json'
 env=os.environ|{'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','deps','lib']),'XDG_CONFIG_HOME':str(config),'XDG_STATE_HOME':str(root/'state'),'WALLPAPER_LAB_LIBRARY':str(library),'WALLPAPER_LAB_UI_STATE':str(statefile),'YDOTOOL_SOCKET':str(socket),'PATH':str(root/'bin')+os.pathsep+os.environ['PATH'],'REAL_HYPRCTL':realhypr}
 daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log);ui=None
 try:
  wait(socket.exists);time.sleep(2);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log)
  def window():return next((w for w in json.loads(run(realhypr,'clients','-j')) if w['pid']==ui.pid),None)
  wait(window)
  displays=json.loads(run(realhypr,'monitors','-j'));target=next((m for m in displays if m['name']=='DP-6'),displays[-1]);run(realhypr,'eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{target["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.6)
  def state():return wait(lambda:json.loads(statefile.read_text()))
  def geom():return wait(lambda:json.loads(pathlib.Path(str(statefile)+'.geometry').read_text()))
  def style():return wait(lambda:json.loads(pathlib.Path(str(statefile)+'.style').read_text()))
  wait(lambda:geom()['header']['settings'][0]>100)
  def click(x,y):
   run(realhypr,'eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.15);w=window();run(realhypr,'eval',f'hl.dispatch(hl.dsp.cursor.move({{x={w["at"][0]+int(x)},y={w["at"][1]+int(y)}}}))');time.sleep(.08);run('ydotool','click','0xC0',env=env);time.sleep(.22)
  def key(*values):run('ydotool','key',*values,env=env);time.sleep(.18)
  def shot(name):
   w=window();run('grim','-g',f'{w["at"][0]},{w["at"][1]} {w["size"][0]}x{w["size"][1]}',out/(name+'.png'))
  h=geom()['header'];assert h['filter'][0]<h['moon'][0] and h['settings'][1]<h['filter'][1];results['header_positions']=True
  if not a.refresh_only:
   click(*h['settings']);wait(lambda:state()['settings']);click(*geom()['settings_tabs'][4]);wait(lambda:style()['settings_tab']==4)
   def visible(key,index=None):
    for attempt in range(24):
     target=geom()[key] if index is None else geom()[key][index];w=window()
     if 250<target[1]<w['size'][1]-180:return target
     run(realhypr,'eval',f'hl.dispatch(hl.dsp.cursor.move({{x={w["at"][0]+w["size"][0]//2},y={w["at"][1]+w["size"][1]//2}}}))')
     run('ydotool','mousemove','--wheel','--','0','-3' if target[1]>w['size'][1]-180 else '3',env=env);time.sleep(.35)
    raise AssertionError(('offscreen control',key,index,target))
   def choose(control,index):
    click(*geom()['choices'][str(control)]['control']);wait(lambda:geom()['choice_open']==control)
    for _ in range(30):
     g=geom();point=g['choices'][str(control)]['options'][index]
     if point[0]>0 and 230<point[1]<window()['size'][1]-190:click(*point);break
     center=g['choices'][str(control)]['control'];w=window();run(realhypr,'eval',f'hl.dispatch(hl.dsp.cursor.move({{x={int(w["at"][0]+center[0])},y={int(w["at"][1]+center[1]+58)}}}))');time.sleep(.12)
     run('ydotool','mousemove','--wheel','--','0','-3'if index>g['choice_scroll']else'3',env=env);time.sleep(.2)
    else:raise AssertionError(('dropdown option unavailable',control,index,geom()))
    wait(lambda:geom()['choice_open']<0)
   for language in range(10):
    choose(40,language);wait(lambda:style()['language']==language);shot(f'language-{language}')
   results['ten_languages_dropdown']=True
   choose(40,1);wait(lambda:style()['language']==1)
   click(*geom()['settings_tabs'][3]);wait(lambda:style()['settings_tab']==3)
   for theme in [0,*range(2,11)]:
    choose(30,theme-1 if theme else 0);wait(lambda:style()['theme']==theme)
    for variant in range(2):
     choose(31,1-variant);wait(lambda:style()['light']==(variant==0));assert style()['theme']==theme;time.sleep(.12);shot(f'theme-{theme}-'+('light' if variant==0 else 'dark'))
   results['ten_themes_both_variants']=True
   for density in range(3):
    choose(32,density);prefs=json.loads(run(app,'--ui-preferences',env=env));assert prefs['settings'].get('layout_density',0)==density
   results['layout_density']=True
   prefs=json.loads(run(app,'--ui-preferences',env=env));index=prefs['fonts'].index('DejaVu Sans');revision=style()['font_revision'];choose(33,index);wait(lambda:style()['font']==index and style()['font_revision']>revision);shot('font-selected');results['font_reload_without_restart']=True
   revision=style()['font_revision'];choose(34,2);wait(lambda:style()['font_revision']>revision);choose(34,1);results['font_weight_and_preview']=True
   if 'Noto Sans CJK SC' in prefs['fonts']:
    collection=prefs['fonts'].index('Noto Sans CJK SC');run(app,'--ui-option','font',str(collection),env=env);wait(lambda:style()['font']==collection);time.sleep(.3);shot('font-collection');run(app,'--ui-option','font',str(index),env=env);wait(lambda:style()['font']==index);results['collection_font_fallback']=True
   key('1:1','1:0');wait(lambda:not state()['settings']);click(*geom()['header']['carousel']);wait(lambda:state()['page']==1)
   click(*geom()['shortcut']);wait(lambda:state()['recording']);assert not state()['settings'];shot('shortcut-recording')
   # Select an unused physical chord so no real compositor action is triggered.
   realbinds=json.loads(run(realhypr,'binds','-j'));assert not any(b.get('modmask')==13 and b.get('key','').lower()=='f12' for b in realbinds),'test chord is already bound'
   key('29:1','56:1','42:1','88:1');time.sleep(.2);key('88:0','42:0','56:0','29:0');wait(lambda:not state()['recording'])
   expected='CTRL+ALT+SHIFT+F12';wait(lambda:style()['shortcut']==expected);assert json.loads((config/'wallpaper-lab/shortcut.json').read_text())['combo']==expected
   click(*geom()['header']['settings']);wait(lambda:state()['settings']);click(*geom()['settings_tabs'][2]);wait(lambda:style()['settings_tab']==2);assert style()['shortcut']==expected;shot('shortcut-settings');results['inline_shortcut_and_settings_synced']=True
   key('1:1','1:0');click(*geom()['header']['library']);wait(lambda:state()['page']==0)
  click(*geom()['header']['library']);wait(lambda:state()['page']==0)
  # Give refresh an active selection and a favorite, then insert a title before it.
  g=geom();click(20+g['tile']-24,208);click(100,300)
  running=root/'state/wallpaper-lab/running.json';first=wait(lambda:json.loads(running.read_text()) if running.exists() else None);time.sleep(.3)
  favorites=json.loads((config/'wallpaper-lab/carousel.json').read_text());project(3,'A Newly downloaded',(20,75,65))
  click(*geom()['header']['refresh']);wait(lambda:state()['results']==3 and not style()['scanning']);assert state()['selection']==1 and state()['detail'];assert json.loads(running.read_text())==first;assert json.loads((config/'wallpaper-lab/carousel.json').read_text())==favorites
  shot('library-refreshed');results['refresh_preserves_selection_favorites_renderer']=True
  # Exercise a realistic dense creator schema without launching a heavy wallpaper.
  dense=json.loads((library/'1/project.json').read_text())
  dense['general']={'properties':{f'option{n}':{'type':'slider','value':n/100,'min':0,'max':1,'order':n,'text':f'Creator setting {n}'} for n in range(102)}}
  (library/'1/project.json').write_text(json.dumps(dense));click(*geom()['header']['refresh']);wait(lambda:not style()['scanning']);time.sleep(1)
  samples=[]
  for _ in range(12):samples.append(style()['creator_ms']);time.sleep(.1)
  assert min(samples)>0,('creator panel was not drawn',samples)
  assert max(samples)<10,('creator controls block the UI',samples)
  results['dense_creator_panel_max_ms']=max(samples)
  results['dense_creator_panel_fps']=style()['fps'];shot('dense-creator-controls')
  saved=json.loads((config/'wallpaper-lab/settings.json').read_text())
  if not a.refresh_only:assert saved['language']=='en' and saved['font_family']=='DejaVu Sans' and saved['ui_theme']==10
  assert 'Failed to compile' not in (out/'ui.log').read_text();results['preferences_persisted']=True
 finally:
  try:run('ydotool','click','0x80',env=env)
  except Exception:pass
  if ui and ui.poll() is None:
   try:shot('last')
   except Exception:pass
   ui.terminate();ui.wait(timeout=12)
  subprocess.run([str(app),'--stop'],env=env,stdout=log,stderr=log,check=True);daemon.terminate();daemon.wait(timeout=5);socket.unlink(missing_ok=True)

  if statefile.exists():shutil.copy2(statefile,out/'last-state.json')
  for suffix in ['.geometry','.style']:
   if pathlib.Path(str(statefile)+suffix).exists():shutil.copy2(pathlib.Path(str(statefile)+suffix),out/('last'+suffix+'.json'))
  if (config/'wallpaper-lab/settings.json').exists():shutil.copy2(config/'wallpaper-lab/settings.json',out/'last-settings.json')
  (out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
assert before==json.loads(run(installed,'--status')),'personal renderer changed'
print(json.dumps(results,indent=2))

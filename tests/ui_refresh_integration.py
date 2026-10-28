#!/usr/bin/env python3
"""0.8 pointer checks on disposable web projects and isolated renderer state.
Requires an unlocked session; never unlocks or changes personal wallpapers.
"""
import argparse,json,os,pathlib,subprocess,tempfile,time,shutil
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args()
if not a.desktop:p.error('--desktop required: moves the pointer and briefly renders static fixtures')
a.output.mkdir(parents=True,exist_ok=True);app=a.app.resolve();results={}
def run(*args,env=None):return subprocess.check_output([str(x) for x in args],text=True,env=env)
def wait(fn):
 end=time.monotonic()+12
 while time.monotonic()<end:
  try:value=fn()
  except (FileNotFoundError,json.JSONDecodeError):value=None
  if value:return value
  time.sleep(.05)
 raise AssertionError('UI condition timed out')
installed=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=json.loads(run(installed,'--status'))
with tempfile.TemporaryDirectory(prefix='ui-fixtures-',dir=a.output.resolve()) as tmp,(a.output/'mouse.log').open('w') as log:
 root=pathlib.Path(tmp);config=root/'config/wallpaper-lab';library=root/'library';config.mkdir(parents=True)
 for index,tags in [(1,['3840 x 1080','5120 x 1440']),(2,['1920 x 1080'])]:
  project=library/str(index);project.mkdir(parents=True)
  (project/'project.json').write_text(json.dumps({'title':f'Fixture {index}','type':'web','file':'index.html','tags':tags}))
  (project/'index.html').write_text('<!doctype html><html><style>html,body{margin:0;width:100%;height:100%;background:linear-gradient(45deg,#216ca0,#c5ebad)}</style></html>')
 socket=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'wallpaper-ui-{os.getpid()}.sock';statefile=root/'ui-state.json'
 env=os.environ|{'XDG_CONFIG_HOME':str(root/'config'),'XDG_STATE_HOME':str(root/'state'),'XDG_DATA_HOME':str(root/'data'),'WALLPAPER_LAB_LIBRARY':str(library),'YDOTOOL_SOCKET':str(socket),'WALLPAPER_LAB_UI_STATE':str(statefile)}
 daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log);ui=None
 try:
  wait(socket.exists);time.sleep(2);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log)
  def window():return next((w for w in json.loads(run('hyprctl','clients','-j')) if w['pid']==ui.pid),None)
  wait(window);time.sleep(1)
  def state():return json.loads(statefile.read_text())
  def geometry():return wait(lambda:json.loads(pathlib.Path(str(statefile)+'.geometry').read_text()))
  def move(x,y):
   w=window();x=x(w) if callable(x) else x;y=y(w) if callable(y) else y
   run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={w["at"][0]+int(x)},y={w["at"][1]+int(y)}}}))')
  def click(x,y):
   run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.2);move(x,y);time.sleep(.08);run('ydotool','click','0xC0',env=env);time.sleep(.22)
  def active(index):
   path=root/'state/wallpaper-lab/running.json'
   return path.exists() and json.loads(path.read_text()).get('path')==str(library/str(index))
  click(lambda w:20+geometry()['tile']-24,208)
  assert (config/'carousel.json').exists(),'Pointer did not reach library: session may be locked'
  results['star_add']=json.loads((config/'carousel.json').read_text())==[str(library/'1')]
  click(120,300);wait(lambda:active(1));assert state()['detail'];results['click_applies_without_button']=True
  time.sleep(.5);track=geometry()['position'][0];profiles=config/'wallpaper_profiles.json'
  first_process=json.loads((root/'state/wallpaper-lab/running.json').read_text())
  move(track[0]+track[2]/2,track[1]);run('ydotool','click','0x40',env=env);time.sleep(.25);(a.output/'drag-start.json').write_text(json.dumps(geometry(),indent=2));run('ydotool','mousemove','--',str(round(track[2]*.2)),'0',env=env);time.sleep(.4);(a.output/'drag-move.json').write_text(json.dumps(geometry(),indent=2));run('ydotool','click','0x80',env=env)
  wait(lambda:profiles.exists() and 0.05<json.loads(profiles.read_text())[str(library/'1')]['position']['x']<=1)
  assert json.loads((root/'state/wallpaper-lab/running.json').read_text())==first_process;results['position_drag_without_restart_or_apply']=True
  click(*geometry()['center']);wait(lambda:json.loads(profiles.read_text())[str(library/'1')]['position']['x']==0);results['center_resets_saved_position']=True
  click(*geometry()['header']['filter']);wait(lambda:geometry()['filter_width']>0)
  # Collapse the single-monitor accordion; dual remains expanded by default.
  (a.output/'before-filter.json').write_text(json.dumps(geometry(),indent=2))
  click(*geometry()['groups']['0']);time.sleep(.4)
  (a.output/'after-collapse.json').write_text(json.dumps(geometry(),indent=2))
  click(*geometry()['resolutions']['7']);wait(lambda:state()['results']==1)
  click(*geometry()['resolutions']['8']);wait(lambda:state()['results']==1);results['multiple_resolution_and_match']=True
  click(*geometry()['resolutions']['9']);wait(lambda:state()['results']==0);results['multiple_resolution_and_reject']=True
  click(110,235);wait(lambda:state()['results']==2)
  click(*geometry()['header']['filter']);wait(lambda:geometry()['filter_width']==0)
  old=geometry()['detail_width'];move(lambda w:w['size'][0]-old,400);run('ydotool','click','0x40',env=env);time.sleep(.15);run('ydotool','mousemove','--','-65','0',env=env);time.sleep(.3);run('ydotool','click','0x80',env=env);time.sleep(.5)
  assert geometry()['detail_width']>old+20;results['inspector_resize']=True
  g=geometry();n=1;x=20+(n%g['columns'])*(g['tile']+22)+80;y=184+(n//g['columns'])*(g['tile']+112)+90
  click(x,y);wait(lambda:active(2));results['second_selection_applied']=True
  click(lambda w:w['size'][0]-34,192);time.sleep(.5)
  click(44,208);assert state()['delete_confirmation'];assert (library/'1').exists();results['delete_second_check']=True
  click(lambda w:(w['size'][0]-560)/2+360,lambda w:(w['size'][1]-300)/2+258)
  wait(lambda:not (library/'1').exists());assert (root/'data/Trash/files/1/project.json').exists();results['recoverable_trash']=True
 finally:
  if ui and ui.poll() is None:
   w=window()
   if (config/'wallpaper_profiles.json').exists():shutil.copy2(config/'wallpaper_profiles.json',a.output/'last-profiles.json')
   if statefile.exists():shutil.copy2(statefile,a.output/'last-state.json')
   if pathlib.Path(str(statefile)+'.geometry').exists():shutil.copy2(pathlib.Path(str(statefile)+'.geometry'),a.output/'last-geometry.json')
   if w:run('grim','-g',f'{w["at"][0]},{w["at"][1]} {w["size"][0]}x{w["size"][1]}',a.output/'last.png')
   ui.terminate();ui.wait(timeout=8)
  run(app,'--stop',env=env);daemon.terminate();daemon.wait(timeout=5);socket.unlink(missing_ok=True)
  (a.output/'results.json').write_text(json.dumps(results,indent=2)+'\n')
assert before==json.loads(run(installed,'--status')),'personal renderer changed'
print(json.dumps(results,indent=2))

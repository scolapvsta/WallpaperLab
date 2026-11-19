"""Opt-in real-pointer SDDM catalog/editor test in isolated XDG directories."""
import argparse,json,os,signal,subprocess,time
from pathlib import Path
from PIL import Image,ImageChops,ImageStat
p=argparse.ArgumentParser();p.add_argument('--app',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);library=out/'library';library.mkdir(exist_ok=True)
socket=Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-sddm-library-{os.getpid()}.sock'
env=os.environ|dict(XDG_CONFIG_HOME=str(out/'config'),XDG_STATE_HOME=str(out/'state'),XDG_DATA_HOME=str(out/'data'),XDG_CACHE_HOME=str(out/'cache'),WALLPAPER_LAB_LIBRARY=str(library),WALLPAPER_LAB_UI_STATE=str(out/'ui.json'),WALLPAPER_LAB_UI_READ_ONLY='1',YDOTOOL_SOCKET=str(socket))
(config/'settings.json').write_text(json.dumps(dict(language='en',ui_design=2,ui_theme=3,theme_scoped_v3=True,caelestia_enabled=True)))
for kind,ext in [('image','png'),('video','mp4'),('gif','gif')]:
 project=library/kind;project.mkdir(exist_ok=True)
 command=['ffmpeg','-v','error','-y','-f','lavfi','-i','testsrc2=size=640x360:rate=12']
 command+=['-frames:v','1']if kind=='image'else['-t','2','-vf','scale=320:180']if kind=='gif'else['-t','2','-c:v','libx264','-pix_fmt','yuv420p']
 subprocess.run(command+['-threads','1',str(project/('media.'+ext))],check=True)
 (project/'project.json').write_text(json.dumps(dict(title='Fixture '+kind,type='video'if kind=='video'else'web',file='media.'+ext,lab_origin='local',lab_media_file='media.'+ext)))
scene=library/'scene';scene.mkdir(exist_ok=True);(scene/'project.json').write_text(json.dumps(dict(title='Unsupported scene',type='scene',file='scene.pkg',preview='preview.png')));(scene/'preview.png').write_bytes((library/'image/media.png').read_bytes())
log=(out/'launcher.log').open('w');ui=None;daemon=None;results={}
def run(*args):return subprocess.check_output([str(a)for a in args],env=env,text=True)
def cli(*args):return json.loads(run(app,'--sddm',*args))
def g():return json.loads((out/'ui.json.geometry').read_text())
def style():return json.loads((out/'ui.json.style').read_text())
def prefs():return json.loads((config/'sddm.json').read_text())
def window():return next(w for w in json.loads(run('hyprctl','clients','-j'))if w['pid']==ui.pid)
def wait(fn,timeout=35):
 deadline=time.monotonic()+timeout
 while time.monotonic()<deadline:
  assert ui is None or ui.poll()is None,('UI exited',ui.returncode,(out/'launcher.log').read_text()[-2000:])
  try:
   value=fn()
   if value:return value
  except(OSError,ValueError,KeyError,StopIteration):pass
  time.sleep(.1)
 raise AssertionError('UI condition timed out')
def move(point):
 # Let focus/raise animations finish before computing screen coordinates.
 # Focus can warp the cursor; verify both focus and position before clicking.
 for attempt in range(4):
  run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.2)
  w=window();x,y=round(w['at'][0]+point[0]),round(w['at'][1]+point[1]);run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={x},y={y},relative=false}}))');time.sleep(.15)
  cursor=json.loads(run('hyprctl','cursorpos','-j'))
  if json.loads(run('hyprctl','activewindow','-j')).get('pid')==ui.pid and abs(cursor['x']-x)<2 and abs(cursor['y']-y)<2:return x,y
 raise AssertionError('Test window could not retain focus; no click sent')
def click(point):move(point);run('ydotool','click','0xC0');time.sleep(.25)
def card(index):return next(row for row in g()['sddm_cards']if row['index']==index)
def pick(index):
 r=wait(lambda:card(index))['rect'];click((r[0]+r[2]/2,r[1]+r[3]/2));wait(lambda:not style()['sddm_busy'])
def shot(name):
 time.sleep(.35);w=window();x,y=w['at'];width,height=w['size'];run('grim','-g',f'{x},{y} {width}x{height}',out/(name+'.png'))
def crop_preview(name):
 shot(name);r=g()['sddm_preview_rect'];return Image.open(out/(name+'.png')).convert('RGB').crop((round(r[0]),round(r[1]),round(r[0]+r[2]),round(r[1]+r[3])))
def stop(process):
 if process and process.poll()is None:process.terminate();process.wait(timeout=25)
try:
 state=cli('enable');assert state['ready']
 image_index=next(i for i,row in enumerate(state['backgrounds'])if row['id']==str(library/'image'))
 scene_index=next(i for i,row in enumerate(state['backgrounds'])if row['id']==str(scene))
 daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log);wait(socket.exists);time.sleep(1.5)
 ui=subprocess.Popen([str(app),'--sddm-settings'],env=env,stdout=log,stderr=log);wait(window);wait(lambda:style()['sddm_ready'])
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.float({{action="on",window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x=1400,y=940,relative=false}}));hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}}))');time.sleep(.8)
 click(g()['sddm_controls']['filter']);click(g()['sddm_origins'][2]);wait(lambda:g()['sddm_filtered']==3)
 pick(image_index);wait(lambda:prefs()['wallpaper']['source']==str(library/'image'))
 thumbnail=state['backgrounds'][image_index]['thumbnail'];wait(lambda:Path(thumbnail).is_file());time.sleep(.4)
 original=crop_preview('image-editor-before');before=prefs();track=g()['number_tracks']['140'];move((track[0]+track[2]*.7,track[1]+1));run('ydotool','click','0x40');time.sleep(.15);run('ydotool','click','0x80');wait(lambda:g()['sddm_dirty'])
 edited=crop_preview('image-editor-draft');assert sum(ImageStat.Stat(ImageChops.difference(original,edited)).mean)>8
 assert prefs()==before
 click(g()['sddm_controls']['save']);wait(lambda:not style()['sddm_busy']and not g()['sddm_dirty']);assert .3<prefs()['wallpaper']['profiles'][str(library/'image')]['brightness']<.5
 results['draft_preview_and_validated_save']=True
 click(g()['sddm_controls']['close']);click(card(image_index)['star']);wait(lambda:not style()['sddm_busy']);click(g()['sddm_controls']['favorites']);wait(lambda:g()['sddm_filtered']==1);shot('favorites');click(g()['sddm_controls']['favorites'])
 click(g()['sddm_origins'][3]);wait(lambda:g()['sddm_filtered']==1);before=prefs();pick(scene_index);assert prefs()==before;shot('unsupported-scene');results['unsupported_scene_remains_catalog_only']=True
 click(g()['sddm_tabs'][1]);wait(lambda:g()['sddm_view']==1);pick(1);wait(lambda:prefs()['ui']['preset']=='black_hole');assert prefs()['wallpaper']==before['wallpaper']
 state=cli('status');wait(lambda:Path(state['ui_previews'][1]['thumbnail']).is_file());shot('login-presets')
 # UI position changes preserve the entire wallpaper profile.
 click(g()['choices']['91']['control']);wait(lambda:g()['choice_open']==91);click(g()['choices']['91']['options'][1]);wait(lambda:not style()['sddm_busy']);assert prefs()['ui']['position']=='left';assert prefs()['wallpaper']==before['wallpaper']
 click(g()['sddm_tabs'][2]);wait(lambda:g()['sddm_view']==2);state=cli('status');wait(lambda:Path(state['final_preview']).is_file());time.sleep(.5);shot('combined-preview');results['real_preset_and_combined_previews']=True
 # Live combined preview is the real greeter, owned by the application helper.
 click(g()['sddm_controls']['preview'])
 def greeter():
  for row in json.loads(run('hyprctl','clients','-j')):
   proc=Path('/proc',str(row['pid']))
   try:
    if proc.joinpath('exe').resolve().name=='sddm-greeter-qt6'and str(out/'state').encode()in proc.joinpath('cmdline').read_bytes():return row
   except OSError:pass
 view=wait(greeter);proc=Path('/proc',str(view['pid']));start=proc.joinpath('stat').read_text().rsplit(')',1)[1].split()[19];exe=proc.joinpath('exe').resolve()
 time.sleep(1);fd=os.pidfd_open(view['pid'])
 try:
  assert proc.joinpath('exe').resolve()==exe and proc.joinpath('stat').read_text().rsplit(')',1)[1].split()[19]==start
  signal.pidfd_send_signal(fd,signal.SIGTERM)
 finally:os.close(fd)
 wait(lambda:not style()['sddm_busy']);assert style()['sddm_ok'];results['combined_live_greeter']=True
 # Return to the saved editor at a half-screen width.
 click(g()['sddm_tabs'][0]);click(g()['sddm_origins'][2]);wait(lambda:g()['sddm_filtered']==3);pick(image_index)
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x=1264,y=900,relative=false}}));hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}}))');time.sleep(.8)
 assert g()['header_decoration'][1]+g()['header_decoration'][3]<89;shot('half-screen-editor');results['filters_favorites_and_half_screen']=True
 assert not (config/'wallpaper_profiles.json').exists();assert not (out/'state/wallpaper-lab/running.json').exists();results['desktop_profiles_and_renderer_untouched']=True
 assert 'Invalid UTF-8' not in (out/'launcher.log').read_text()
 (out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
finally:stop(ui);stop(daemon);log.close();socket.unlink(missing_ok=True)

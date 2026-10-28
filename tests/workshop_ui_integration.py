#!/usr/bin/env python3
"""Opt-in real Steam + desktop demo checks. --download-id authorizes a real subscription."""
import argparse,json,pathlib,subprocess,os,time,signal
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--download-id');a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
statepath=out/'ui.json';socket=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-workshop-{os.getpid()}.sock';env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'WALLPAPER_LAB_UI_STATE':str(statepath),'YDOTOOL_SOCKET':str(socket),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','deps','lib'])}
log=(out/'ui.log').open('w');daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log);ui=None;results={}
def run(*args,**kwargs):return subprocess.check_output([str(x)for x in args],env=env,text=True,**kwargs)
def wait(fn,timeout=30):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  try:value=fn()
  except(FileNotFoundError,json.JSONDecodeError,StopIteration):value=None
  if value:return value
  if ui and ui.poll()is not None:raise AssertionError(('UI exited',ui.returncode,(out/'ui.log').read_text()[-2000:]))
  time.sleep(.1)
 raise AssertionError('Condition timed out')
def window():return next(w for w in json.loads(run('hyprctl','clients','-j'))if w['pid']==ui.pid)
def state():return json.loads(statepath.read_text())
def geometry():return json.loads(pathlib.Path(str(statepath)+'.geometry').read_text())
def style():return json.loads(pathlib.Path(str(statepath)+'.style').read_text())
def focus():run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.12)
def move(x,y):
 w=window();run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={int(w["at"][0]+x)},y={int(w["at"][1]+y)}}}))');time.sleep(.05)
def click(x,y):focus();move(x,y);run('ydotool','click','0xC0');time.sleep(.25)
def keys(*seq):focus();run('ydotool','key',*seq);time.sleep(.16)
def paste(text):subprocess.run(['wl-copy','--',text],env=env,check=True);keys('29:1','47:1','47:0','29:0')
def search(text):keys('29:1','38:1','38:0','29:0');paste(text);keys('28:1','28:0')
def shot(name):
 focus();w=window();run('grim','-g',f'{w["at"][0]},{w["at"][1]} {w["size"][0]}x{w["size"][1]}',out/(name+'.png'))
try:
 wait(socket.exists);run('ydotool','click','0x80');ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log);wait(window);wait(lambda:'search_focus'in style());
 m=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']=='DP-6');run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}))');focus();time.sleep(.8)
 initial=state()['results'];keys('29:1','38:1','38:0','29:0');wait(lambda:style()['search_focus']);value='Frière 🦊 — clipboard';paste(value);wait(lambda:style()['search_count']==len(value.encode()));assert state()['results']==0
 keys('29:1','30:1','30:0','46:1','46:0','29:0');copied=run('wl-paste','--no-newline',timeout=4);assert copied==value,(copied,value)
 keys('29:1','45:1','45:0','29:0');wait(lambda:style()['search_count']==0);assert state()['results']==initial
 keys('29:1','47:1','47:0','29:0');wait(lambda:style()['search_count']==len(value.encode()));results['ctrl_l_utf8_select_copy_cut_paste']=True
 keys('29:1','30:1','30:0','14:1','14:0','29:0');keys('1:1','1:0');click(*geometry()['header']['filter']);wait(lambda:geometry()['filter_width']>200);width=geometry()['filter_width'];move(width,400);run('ydotool','click','0x40');run('ydotool','mousemove','--','125','0');time.sleep(.3);run('ydotool','click','0x80');wait(lambda:geometry()['filter_width']>width+80);results['filter_resize_persisted']=json.loads((out/'config/wallpaper-lab/settings.json').read_text())['filter_width'];shot('library-filters-resized')
 click(*geometry()['header']['settings']);wait(lambda:state()['settings']);shot('settings-display');click(*geometry()['settings_tabs'][3]);time.sleep(.3);shot('settings-appearance');click(*geometry()['font_control']);time.sleep(.4);shot('font-dropdown');keys('1:1','1:0');keys('1:1','1:0');wait(lambda:not state()['settings']);results['compact_categorized_settings']=True
 click(*geometry()['header']['workshop']);wait(lambda:state()['page']==3);wait(lambda:geometry()['workshop']['count']>0 and not geometry()['workshop']['busy'],45);time.sleep(2);shot('workshop-browser');assert geometry()['workshop']['count']==50
 search('Frieren');wait(lambda:not geometry()['workshop']['busy'],40);wait(lambda:geometry()['workshop']['revision']>=2);assert geometry()['workshop']['count']>0;shot('workshop-search');results['steam_search_and_metadata']=True
 # Single ID is deterministic and keeps the test's download small.
 if a.download_id:
  revision=geometry()['workshop']['revision'];search(a.download_id);wait(lambda:geometry()['workshop']['revision']>revision,40);assert geometry()['workshop']['count']==1;click(*geometry()['workshop']['cards'][0]);wait(lambda:geometry()['workshop']['selected']==0);time.sleep(.8);shot('workshop-details');click(*geometry()['workshop']['download']);
  roots=[pathlib.Path.home()/'.local/share/Steam/steamapps/workshop/content/431960',pathlib.Path.home()/'.steam/steam/steamapps/workshop/content/431960'];project=wait(lambda:next((r/a.download_id/'project.json'for r in roots if(r/a.download_id/'project.json').is_file()),None),90);wait(lambda:not style()['scanning']);time.sleep(2);shot('workshop-downloaded');results['one_click_steam_download']=str(project);results['downloaded_project']=json.loads(project.read_text()).get('title')
 results['ui_fps']=style()['fps']
finally:
 try:run('ydotool','click','0x80')
 except Exception:pass
 if ui and ui.poll()is None:
  try:shot('last')
  except Exception:pass
  ui.terminate();ui.wait(12)
 daemon.terminate();daemon.wait(5);socket.unlink(missing_ok=True);log.close();(out/'results.json').write_text(json.dumps(results,indent=2))
print(json.dumps(results,indent=2))

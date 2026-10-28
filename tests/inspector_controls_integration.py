#!/usr/bin/env python3
"""Real pointer tests for isolated inspector and the dedicated display popup."""
import argparse,json,os,pathlib,subprocess,time,shutil
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
root=out/'library/1';root.mkdir(parents=True,exist_ok=True)
schema={'color':{'type':'color','text':'Wallpaper color','value':'0 0 0','order':0},'speed':{'type':'slider','text':'Speed','value':.5,'min':0,'max':1,'order':1},'enabled':{'type':'bool','text':'Motion','value':True,'order':2}}
(root/'project.json').write_text(json.dumps({'title':'Inspector controls','type':'web','file':'index.html','general':{'properties':schema}}));(root/'index.html').write_text('<!doctype html><style>html,body{margin:0;width:100%;height:100%;background:linear-gradient(45deg,#245183,#d9bb78)}</style>')
statepath=out/'ui.json';socket=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-inspector-{os.getpid()}.sock';env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'WALLPAPER_LAB_LIBRARY':str(out/'library'),'WALLPAPER_LAB_UI_STATE':str(statepath),'YDOTOOL_SOCKET':str(socket),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','deps','lib'])}
def run(*args):return subprocess.check_output([str(s)for s in args],env=env,text=True)
def wait(fn,timeout=20):
 until=time.monotonic()+timeout
 while time.monotonic()<until:
  try:v=fn()
  except(FileNotFoundError,json.JSONDecodeError,StopIteration):v=None
  if v:return v
  time.sleep(.08)
 raise AssertionError('condition timed out')
log=(out/'ui.log').open('w');daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log);ui=None;result={}
try:
 wait(socket.exists);run('ydotool','click','0x80');time.sleep(1);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log)
 def window():return next(w for w in json.loads(run('hyprctl','clients','-j'))if w['pid']==ui.pid)
 wait(window)
 def state():return json.loads(statepath.read_text())
 def style():return json.loads(pathlib.Path(str(statepath)+'.style').read_text())
 def geometry():return json.loads(pathlib.Path(str(statepath)+'.geometry').read_text())
 wait(lambda:'zoom_track'in geometry());time.sleep(1)
 monitors=json.loads(run('hyprctl','monitors','-j'));target=next((m for m in monitors if m['name']=='DP-6'),monitors[-1]);run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{target["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.5)
 def move(x,y):
  w=window();run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={int(w["at"][0]+x)},y={int(w["at"][1]+y)}}}))');time.sleep(.06)
 def click(x,y):run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.2);move(x,y);run('ydotool','click','0xC0');time.sleep(.22)
 def key(*keys):run('ydotool','key',*keys);time.sleep(.18)
 def visible(key):
  for _ in range(25):
   point=geometry()[key];w=window();y=point[1];bottom=y+(point[3]if key=='color_box'else 0)
   if not state()['detail']:click(120,300);time.sleep(.8);continue
   if 240<y and bottom<w['size'][1]-185 and point[0]<w['size'][0]-15:return point
   move(w['size'][0]-150,w['size'][1]//2);run('ydotool','mousemove','--wheel','--','0','-3'if bottom>w['size'][1]-185 else'3');time.sleep(.35)
  raise AssertionError(('offscreen',key,point))
 def shot(name):
  w=window();run('grim','-g',f'{w["at"][0]},{w["at"][1]} {w["size"][0]}x{w["size"][1]}',out/(name+'.png'))
 click(120,300);wait(lambda:state()['detail']);running=out/'state/wallpaper-lab/running.json';wait(running.exists);time.sleep(1)
 click(*visible('color_swatch'));wait(lambda:geometry()['color_box'][2]>20);time.sleep(.2)
 visible('color_box');hue=geometry()['color_hue'];click(hue[0]+hue[2]/2,hue[1]+hue[3]*.6);wait(lambda:not style()['applying']);box=visible('color_box');click(box[0]+box[2]*.8,box[1]+box[3]*.2);profiles=out/'config/wallpaper-lab/wallpaper_profiles.json'
 wait(lambda:profiles.exists()and'properties'in json.loads(profiles.read_text()).get(str(root),{}));color=json.loads(profiles.read_text())[str(root)]['properties']['color'];assert color!='0 0 0',color;channels=list(map(float,color.split()));assert channels[2]>channels[0] and channels[2]>channels[1],color;result['graphic_color_saved']=color;shot('color-picker')
 track=visible('zoom_track');x=track[0]+track[2]*.45;move(track[0]+track[2]*.1,track[1]);run('ydotool','click','0x40');run('ydotool','mousemove','--',str(round(track[2]*.35)),'0');time.sleep(.4)
 live=out/'state/wallpaper-lab/placement-live.json';wait(live.exists);livezoom=json.loads(live.read_text())['position']['zoom'];assert 1.7<livezoom<2.1,livezoom
 run('ydotool','click','0x80');wait(lambda:json.loads(profiles.read_text())[str(root)]['position']['zoom']>1.7);result['zoom_live_and_persistent']=True;shot('zoom-slider')
 click(*visible('creator_reset'));wait(lambda:not json.loads(profiles.read_text())[str(root)].get('properties'));assert json.loads(profiles.read_text())[str(root)]['position']['zoom']>1.7;result['creator_reset_preserves_position']=True
 wait(lambda:not style()['applying']);click(*geometry()['display_control']);wait(lambda:style()['display_popup']);assert not state()['settings'];shot('display-popup');click(*geometry()['display_save']);wait(lambda:'display_layout'in json.loads((out/'config/wallpaper-lab/settings.json').read_text()));saved_layout=json.loads((out/'config/wallpaper-lab/settings.json').read_text())['display_layout'];key('1:1','1:0');wait(lambda:not style()['display_popup'])
 click(*geometry()['header']['settings']);wait(lambda:state()['settings']);click(*geometry()['settings_tabs'][0]);time.sleep(.2);assert json.loads((out/'config/wallpaper-lab/settings.json').read_text())['display_layout']==saved_layout;shot('display-in-settings');key('1:1','1:0');result['display_popup_and_settings_synced']=True
 click(*geometry()['header']['carousel']);wait(lambda:state()['page']==1);wait(lambda:not style()['applying']);click(*geometry()['display_control']);wait(lambda:style()['display_popup']);assert not state()['settings'];key('1:1','1:0');result['dedicated_display_popup_both_pages']=True
 click(*geometry()['header']['settings']);wait(lambda:state()['settings']);click(*geometry()['settings_tabs'][3]);time.sleep(.4)
 for _ in range(20):
  point=geometry()['font_control'];w=window()
  if 250<point[1]<w['size'][1]-185:break
  move(w['size'][0]//2,w['size'][1]//2);run('ydotool','mousemove','--wheel','--','0','-3'if point[1]>w['size'][1]-185 else'3');time.sleep(.3)
 click(*geometry()['font_control']);wait(lambda:style()['font_menu']);move(window()['size'][0]//2,window()['size'][1]//2);run('ydotool','mousemove','--wheel','--','0','-7');time.sleep(.5);shot('font-previews');result['font_preview_rendered']=True

finally:
 try:run('ydotool','click','0x80')
 except Exception:pass
 if ui and ui.poll()is None:
  try:shot('last')
  except Exception:pass
  ui.terminate();ui.wait(10)
 subprocess.run([str(app),'--stop'],env=env,stdout=log,stderr=log);daemon.terminate();daemon.wait(5);socket.unlink(missing_ok=True);log.close();(out/'results.json').write_text(json.dumps(result,indent=2))
print(json.dumps(result,indent=2))

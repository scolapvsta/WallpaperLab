#!/usr/bin/env python3
"""Explicit desktop check for editable awww timing fields and numeric steppers."""
import argparse,json,os,pathlib,shutil,subprocess,time
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args()
if not a.desktop:p.error('--desktop required: opens a test window and injects pointer/keyboard input')
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);library=out/'library';library.mkdir(exist_ok=True)
(config/'settings.json').write_text(json.dumps({'wallpaper_backend':1,'language':'en','awww_transition':12,'awww_step':2,'awww_duration':2000,'caelestia_enabled':False}))
state=out/'ui.json';sock=out/'input.sock';sock.unlink(missing_ok=True);env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'XDG_DATA_HOME':str(out/'data'),'WALLPAPER_LAB_LIBRARY':str(library),'WALLPAPER_LAB_UI_STATE':str(state),'WALLPAPER_LAB_UI_READ_ONLY':'1','YDOTOOL_SOCKET':str(sock)}
ydotool=shutil.which('ydotool');ydotoold=shutil.which('ydotoold');assert ydotool and ydotoold
log=(out/'ui.log').open('w');daemon=subprocess.Popen([ydotoold,'--socket-path='+str(sock)],stdout=log,stderr=log);ui=None

def run(*args):return subprocess.check_output(list(map(str,args)),env=env,text=True,stderr=log)
def wait(fn):
 for n in range(200):
  try:
   result=fn()
   if result:return result
  except (FileNotFoundError,json.JSONDecodeError):pass
  time.sleep(.06)
 raise AssertionError('GUI condition timed out')
def read(suffix=''):return json.loads(pathlib.Path(str(state)+suffix).read_text())
def geometry():return read('.geometry')
def prefs():return json.loads((config/'settings.json').read_text())
def window():return next((w for w in json.loads(run('hyprctl','clients','-j')) if w['pid']==ui.pid),None)
def click(point):
 w=window();run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}})); hl.dispatch(hl.dsp.cursor.move({{x={w["at"][0]+round(point[0])},y={w["at"][1]+round(point[1])}}}))');time.sleep(.12);run(ydotool,'click','0xC0');time.sleep(.16)
def key(code):run(ydotool,'key',f'{code}:1',f'{code}:0');time.sleep(.12)
def field(id,part='value'):return geometry()['number_fields'][str(id)][part]
def edit(id,text,commit=True):
 click(field(id));run(ydotool,'type','--',text);time.sleep(.12)
 if commit:key(28)
def value(keyname,expected):wait(lambda:prefs().get(keyname)==expected)
def screenshot(name):
 w=window();run('grim','-g',f'{w["at"][0]},{w["at"][1]} {w["size"][0]}x{w["size"][1]}',out/name)
try:
 wait(sock.exists);time.sleep(2);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log);wait(window)
 run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.float({{action="on",window="pid:{ui.pid}"}})); hl.dispatch(hl.dsp.window.resize({{x=1440,y=900,relative=false,window="pid:{ui.pid}"}})); hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}}))')
 wait(lambda:geometry().get('header'));time.sleep(1)
 click(geometry()['header']['settings']);wait(lambda:read()['settings']);click(geometry()['settings_tabs'][2]);wait(lambda:read('.style')['settings_tab']==2)
 wait(lambda:'130' in geometry()['number_fields'])
 edit(129,'37');value('awww_step',37);click(field(129,'up'));value('awww_step',38);click(field(129,'down'));value('awww_step',37)
 edit(130,'2.25');value('awww_duration',2250);click(field(130,'up'));value('awww_duration',3250);click(field(130,'down'));value('awww_duration',2250)
 edit(129,'200');value('awww_step',100);click(field(129,'up'));value('awww_step',100)
 edit(130,'15');value('awww_duration',10000);click(field(130,'down'));value('awww_duration',9000);click(field(130,'up'));value('awww_duration',10000)
 edit(129,'nan');value('awww_step',100);click(field(129));key(14);key(28);value('awww_step',100)
 edit(129,'42',False);key(1);value('awww_step',100);assert read()['settings']
 edit(129,'42',False);click(field(130));value('awww_step',42);run(ydotool,'type','--','0.5');key(28);value('awww_duration',500)
 click(field(130,'down'));value('awww_duration',100);click(field(130,'up'));value('awww_duration',1100)
 click(field(129));key(103);key(28);value('awww_step',43)
 for id,keyname,lo,hi in [(129,'awww_step',1,100),(130,'awww_duration',100,10000)]:
  x,y,width=geometry()['number_tracks'][str(id)];click((x+width,y));value(keyname,hi);click((x,y));value(keyname,lo)
 edit(129,'42');edit(130,'2.5');value('awww_duration',2500);screenshot('inline-numbers.png')
 click(geometry()['animation_controls'][3]);wait(lambda:read('.style')['customize']==2);wait(lambda:'125' in geometry()['number_fields'])
 edit(126,'19');value('awww_step',19);edit(125,'3.75');value('awww_duration',3750);click(field(125,'up'));value('awww_duration',4750)
 edit(125,'9',False);key(1);value('awww_duration',4750);assert read('.style')['customize']==2
 screenshot('modal-numbers.png')
 result={'manual_integer_and_decimal_input':True,'unit_step_buttons':True,'bounds_and_disabled_max':True,'invalid_empty_input_preserved':True,'escape_cancels_field_only':True,'click_outside_commits':True,'keyboard_up_step':True,'sliders_reach_caps':True,'inline_and_modal_controls':True}
 (out/'results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
finally:
 if ui:
  ui.terminate()
  try:ui.wait(timeout=10)
  except subprocess.TimeoutExpired:ui.kill();ui.wait()
 daemon.terminate();daemon.wait();sock.unlink(missing_ok=True);log.close()

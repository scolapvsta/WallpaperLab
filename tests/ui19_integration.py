#!/usr/bin/env python3
"""Native UI: binary settings toggle directly and preserve typed creator values."""
import argparse,json,pathlib,subprocess,os,time,shlex,shutil
from PIL import Image,ImageChops,ImageStat
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);source=out/'originals';source.mkdir(exist_ok=True);bin_=out/'bin';bin_.mkdir(exist_ok=True);sock=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-ui19-{os.getpid()}.sock'
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
  run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={target[0]},y={target[1]}}}))');time.sleep(.12)
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




personal=json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));env['WALLPAPER_LAB_UI_READ_ONLY']='1';fixture=out/'library/1000';fixture.mkdir(parents=True,exist_ok=True);env['WALLPAPER_LAB_LIBRARY']=str(fixture.parent)
schema={'enabled':{'type':'bool','text':'Creator enabled','value':False,'order':0},'pair':{'type':'combo','text':'Two choices','value':'a','options':[{'label':'First option','value':'a'},{'label':'Second option','value':'b'}],'order':1},'many':{'type':'combo','text':'Three choices','value':7,'options':[{'label':'One','value':2},{'label':'Two','value':7},{'label':'Three','value':9}],'order':2},'resolution':{'type':'combo','text':'Resolution','value':'hd','options':[{'label':'1920 x 1080','value':'hd'}],'order':3}}
(fixture/'project.json').write_text(json.dumps({'type':'web','title':'Binary controls','file':'index.html','preview':'preview.png','general':{'properties':schema}}));(fixture/'index.html').write_text('<style>html,body{margin:0;width:100%;height:100%;background:#28486c}</style>');Image.new('RGB',(640,360),(40,72,108)).save(fixture/'preview.png');(config/'settings.json').write_text(json.dumps({'mode':'single','screen':0,'fps':30,'muted':True,'fullscreen_policy':0,'theme_catalog':True,'ui_theme':0,'carousel_layout':1}));profiles=config/'wallpaper_profiles.json'
def pref():return json.loads((config/'settings.json').read_text())
def profile():return json.loads(profiles.read_text())[str(fixture)]
def visible(getter):
 for _ in range(26):
  point=getter();height=w()['size'][1]
  if 248<point[1]<height-220:time.sleep(.45);return getter()
  move(w()['size'][0]-140,500);run('ydotool','mousemove','--wheel','--','0','-3'if point[1]>=height-220 else'3');time.sleep(.3)
 raise AssertionError(('control not visible',getter()))
def toggle(id):
 control=g()['choices'][str(id)];assert control['kind']=='toggle',id;old=control['value'];click(*control['control']);wait(lambda:g()['choices'][str(id)]['value']!=old);assert g()['choice_open']==-1
try:
 wait(sock.exists);time.sleep(1);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log);wait(w);wait(lambda:'library_cards'in g());m=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']=='DP-6');run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.5);card=g()['library_cards'][0];click(card[0]+card[2]/2,card[1]+card[3]/2);wait(lambda:state()['detail'])
 visible(lambda:g()['choices']['70']['control']);toggle(70);wait(lambda:profile().get('variant',{}).get('value')=='hd');toggle(70);wait(lambda:'variant'not in profile());results['automatic_single_variant_is_a_toggle']=True
 # Collapse image controls, then exercise all five local playback switches.
 click(*visible(lambda:g()['inspector_sections'][1]));click(*visible(lambda:g()['inspector_sections'][2]));visible(lambda:g()['choices']['64']['control'])
 for id,key in enumerate(['audio','reactive','mouse','parallax','particles'],64):visible(lambda:g()['choices'][str(id)]['control']);toggle(id);assert profile()['playback'][key]is False
 results['five_preview_playback_toggles_save']=True;click(*visible(lambda:g()['inspector_sections'][2]));click(*visible(lambda:g()['inspector_sections'][3]));visible(lambda:g()['creator_binary'][0]['control']);assert len(g()['creator_binary'])==2
 click(*visible(lambda:g()['creator_binary'][0]['control']));wait(lambda:profile()['properties'].get('enabled')is True)
 click(*visible(lambda:g()['creator_binary'][1]['control']));wait(lambda:profile()['properties'].get('pair')=='b');assert not profile()['properties'].get('many');shot('creator-pair-second')
 click(*g()['creator_binary'][1]['control']);wait(lambda:profile()['properties']['pair']=='a');results['creator_bool_and_pair_preserve_typed_values']=True
 click(*g()['header']['settings']);wait(lambda:state()['settings']);click(*g()['settings_tabs'][4]);toggle(41);assert pref()['reduce_motion']is True;toggle(42);assert pref()['filter_any']==1
 click(*g()['settings_tabs'][1]);assert g()['choices']['10']['kind']=='dropdown'
 for id,key in enumerate(['muted','disable_audio_processing','disable_mouse','disable_parallax','disable_particles'],11):
  before=pref().get(key,True if key in['muted','disable_mouse','disable_parallax']else False);toggle(id);assert pref()[key]!=before
 shot('global-playback-toggles');results['global_boolean_settings_are_toggles']=True
 click(*g()['settings_tabs'][2]);assert g()['choices']['21']['kind']=='toggle';click(*g()['animation_controls'][2]);wait(lambda:s()['customize']==1);toggle(75);assert pref()['switcher_reverse']==1;esc();wait(lambda:s()['customize']==0);results['shortcut_and_switcher_binary_controls']=True
 click(*g()['settings_tabs'][0]);visible_point=g()['choices']['2']['control'];toggle(2);toggle(2);assert g()['choices']['1']['kind']=='dropdown';results['display_basis_has_named_two_state_switch']=True
 click(*g()['settings_tabs'][5]);toggle(50);assert pref()['workshop_mature']is True
 click(*g()['settings_tabs'][7]);assert g()['choices']['73']['kind']=='toggle';click(*g()['settings_tabs'][3]);assert g()['choices']['38']['kind']=='dropdown';shot('appearance-multiple-choices-retained');esc();wait(lambda:not state()['settings'])
 click(*g()['header']['filter']);wait(lambda:'54'in g()['choices']);toggle(54);assert pref()['filter_any']==0;results['library_logic_switch_synchronized_with_settings']=True
 click(*g()['header']['workshop']);wait(lambda:state()['page']==3);wait(lambda:'59'in g()['choices']);toggle(59);assert g()['choices']['60']['kind']=='dropdown';shot('workshop-logic-toggle');results['workshop_logic_uses_same_toggle']=True
finally:
 if __import__('sys').exc_info()[0]and ui and ui.poll()is None:
  try:shot('failure')
  except Exception:pass
 try:run('ydotool','click','0x80')
 except Exception:pass
 if ui and ui.poll()is None:ui.terminate();ui.wait(15)
 run(app,'--stop');daemon.terminate();daemon.wait(5);sock.unlink(missing_ok=True);(out/'results.json').write_text(json.dumps(results,indent=2))
assert personal==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));print(json.dumps(results,indent=2))

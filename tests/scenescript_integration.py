#!/usr/bin/env python3
"""Opt-in SceneScript lifecycle/input test on an isolated native renderer."""
import argparse,json,os,pathlib,signal,subprocess,time
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--assets',required=True);p.add_argument('--screen',default='DP-6');p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args()
if not a.desktop:p.error('--desktop is required')
out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);fixture=out/'fixture';fixture.mkdir(exist_ok=True)
def run(*args):return subprocess.check_output(args,text=True)
def identity(pid):
 d=pathlib.Path('/proc')/str(pid);st=(d/'exe').stat();return (st.st_dev,st.st_ino),(d/'stat').read_text().rsplit(')',1)[1].split()[19]
project={'type':'scene','title':'SceneScript regression','file':'scene.json','general':{'properties':{'enabled':{'type':'bool','value':True},'factor':{'type':'slider','value':.5}}}}
(fixture/'project.json').write_text(json.dumps(project))
# Synthetic effect assets; no Workshop content is required by this regression.
for d in ('effects/opacity','materials/effects','shaders/effects'):(fixture/d).mkdir(parents=True,exist_ok=True)
(fixture/'effects/opacity/effect.json').write_text(json.dumps({'passes':[{'material':'materials/effects/opacity.json'}]}))
(fixture/'materials/effects/opacity.json').write_text(json.dumps({'passes':[{'shader':'effects/lab_opacity','blending':'normal','depthtest':'disabled','depthwrite':'disabled','cullmode':'nocull'}]}))
(fixture/'shaders/effects/lab_opacity.vert').write_text('uniform mat4 g_ModelViewProjectionMatrix; attribute vec3 a_Position; attribute vec2 a_TexCoord; varying vec2 v_TexCoord; void main(){v_TexCoord=a_TexCoord;gl_Position=g_ModelViewProjectionMatrix*vec4(a_Position,1.0);}')
(fixture/'shaders/effects/lab_opacity.frag').write_text('uniform sampler2D g_Texture0; varying vec2 v_TexCoord; void main(){gl_FragColor=texSample2D(g_Texture0,v_TexCoord)*0.0;}')

script="""export var scriptProperties=createScriptProperties().addSlider({name:'factor',value:1}).finish();
let initialized=false,configured=false,frames=0,moves=0;
export function applyUserProperties(p){configured=p.enabled===true;}
export function init(value){initialized=true;return value;}
export function cursorMove(event){moves++;let p=input.cursorWorldPosition;thisScene.getLayer('target').origin={x:p.x,y:p.y,z:0};}
export function update(value){
 let target=thisScene.getLayer('target'),effect=target.getEffect('hide');
 if(!initialized||!configured||scriptProperties.factor!==.5)throw Error('lifecycle/properties');
 if(frames===10){target.visible=false;if(target.visible!==false)throw Error('visible setter');target.visible=true;effect.visible=true;}
 if(frames===20){if(!effect.visible)throw Error('effect setter');effect.visible=false;}
 if(frames===30){target.scale={x:.5,y:.5,z:1};if(target.scale.x!==.5)throw Error('vector setter');target.angles={x:0,y:0,z:90};if(Math.abs(target.angles.z-90)>.001)throw Error('angle units '+target.angles.z);}
 if(frames++%30===0)console.error('SCRIPT_PROBE:'+JSON.stringify({frames,moves,x:input.cursorWorldPosition.x,y:input.cursorWorldPosition.y,tx:target.origin.x,ty:target.origin.y,initialized,configured}));
 return value;
}"""
scene={'camera':{'center':'0 0 0','eye':'0 0 1','up':'0 1 0'},'general':{'orthogonalprojection':{'width':640,'height':360},'clearcolor':'0 0 0'},'objects':[
 {'id':1,'name':'target','image':'models/util/solidlayer.json','origin':'320 180 0','size':'80 80','color':'1 0 0','effects':[{'id':1,'name':'hide','file':'effects/opacity/effect.json','visible':False,'passes':[{'id':0,'constantshadervalues':{'alpha':0}}]}]},
 {'id':2,'name':'input','image':'models/util/solidlayer.json','origin':'320 180 0','size':'640 360','alpha':0,'visible':{'value':True,'script':script,'scriptproperties':{'factor':{'user':'factor','value':1}}}}
]}
(fixture/'scene.json').write_text(json.dumps(scene));log=out/'renderer.log';ready=out/'ready';ready.unlink(missing_ok=True)
old_cursor=json.loads(run('hyprctl','cursorpos','-j'));status=run(str(pathlib.Path.home()/'.local/bin/wallpaper-lab'),'--status')
monitor=next(m for m in json.loads(run('hyprctl','monitors','-j')) if m['name']==a.screen)
env=os.environ.copy();env['XDG_STATE_HOME']=str(out/'state');env['XDG_CONFIG_HOME']=str(out/'config')
app=a.app.resolve();record=None
with log.open('w') as f:
 child=subprocess.Popen([str(app),'--render','--ready-file',str(ready),'--assets-dir',a.assets,'--fps','30','--volume','0','--no-fullscreen-pause','--scaling','fill','--screen-root',a.screen,str(fixture)],env=env,stdout=f,stderr=f,start_new_session=True)
 try:
  record=identity(child.pid);deadline=time.monotonic()+20
  while not ready.exists():
   assert child.poll() is None,log.read_text()[-4000:]
   assert time.monotonic()<deadline,'readiness timeout';time.sleep(.1)
  def move(x,y):run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={x},y={y}}}))')
  points=[]
  for ratio in (.3,.7):
   move(round(monitor['x']+monitor['width']/monitor['scale']*ratio),round(monitor['y']+monitor['height']/monitor['scale']*.5))
   deadline=time.monotonic()+12
   while True:
    assert child.poll() is None,log.read_text()[-4000:]
    probes=[json.loads(line.split('SCRIPT_PROBE:',1)[1]) for line in log.read_text().splitlines() if 'SCRIPT_PROBE:' in line]
    if probes and probes[-1]['frames']>30 and (not points or probes[-1]['frames']>points[-1]['frames']) and (not points or probes[-1]['moves']>points[-1]['moves']):
     points.append(probes[-1]);break
    assert time.monotonic()<deadline,log.read_text()[-4000:]
    time.sleep(.1)
  assert points[1]['moves']>points[0]['moves']>0,points
  assert abs(points[1]['x']-points[0]['x'])+abs(points[1]['y']-points[0]['y'])>50,points
  for point in points:assert abs(point['x']-point['tx'])<2 and abs(point['y']-point['ty'])<2,point
  assert 'ScriptEngine [' not in log.read_text(),log.read_text()[-4000:]
  result={'lifecycle':True,'script_properties':True,'layer_setters':True,'effect_visibility':True,'cursor_move_world_coordinates':points}
 finally:
  run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={old_cursor["x"]},y={old_cursor["y"]}}}))')
  if child.poll() is None and record and identity(child.pid)==record:
   os.killpg(child.pid,signal.SIGTERM)
   try:child.wait(timeout=8)
   except subprocess.TimeoutExpired:
    if identity(child.pid)==record:os.killpg(child.pid,signal.SIGKILL)
    child.wait(timeout=3)
assert child.returncode==0,log.read_text()[-4000:]
assert run(str(pathlib.Path.home()/'.local/bin/wallpaper-lab'),'--status')==status,'personal renderer changed'
result['clean_shutdown']=True;result['personal_renderer_preserved']=True
(out/'results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))

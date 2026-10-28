#!/usr/bin/env python3
"""Exercise real PulseAudio/PipeWire -> FFT -> CEF/SceneScript using a silent null sink.
Needs a running graphical session, Node >=22, pactl and paplay. Never changes the
user's default audio devices, wallpaper settings or desktop renderer.
"""
import argparse, base64, json, math, os, signal, socket, struct, subprocess, time
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument('--app',required=True,help='Runtime executable, with CEF resources beside it')
p.add_argument('--assets',required=True)
p.add_argument('--output',required=True)
p.add_argument('--song',help='Local real song used for an additional visualizer check')
p.add_argument('--preset',help='Optional installed preset ID for asset/override checks')
p.add_argument('--workshop',help='Optional actual web wallpaper to exercise')
a=p.parse_args();app=Path(a.app).resolve();out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
probe=Path(__file__).with_name('cef_probe.mjs').resolve()
results={};processes=[];module=None
sink=f'wallpaper_lab_test_{os.getpid()}'
env=os.environ.copy();env['LD_LIBRARY_PATH']=':'.join(str(app.parent/x) for x in ['.','lib','deps'])
env['WALLPAPER_LAB_AUDIO_MONITOR']=sink+'.monitor'
env['XDG_CONFIG_HOME']=str(out/'config');env['XDG_STATE_HOME']=str(out/'state')
with socket.socket() as sock:
 sock.bind(('127.0.0.1',0));port=sock.getsockname()[1]
def run(*args,**kwargs):return subprocess.check_output(args,text=True,timeout=20,**kwargs).strip()
def evaluate(expression):return json.loads(run('node',str(probe),str(port),expression))
def identity(pid):
 path=Path('/proc')/str(pid)
 return (path.joinpath('stat').read_text().rsplit(')',1)[1].split()[19],path.joinpath('exe').resolve())
def stop(record):
 child,expected=record
 if child.poll() is None:
  try:
   if identity(child.pid)==expected:os.killpg(child.pid,signal.SIGTERM)
  except FileNotFoundError:pass
  try:child.wait(timeout=5)
  except subprocess.TimeoutExpired:
   if identity(child.pid)==expected:os.killpg(child.pid,signal.SIGKILL)
   child.wait(timeout=5)
 return child.returncode

def start(project,name,extra=(),ready_expected=True):
 ready=out/(name+'.ready');ready.unlink(missing_ok=True)
 log=out/(name+'.log')
 args=[str(app),'--render','--ready-file',str(ready),'--assets-dir',a.assets,'--volume','0',
       '--fps','30','--window','0x0x640x360','--no-fullscreen-pause',f'--remote-debugging-port={port}',*extra,str(project)]
 with log.open('w') as f:child=subprocess.Popen(args,env=env,cwd=app.parent,stdout=f,stderr=subprocess.STDOUT,start_new_session=True)
 record=(child,identity(child.pid));processes.append(record)
 deadline=time.monotonic()+15
 while child.poll() is None and not ready.exists() and time.monotonic()<deadline:time.sleep(.05)
 if ready_expected:assert ready.exists() and child.poll() is None,log.read_text()[-4000:]
 else:assert not ready.exists() and child.poll() not in (None,0),'Broken page reported ready'
 return record

def audio_sweep(read_metrics,prefix):
 time.sleep(.5)
 quiet=read_metrics();assert quiet['left']<.02 and quiet['right']<.02,quiet
 measurements={'silence':quiet}
 for channel in (0,1):
  tone=out/f'tone-{channel}.raw'
  tone.write_bytes(b''.join(struct.pack('<ff',*( [0.25*math.sin(2*math.pi*440*i/44100),0] if channel==0 else [0,0.25*math.sin(2*math.pi*440*i/44100)])) for i in range(44100*3)))
  player=subprocess.Popen(['paplay','--raw','--format=float32le','--rate=44100','--channels=2','--latency-msec=30','--device='+sink,str(tone)])
  began=time.monotonic();latest=None
  try:
   while time.monotonic()-began<1.5:
    time.sleep(.08);latest=read_metrics()
    if latest['left' if channel==0 else 'right']>.2:break
   assert latest['left' if channel==0 else 'right']>.2,latest
   assert latest['right' if channel==0 else 'left']<.02,latest
   measurements[f'channel{channel}']={**latest,'response_seconds':round(time.monotonic()-began,3)}
   if prefix=='workshop':
    image=evaluate('document.querySelector("canvas").toDataURL()')
    (out/f'{prefix}-{channel}.png').write_bytes(base64.b64decode(image.split(',')[1]))
  finally:
   player.wait(timeout=6)
  time.sleep(.7);quiet=read_metrics()
  assert quiet['left']<.02 and quiet['right']<.02,quiet
  measurements[f'after{channel}']=quiet
 results[prefix]=measurements

try:
 module=run('pactl','load-module','module-null-sink','sink_name='+sink)
 fixture=out/'web-fixture';fixture.mkdir(exist_ok=True)
 project={'type':'web','title':'Audio test','file':'index.html','general':{'supportsaudioprocessing':True,'properties':{'level':{'type':'slider','value':7,'min':0,'max':10,'text':'Level'},'enabled':{'type':'bool','value':True,'text':'Enabled'}}}}
 (fixture/'project.json').write_text(json.dumps(project))
 (fixture/'index.html').write_text('''<!doctype html><canvas id="canvas" width="640" height="360"></canvas><script>
window.metrics={left:0,right:0,n:0};
wallpaperRegisterAudioListener(a=>{metrics={left:Math.max(...a.slice(0,64)),right:Math.max(...a.slice(64)),n:metrics.n+1,length:a.length,finite:a.every(Number.isFinite)};let c=canvas.getContext('2d');c.fillStyle='#10202b';c.fillRect(0,0,640,360);c.fillStyle='#76c9a6';c.fillRect(30,330-300*metrics.left,240,300*metrics.left);c.fillStyle='#aa99ee';c.fillRect(370,330-300*metrics.right,240,300*metrics.right);});
window.wallpaperPropertyListener={applyUserProperties:p=>window.props=p,applyGeneralProperties:p=>window.general=p};
</script>''')
 # Verify the actual controller launch plan, including legacy muted settings.
 config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True)
 settings=config/'settings.json';settings.write_text(json.dumps({'muted':True}))
 plan_env=env|{'WALLPAPER_LAB_LIBRARY':str(out)}
 plan=json.loads(run(str(app),'--plan',str(fixture),env=plan_env))
 assert plan[plan.index('--volume')+1]=='0' and '--no-audio-processing' not in plan
 settings.write_text(json.dumps({'muted':False,'disable_audio_processing':True}))
 plan=json.loads(run(str(app),'--plan',str(fixture),env=plan_env))
 assert plan[plan.index('--volume')+1]=='100' and '--no-audio-processing' in plan
 results['independent_audio_controls']=True
 record=start(fixture,'web')
 time.sleep(.3)
 assert evaluate('props.level.value===7 && props.enabled.value===true && general.fps===30')
 audio_sweep(lambda:evaluate('metrics'),'web')
 assert evaluate('metrics.length===128 && metrics.finite && metrics.n>5')
 evaluate('location.reload();null');time.sleep(1)
 assert evaluate('metrics.n>5 && props.level.value===7'),'Bridge lost after reload'
 results['reload']=True
 state=out/'state/wallpaper-lab';state.mkdir(parents=True,exist_ok=True)
 current={'pid':record[0].pid,'start':record[1][0],'path':str(fixture),'title':'Audio test'}
 (state/'running.json').write_text(json.dumps(current))
 broken=out/'broken-fixture';broken.mkdir(exist_ok=True)
 (broken/'project.json').write_text(json.dumps({'type':'scene','title':'Broken','file':'scene.json'}))
 (broken/'scene.json').write_text('{invalid JSON')
 failed=subprocess.run([str(app),'--apply',str(broken)],env=plan_env,capture_output=True,text=True,timeout=40)
 assert failed.returncode!=0 and record[0].poll() is None
 assert json.loads((state/'running.json').read_text())==current
 results['failed_launch_preserves_previous']=True
 assert stop(record)==0,'Web shutdown crashed'
 record=start(fixture,'overrides',['--set-property','level=9','--set-property','enabled=false'])
 time.sleep(.3);assert evaluate('props.level.value===9 && props.enabled.value===false');assert stop(record)==0;results['typed_web_overrides']=True
 record=start(fixture,'disabled',['--no-audio-processing'])
 time.sleep(.5)
 assert evaluate('metrics.n>5 && metrics.left===0 && metrics.right===0')
 assert not any(s['properties'].get('application.process.id')==str(record[0].pid) for s in json.loads(run('pactl','-f','json','list','source-outputs')))
 results['disabled_no_capture']=True
 assert stop(record)==0
 if a.workshop:
  record=start(Path(a.workshop),'workshop')
  evaluate('wallpaperRegisterAudioListener(a=>{window.labMetrics={left:Math.max(...a.slice(0,64)),right:Math.max(...a.slice(64))};wallpaperAudioListener(a)});null')
  audio_sweep(lambda:evaluate('labMetrics'),'workshop')
  if a.song:
   player=subprocess.Popen(['paplay','--device='+sink,str(Path(a.song).resolve())]);time.sleep(.2)
   try:
    samples=[]
    for _ in range(20):samples.append(evaluate('labMetrics'));time.sleep(.2)
    assert max(max(x['left'],x['right'])for x in samples)>.02,samples
    results['real_song_visualizer']={'samples':samples,'peak':max(max(x['left'],x['right'])for x in samples)}
   finally:player.terminate();player.wait(5)
   time.sleep(.7);quiet=evaluate('labMetrics');assert quiet['left']<.02 and quiet['right']<.02,quiet
  assert stop(record)==0
 if a.preset:
  plan=json.loads(run(str(app),'--plan',a.preset,env=env));resolved=plan[plan.index('--bg')+1]
  record=start(Path(resolved),'preset')
  evaluate('window.labImage=new Image();labImage.src=imagePath;null');time.sleep(1)
  loaded=evaluate('({width:labImage.naturalWidth,height:labImage.naturalHeight,background:imageBackground,path:imagePath})')
  assert loaded['width']>0 and loaded['background'],loaded
  results['preset_file_loaded']=loaded
  assert stop(record)==0
 # A missing HTML page must fail before the controller would replace the old scene.
 project['file']='missing.html';(fixture/'project.json').write_text(json.dumps(project))
 record=start(fixture,'missing',ready_expected=False)
 results['missing_page_not_ready']=True
 # SceneScript fixture tests persistent typed buffers at every supported resolution.
 scene=out/'scene-fixture';scene.mkdir(exist_ok=True)
 (scene/'project.json').write_text(json.dumps({'type':'scene','title':'Scene audio test','file':'scene.json','general':{'supportsaudioprocessing':True}}))
 script="""const buffers=[16,32,64].map(n=>engine.registerAudioBuffers(n));
const saved=buffers.map(b=>b.left); let last=-1;
export function update(value){
 const b=buffers[2]; let valid=true;
 for(let k=0;k<3;k++){let x=buffers[k];valid=valid && x.left instanceof Float32Array && x.left===saved[k] && x.left.length===[16,32,64][k];for(let i=0;i<x.left.length;i++)valid=valid && Math.abs(x.average[i]-(x.left[i]+x.right[i])/2)<0.00001;}
 const tick=Math.floor(engine.frametime?engine.time*5:0);
 if(tick!==last){last=tick;console.log('AUDIO_PROBE:'+JSON.stringify({left:Math.max(...b.left),right:Math.max(...b.right),valid:valid}));}
 return 'Audio test';} """
 (scene/'scene.json').write_text(json.dumps({'camera':{'center':'0 0 0','eye':'0 0 1','up':'0 1 0'},'general':{'orthogonalprojection':{'width':640,'height':360},'clearcolor':'0 0 0'},'objects':[{'id':1,'name':'Audio test','origin':'320 180 0','text':{'value':'Audio test','script':script},'pointsize':32}]}))
 record=start(scene,'scene')
 def scene_metrics():
  lines=[x.split('AUDIO_PROBE:',1)[1] for x in (out/'scene.log').read_text().splitlines() if 'AUDIO_PROBE:' in x]
  assert lines,(out/'scene.log').read_text()[-3000:]
  value=json.loads(lines[-1]);assert value['valid'],value
  return value
 audio_sweep(scene_metrics,'scene')
 assert stop(record)==0,'Scene shutdown crashed'
 (out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
 print(json.dumps(results,indent=2))
finally:
 for record in processes:stop(record)
 if module:run('pactl','unload-module',module)

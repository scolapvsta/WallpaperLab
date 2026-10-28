#!/usr/bin/env python3
"""Isolated SDL/FFmpeg playback and SceneScript transport checks on a null sink."""
import argparse,json,os,pathlib,signal,struct,subprocess,time,wave,math
from array import array
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--assets',required=True);p.add_argument('--workshop',type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
sink='lab_transport_'+str(os.getpid());module=subprocess.check_output(['pactl','load-module','module-null-sink','sink_name='+sink,'sink_properties=device.description='+sink],text=True).strip()
env=os.environ|{'PULSE_SINK':sink,'WALLPAPER_LAB_AUDIO_OUTPUT':sink,'SDL_AUDIODRIVER':'pulseaudio','WALLPAPER_LAB_AUDIO_TRACE':'1','WALLPAPER_LAB_AUDIO_MONITOR':sink+'.monitor','XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])}
children=[]
def identity(pid):
 p=pathlib.Path('/proc')/str(pid);return(p.joinpath('stat').read_text().rsplit(')',1)[1].split()[19],str(p.joinpath('exe').resolve()))
def start(args,name):
 log=(out/(name+'.log')).open('wb');c=subprocess.Popen(args,env=env,cwd=app.parent,stdout=log,stderr=log,start_new_session=True);r=(c,identity(c.pid),log);children.append(r);return r
def stop(r):
 c,ident,log=r
 if c.poll() is None and identity(c.pid)==ident:os.killpg(c.pid,signal.SIGTERM)
 try:c.wait(5)
 except subprocess.TimeoutExpired:
  if identity(c.pid)==ident:os.killpg(c.pid,signal.SIGKILL)
  c.wait()
 log.close()
def waitfile(path,c,limit=15):
 until=time.monotonic()+limit
 while not path.exists() and c.poll() is None and time.monotonic()<until:time.sleep(.05)
 assert path.exists() and c.poll() is None,(path,c.poll())
try:
 root=out/'fixture';root.mkdir(exist_ok=True)
 for freq in (440,880):
  with wave.open(str(root/f'{freq}.wav'),'wb')as f:
   f.setnchannels(2);f.setsampwidth(2);f.setframerate(48000);data=b''.join(struct.pack('<hh',*(2*[round(10000*math.sin(2*math.pi*freq*i/48000))]))for i in range(round(48000*(.3 if freq==440 else .6))));f.writeframes(data)
 script="""let age=0,phase=-1,ended=false;export function init(){let v=new Vec3(1,2,3).mix(new Vec3(3,4,5),.5);if(v.x!==2||v.y!==3||v.z!==4)throw new Error('Vector transport control regression');console.log('TRANSPORT:'+JSON.stringify({silent:!thisScene.getLayer('First').isPlaying()&&!thisScene.getLayer('Second').isPlaying()}));}
 export function update(v){age+=engine.frametime;if(age>3.8&&!ended){ended=true;console.log('TRANSPORT:'+JSON.stringify({ended:!thisScene.getLayer('Second').isPlaying()}));}let n=Math.min(4,Math.floor(age));if(n!==phase){phase=n;let a=thisScene.getLayer('First'),b=thisScene.getLayer('Second');if(n===0){a.volume=.4;a.play();}if(n===1)a.pause();if(n===2)a.play();if(n===3){a.stop();b.volume=.8;b.play();}if(n===4)b.stop();console.log('TRANSPORT:'+JSON.stringify({phase:n,first:a.isPlaying(),second:b.isPlaying()}));}return 'Audio transport';}"""
 (root/'project.json').write_text(json.dumps({'type':'scene','file':'scene.json','title':'Transport','general':{'supportsaudioprocessing':True}}))
 scene={'camera':{'center':'0 0 0','eye':'0 0 1','up':'0 1 0'},'general':{'orthogonalprojection':{'width':640,'height':360},'clearcolor':'0 0 0'},'objects':[{'id':1,'name':'First','sound':['440.wav'],'startsilent':True,'playbackmode':'loop'},{'id':2,'name':'Second','sound':['880.wav'],'startsilent':True,'playbackmode':'single'},{'id':3,'name':'Controller','origin':'320 180 0','text':{'value':'Transport','script':script},'pointsize':24}]}
 (root/'scene.json').write_text(json.dumps(scene));ready=out/'ready';ready.unlink(missing_ok=True)
 raw=(out/'capture.f32').open('wb');capture=subprocess.Popen(['parec','--device='+sink+'.monitor','--format=float32le','--rate=48000','--channels=2','--latency-msec=20'],stdout=raw,stderr=subprocess.DEVNULL)
 renderer=start([str(app),'--render','--ready-file',str(ready),'--assets-dir',a.assets,'--fps','30','--volume','100','--noautomute','--window','0x0x640x360','--no-fullscreen-pause',str(root)],'transport');waitfile(ready,renderer[0]);
 monitors=json.loads(subprocess.check_output(['hyprctl','monitors','-j'],text=True));target=next((m for m in monitors if m['name']=='DP-6'),monitors[-1]);subprocess.run(['hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{renderer[0].pid}",workspace="{target["activeWorkspace"]["id"]}",follow=false}}))'],capture_output=True)
 time.sleep(.6);(out/"sink-inputs.json").write_text(subprocess.check_output(["pactl","-f","json","list","sink-inputs"],text=True));(out/"sinks.json").write_text(subprocess.check_output(["pactl","-f","json","list","sinks"],text=True));time.sleep(4.4)
 stop(renderer);capture.terminate();capture.wait(5);raw.close()
 lines=[json.loads(x.split('TRANSPORT:',1)[1])for x in(out/'transport.log').read_text().splitlines()if'TRANSPORT:'in x];assert lines[0]['silent'];phases={x['phase']:x for x in lines if'phase'in x};assert len(phases)==5 and any(x.get('ended')for x in lines),lines
 assert phases[0]['first'] and not phases[0]['second'];assert not phases[1]['first'];assert phases[2]['first'];assert not phases[3]['first']and phases[3]['second'];assert not phases[4]['second']
 stereo=array('f');stereo.frombytes((out/'capture.f32').read_bytes());data=[(stereo[i]+stereo[i+1])/2 for i in range(0,len(stereo)-1,2)];hop=4800;segments=[]
 for offset in range(0,len(data)-hop,hop):
  block=data[offset:offset+hop];rms=math.sqrt(sum(x*x for x in block)/hop);powers={}
  for freq in [440,880]:
   coeff=2*math.cos(2*math.pi*freq/48000);q1=q2=0
   for x in block:q=x+coeff*q1-q2;q2=q1;q1=q
   powers[freq]=max(0,q1*q1+q2*q2-coeff*q1*q2)
  segments.append({'seconds':round(offset/48000,2),'rms':rms,'hz':max(powers,key=powers.get),'powers':powers})
 active=[s for s in segments if s['rms']>.005];assert any(abs(s['hz']-440)<20 for s in active)and any(abs(s['hz']-880)<20 for s in active),segments
 assert all(min(abs(s['hz']-440),abs(s['hz']-880))<25 for s in active),active
 assert sum(s['rms']<.001 for s in segments)>8,'Pause did not produce silence'
 result={'transport':lines,'capture':segments,'renderer_exit':renderer[0].returncode};assert result['renderer_exit']==0,result['renderer_exit'];
 if a.workshop:
  ready=out/'music.ready';ready.unlink(missing_ok=True);raw=(out/'music-capture.f32').open('wb');capture=subprocess.Popen(['parec','--device='+sink+'.monitor','--format=float32le','--rate=48000','--channels=2','--latency-msec=20'],stdout=raw,stderr=subprocess.DEVNULL)
  real=start([str(app),'--render','--ready-file',str(ready),'--assets-dir',a.assets,'--fps','30','--volume','100','--noautomute','--window','0x0x640x360','--no-fullscreen-pause',str(a.workshop.resolve())],'real-music');waitfile(ready,real[0]);time.sleep(6);stop(real);capture.terminate();capture.wait(5);raw.close()
  values=array('f');values.frombytes((out/'music-capture.f32').read_bytes());peak=max(map(abs,values));rms=math.sqrt(sum(x*x for x in values)/len(values));played=[line for line in(out/'real-music.log').read_text().splitlines()if'Sound layer 'in line and' playing:'in line]
  assert len(played)==1,played;assert peak>.02 and rms>.001,{'peak':peak,'rms':rms};assert real[0].returncode==0
  from workshop_assets import read_asset
  project=json.loads((a.workshop/'project.json').read_text());scene=json.loads(read_asset(a.workshop,project['file']));song=next(o['sound'][0]for o in scene['objects']if o.get('sound'));(out/'real-song.ogg').write_bytes(read_asset(a.workshop,song))
  result['real_music']={'one_active_track':played,'peak':peak,'rms':rms,'clean_exit':True}
 (out/'results.json').write_text(json.dumps(result,indent=2));print(json.dumps({'transport_passed':True,'captured_seconds':len(data)/48000,'clean_exit':True}))
finally:
 for c in children:
  if c[0].poll() is None:stop(c)
 subprocess.run(['pactl','unload-module',module],check=True)

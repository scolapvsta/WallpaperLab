#!/usr/bin/env python3
"""Browser-level audio gates, measured on an isolated silent PulseAudio sink."""
import argparse,array,json,math,os,pathlib,signal,socket,struct,subprocess,time,wave
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);project=out/'fixture';project.mkdir(exist_ok=True);settings=config/'settings.json';profiles=config/'wallpaper_profiles.json';child=None;module=None;result={}
sink='lab_web_mute_'+str(os.getpid());env=os.environ|{'PULSE_SINK':sink,'WALLPAPER_LAB_AUDIO_MONITOR':sink+'.monitor','XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])+(':'+os.environ['LD_LIBRARY_PATH'] if os.environ.get('LD_LIBRARY_PATH') else '')}
with socket.socket()as sock:sock.bind(('127.0.0.1',0));port=sock.getsockname()[1]
def run(*args,**kwargs):return subprocess.check_output([str(x)for x in args],text=True,timeout=25,**kwargs).strip()
def write(file,value):
 tmp=file.with_suffix('.new');tmp.write_text(json.dumps(value));tmp.replace(file)
def policy(global_mute,local_audio):
 write(settings,{'muted':global_mute});write(profiles,{str(project):{'playback':{'audio':local_audio}}});time.sleep(.7)
def rms():
 rec=subprocess.Popen(['parec','--device='+sink+'.monitor','--format=float32le','--rate=44100','--channels=2','--latency-msec=20'],stdout=subprocess.PIPE,stderr=subprocess.DEVNULL)
 time.sleep(.7);rec.terminate();raw=rec.communicate(timeout=5)[0];samples=array.array('f');samples.frombytes(raw[:len(raw)//4*4]);assert len(samples)>10000
 return math.sqrt(sum(v*v for v in samples)/len(samples))
def probe(js):return json.loads(run('node',pathlib.Path(__file__).with_name('cef_probe.mjs'),port,js))
def check(name,audible):
 value=rms();result[name]=value;assert (value>.005)if audible else(value<.0001),(name,value)
try:
 module=run('pactl','load-module','module-null-sink','sink_name='+sink)
 with wave.open(str(project/'tone.wav'),'wb')as f:f.setparams((1,2,44100,0,'NONE','not compressed'));f.writeframes(b''.join(struct.pack('<h',int(6000*math.sin(i*2*math.pi*330/44100)))for i in range(44100)))
 (project/'project.json').write_text(json.dumps({'type':'web','title':'Synthetic live mute regression','file':'index.html'}))
 (project/'index.html').write_text('''<!doctype html><body style="background:#17213b"><audio id="media" src="tone.wav" autoplay loop></audio><iframe srcdoc="<script>let a=new AudioContext(),o=a.createOscillator(),g=a.createGain();g.gain.value=.08;o.frequency.value=880;o.connect(g).connect(a.destination);o.start();a.resume();</script>"></iframe><script>window.ac=new AudioContext();let o=ac.createOscillator(),g=ac.createGain();g.gain.value=.1;o.connect(g).connect(ac.destination);o.start();ac.resume();</script>''')
 policy(True,True);ready=out/'ready';log=(out/'renderer.log').open('w')
 child=subprocess.Popen([str(app),'--render','--ready-file',str(ready),'--assets-dir',str(project),'--volume','100','--fps','30','--no-fullscreen-pause','--window','0x0x640x360',f'--remote-debugging-port={port}','--lab-profile-file',str(profiles),'--lab-project',str(project),str(project)],env=env,cwd=app.parent,stdout=log,stderr=log,start_new_session=True)
 stat=pathlib.Path('/proc',str(child.pid),'stat').read_text();identity=(stat.rsplit(')',1)[1].split()[19],os.readlink(f'/proc/{child.pid}/exe'))
 deadline=time.monotonic()+25
 while not ready.exists():assert child.poll()is None,(out/'renderer.log').read_text()[-2500:];assert time.monotonic()<deadline;time.sleep(.1)
 time.sleep(.7);check('startup_global_mute',False)
 policy(False,False);check('local_mute',False)
 policy(False,True);check('both_enabled',True)
 assert probe('ac.state')=='running';policy(True,True);check('global_mute_live',False)
 policy(False,True);check('unmute_live',True)
 policy(False,False);probe('media.muted=false;media.volume=1;location.reload();null');time.sleep(1);check('local_mute_survives_reload_and_media_unmute',False)
 assert child.poll()is None;result['same_renderer_pid']=child.pid;print(json.dumps(result,indent=2))
finally:
 if child and child.poll()is None:
  proc=pathlib.Path('/proc',str(child.pid));now=((proc/'stat').read_text().rsplit(')',1)[1].split()[19],os.readlink(proc/'exe'));assert now==identity;os.killpg(child.pid,signal.SIGTERM);child.wait(10)
 if module:run('pactl','unload-module',module)
 (out/'results.json').write_text(json.dumps(result,indent=2))

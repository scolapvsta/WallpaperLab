#!/usr/bin/env python3
"""Controller-to-compositor transition: owned snapshots, monitor gutter and lifecycle."""
import argparse,json,pathlib,os,subprocess,time,io,threading,signal
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--interfere',action='store_true');p.add_argument('--slow-retire',action='store_true');p.add_argument('--effect',type=int,default=1,choices=range(1,9));p.add_argument('--mode',default='single',choices=['single','mirror','span']);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);env=os.environ|{f'XDG_{n}_HOME':str(out/n.lower())for n in ['CONFIG','STATE','DATA','CACHE']};env['LD_LIBRARY_PATH']=':'.join(str(app.parent/x)for x in ['.','lib','deps']);env['WALLPAPER_LAB_TRANSITION_TRACE']=str(out/'trace.log');(out/'trace.log').unlink(missing_ok=True);log=open(out/'controller.log','w');personal=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=json.loads(subprocess.check_output([personal,'--status'],text=True));config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True)
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,text=True,stderr=log)
def status():return json.loads(run(app,'--status'))
def pixel(x,y):return tuple(Image.open(io.BytesIO(subprocess.check_output(['grim','-g',f'{x},{y} 2x2','-t','ppm','-'],env=env))).convert('RGB').getpixel((0,0)))
colors=[(180,20,20),(20,60,180)];paths=[];samples=[];job=None;interference=None;writes=[0];stop_writes=threading.Event();retirement=None;paused=[False];old=None
def same_old():
 try:
  proc=pathlib.Path('/proc')/str(old['pid']);return proc.joinpath('exe').resolve()==app and proc.joinpath('stat').read_text().rsplit(')',1)[1].split()[19]==old['start']
 except(FileNotFoundError,TypeError,KeyError):return False
try:
 for n,color in enumerate(colors):
  file=out/f'{n}.png';Image.new('RGB',(1280,720),color).save(file);paths.append(json.loads(run(app,'--import',file))[0])
 (config/'settings.json').write_text(json.dumps({'workshop_path':str(steam),'mode':a.mode,'screen':0,'fps':60,'muted':True,'transition_type':0,'transition_duration':2000,'fullscreen_policy':0}));run(app,'--apply',paths[0]);time.sleep(.3);old=status();points=[(2,300),(4,500),(500,2),(1,700),(10,10)];visible=[(xy,pixel(*xy))for xy in points];point=next((xy for xy,c in visible if c[0]>90 and c[1]<70 and c[2]<70),None);assert point,('no exposed gutter',visible)
 run(app,'--ui-option','transition_type',a.effect);
 if a.slow_retire:
  def stall_shutdown():
   deadline=time.monotonic()+12
   while not stop_writes.is_set()and time.monotonic()<deadline:
    if job and (pathlib.Path('/proc')/str(job.pid)/'task'/str(job.pid)/'children').exists()and (pathlib.Path('/proc')/str(job.pid)/'task'/str(job.pid)/'children').read_text().strip()and list((out/'state/wallpaper-lab/transitions').glob('*/0.bin'))and same_old():
     os.killpg(old['pid'],signal.SIGSTOP);paused[0]=True;return
    time.sleep(.002)
  retirement=threading.Thread(target=stall_shutdown);retirement.start()
 if a.interfere:
  argv=(pathlib.Path('/proc')/str(old['pid'])/'cmdline').read_bytes().decode().split('\0');feed=pathlib.Path(argv[argv.index('--lab-feed-dir')+1]);feed.mkdir(parents=True,exist_ok=True);
  def clobber():
   while not stop_writes.is_set():
    temp=feed/'inspector.tmp';temp.write_text(json.dumps({'path':'inspector-changing-project','output':'DP-5'}));temp.replace(feed/'request.json');writes[0]+=1;time.sleep(.01)
  interference=threading.Thread(target=clobber);interference.start()
 began=time.monotonic();job=subprocess.Popen([str(app),'--apply',paths[1]],env=env,stdout=log,stderr=log)
 while time.monotonic()-began<(8 if a.slow_retire else 5):
  samples.append({'t':time.monotonic()-began,'rgb':pixel(*point)})
  time.sleep(.025)
 job.wait(35);assert job.returncode==0;current=status();assert current['path']==paths[1]and current['pid']!=old['pid'];argv=(pathlib.Path('/proc')/str(current['pid'])/'cmdline').read_bytes().decode().split('\0');transition_config=json.loads(argv[argv.index('--lab-transition')+1])if'--lab-transition'in argv else{};outputs=json.loads(run(app,'--display-layout'))['renderer'];expected=[min(outputs,key=lambda n:outputs[n][0])]if a.mode=='single'else list(outputs);traces=(out/'trace.log').read_text()if(out/'trace.log').exists()else'';missing=[name for name in expected if name+' started '+str(a.effect)not in traces or name+' completed'not in traces]
 transition=[s for s in samples if s['rgb'][0]>30 and s['rgb'][2]>30];black=[s for s in samples if sum(s['rgb'])<35];result={'mode':a.mode,'effect':a.effect,'slow_retirement':paused[0],'missing_outputs':missing,'transition_config':transition_config,'sample_point':point,'old_pid':old['pid'],'new_pid':current['pid'],'intermediate_frames':len(transition),'black_frames':len(black),'concurrent_inspector_writes':writes[0],'samples':samples};(out/'results.json').write_text(json.dumps(result,indent=2));print(json.dumps({k:v for k,v in result.items()if k!='samples'},indent=2))
 if a.interfere:assert writes[0]>100,'interference writer did not run'

 if a.slow_retire:assert paused[0],'slow teardown was not simulated'
 if a.effect==1:assert len(transition)>8,'desktop skipped the configured fade'
 if a.effect not in(2,8):assert not black,'desktop flashed black'
 assert not missing,('transition did not run on every output',missing,traces)
 if (gates:={v.get('start_file')for v in transition_config.values()})!={None}:assert len(gates)==1 and all(gates),'outputs must share one start gate'
finally:
 stop_writes.set()
 if interference:interference.join(3)
 if retirement:retirement.join(3)
 if paused[0]and same_old():os.killpg(old['pid'],signal.SIGCONT)
 if job and job.poll()is None:job.wait(35)
 run(app,'--stop')
assert before==json.loads(subprocess.check_output([personal,'--status'],text=True)),'personal renderer changed'

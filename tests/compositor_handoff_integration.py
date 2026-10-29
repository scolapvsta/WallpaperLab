#!/usr/bin/env python3
"""Patterned desktop pixels must stay still during renderer handoff, despite layer animations."""
import argparse,pathlib,json,subprocess,os,time,signal,io,threading
from PIL import Image,ImageDraw
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);env=os.environ|{f'XDG_{n}_HOME':str(out/n.lower())for n in ['CONFIG','STATE','DATA','CACHE']};env['LD_LIBRARY_PATH']=':'.join(str(app.parent/x)for x in ['.','lib','deps']);log=open(out/'log','w');config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);steam=out/'steam';steam.mkdir(exist_ok=True);personal=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=subprocess.check_output([personal,'--status']);old=None;job=None;paused=False;thread=None;stop=threading.Event();samples=[]
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,stderr=log,text=True)
def same_old():
 try:
  proc=pathlib.Path('/proc')/str(old['pid']);return proc.joinpath('exe').resolve()==app and proc.joinpath('stat').read_text().rsplit(')',1)[1].split()[19]==old['start']
 except(FileNotFoundError,TypeError):return False
def pixel():return tuple(Image.open(io.BytesIO(subprocess.check_output(['grim','-g','2,500 2x2','-t','ppm','-']))).convert('RGB').getpixel((0,0)))
try:
 old_image=out/'old.png';im=Image.new('RGB',(1280,720),(20,20,180));d=ImageDraw.Draw(im);d.rectangle((0,0,1279,239),fill=(180,20,20));d.rectangle((0,240,1279,479),fill=(20,180,20));im.save(old_image);new_image=out/'new.png';Image.new('RGB',(1280,720),(180,160,20)).save(new_image);paths=[json.loads(run(app,'--import',f))[0]for f in [old_image,new_image]];(config/'settings.json').write_text(json.dumps({'workshop_path':str(steam),'mode':'single','screen':0,'fps':60,'muted':True,'fullscreen_policy':0,'transition_type':0}));run(app,'--apply',paths[0]);time.sleep(.9);old=json.loads(run(app,'--status'));base=pixel();assert base[1]>base[0]+80 and base[1]>base[2]+80,('desktop gutter not visible',base);run(app,'--ui-option','transition_type',1)
 def pause_old():
  global paused
  while not stop.is_set():
   try:
    if job:
     children=pathlib.Path('/proc',str(job.pid),'task',str(job.pid),'children').read_text().split();renderers=[pid for pid in children if pathlib.Path('/proc',pid,'exe').resolve()==app and b'--render'in pathlib.Path('/proc',pid,'cmdline').read_bytes().split(b'\0')]
     if renderers and same_old():os.killpg(old['pid'],signal.SIGSTOP);paused=True;return
   except FileNotFoundError:pass
   time.sleep(.002)
 thread=threading.Thread(target=pause_old);thread.start();began=time.monotonic();job=subprocess.Popen([str(app),'--apply',paths[1]],env=env,stdout=log,stderr=log)
 while time.monotonic()-began<6:
  if list((out/'state/wallpaper-lab/transitions').glob('*/start.json')):break
  value=pixel()
  if not list((out/'state/wallpaper-lab/transitions').glob('*/start.json')):samples.append({'t':time.monotonic()-began,'rgb':value})
  time.sleep(.025)
 job.wait(35);assert job.returncode==0 and paused;assert len(samples)>20 and list((out/'state/wallpaper-lab/transitions').glob('*/start.json')),'handoff gate was not exercised'
 changed=[s for s in samples if max(abs(v-b)for v,b in zip(s['rgb'],base))>5];result={'baseline':base,'pre_gate_samples':len(samples),'unexpected_changes':len(changed),'changed_samples':changed,'all_samples':samples};(out/'results.json').write_text(json.dumps(result,indent=2));print(json.dumps({k:v for k,v in result.items()if not k.endswith('samples')},indent=2));assert not changed,'compositor animated the held snapshot before the app transition started'
finally:
 stop.set()
 if thread:thread.join(3)
 if paused and same_old():os.killpg(old['pid'],signal.SIGCONT)
 if job and job.poll()is None:job.wait(35)
 run(app,'--stop')
assert before==subprocess.check_output([personal,'--status']),'personal wallpaper changed'

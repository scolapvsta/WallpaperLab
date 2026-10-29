#!/usr/bin/env python3
"""Real embedded-renderer transition orientation/readiness/state and live color regression.
Opt-in desktop test: own isolated children, PID/starttime/executable checked on shutdown.
"""
import argparse,json,os,pathlib,signal,struct,subprocess,time
from PIL import Image,ImageChops
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])}
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,text=True)
def identity(pid):
 root=pathlib.Path('/proc')/str(pid);return str((root/'exe').readlink()),(root/'stat').read_text().rsplit(')',1)[1].split()[19]
def write(path,value):
 temp=path.with_suffix('.tmp');temp.write_text(json.dumps(value));temp.replace(path)
personal=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=json.loads(subprocess.check_output([personal,'--status'],text=True));layout=json.loads(run(app,'--display-layout'))['renderer'];name=min(layout,key=lambda n:layout[n][0]);rects={name:layout[name]}
valid=next(i for i in json.loads(run(app,'--list'))if not i['reason']);plan=json.loads(run(app,'--plan',valid['id']));assets=plan[plan.index('--assets-dir')+1]
fixture=out/'fixture';fixture.mkdir(exist_ok=True);(fixture/'project.json').write_text(json.dumps({'type':'web','title':'Transition/color fixture','file':'index.html'}));(fixture/'index.html').write_text('<style>html,body{margin:0;width:100%;height:100%;background:linear-gradient(to bottom,rgb(80,120,160) 0%,rgb(80,120,160) 50%,rgb(120,60,20) 50%,rgb(120,60,20) 100%)}</style>')
feed=out/'feed';feed.mkdir(exist_ok=True);request=feed/'request.json';profiles=out/'profiles.json';live=out/'live.json';trace=out/'trace.log';trace.unlink(missing_ok=True)
old=Image.new('RGB',(640,360),(180,20,20));old.paste((20,180,20),(0,180,640,360));snapshot=out/'old.bin';snapshot.write_bytes(struct.pack('=5I',0x574c4631,0,640,360,0)+old.tobytes())
results={};allframes=[]
for effect in range(1,9):
 (feed/'frame.bin').unlink(missing_ok=True);write(request,{'path':str(fixture),'output':name});write(profiles,{str(fixture):{'image':{'brightness':0,'contrast':1,'saturation':1}}});live.unlink(missing_ok=True)
 transition={name:{'file':str(snapshot),'effect':effect,'x':.3,'y':.3}}
 args=[str(app),'--render','--assets-dir',assets,'--fps','60','--volume','0','--no-fullscreen-pause','--scaling','stretch','--screen-root',name,'--bg',str(fixture),'--lab-layout',json.dumps(rects),'--lab-project',str(fixture),'--lab-profile-file',str(profiles),'--lab-live-file',str(live),'--lab-feed-dir',str(feed),'--lab-transition',json.dumps(transition)]
 log=open(out/f'engine-{effect}.log','w');child=subprocess.Popen(args,env=env|{'WALLPAPER_LAB_TRANSITION_TRACE':str(trace)},stdout=log,stderr=log,start_new_session=True);owner=None
 try:
  owner=identity(child.pid);frames=[];seq=-1;end=time.monotonic()+5
  while time.monotonic()<end:
   assert child.poll()is None,(out/f'engine-{effect}.log').read_text()[-3000:];write(request,{'path':str(fixture),'output':name})
   try:
    raw=(feed/'frame.bin').read_bytes();magic,pid,w,h,current=struct.unpack('=5I',raw[:20]);assert magic==0x574c4631 and pid==child.pid
    if current!=seq:seq=current;frames.append(Image.frombytes('RGB',(w,h),raw[20:]));
   except FileNotFoundError:pass
   if len(frames)>=25 and trace.exists()and trace.read_text().count('completed')>=effect:break
   time.sleep(.025)
  assert len(frames)>8,('too few frames',effect,len(frames));first,last=frames[0],frames[-1];top=(last.width//2,last.height//4);bottom=(last.width//2,last.height*3//4)
  assert max(abs(a-b)for a,b in zip(last.getpixel(top),(80,120,160)))<8,('new orientation/top',effect,last.getpixel(top));assert max(abs(a-b)for a,b in zip(last.getpixel(bottom),(120,60,20)))<8,('new orientation/bottom',effect,last.getpixel(bottom))
  assert first.getpixel(top)[0]>first.getpixel(top)[1]+70,('old orientation/top',effect,first.getpixel(top));assert first.getpixel(bottom)[1]>first.getpixel(bottom)[0]+70,('old orientation/bottom',effect,first.getpixel(bottom))
  if effect not in (2,8):assert all(sum(f.getpixel(top))>100 and sum(f.getpixel(bottom))>100 for f in frames),('unexpected black flash',effect)
  for n,f in enumerate(frames[::3]):f.save(out/f'effect-{effect}-{n}.png')
  if effect==8:
   def fresh(after):
    deadline=time.monotonic()+4
    while time.monotonic()<deadline:
     write(request,{'path':str(fixture),'output':name});raw=(feed/'frame.bin').read_bytes();header=struct.unpack('=5I',raw[:20])
     if header[4]>after:return Image.frombytes('RGB',(header[2],header[3]),raw[20:]),header[4]
     time.sleep(.03)
    raise AssertionError('color frame timeout')
   write(live,{'path':str(fixture),'image':{'brightness':.1,'contrast':1.2,'saturation':0}});changed,seq=fresh(seq);changed,seq=fresh(seq);pixel=changed.getpixel(top);assert max(pixel)-min(pixel)<3 and 115<pixel[0]<155,('live image values',pixel);changed.save(out/'adjusted.png')
   write(profiles,{str(fixture):{'image':{'brightness':.1,'contrast':1.2,'saturation':0}}});live.unlink();time.sleep(.12);saved,seq=fresh(seq);assert max(abs(a-b)for a,b in zip(pixel,saved.getpixel(top)))<3;assert identity(child.pid)==owner
   write(profiles,{str(fixture):{'image':{'brightness':0,'contrast':1,'saturation':1}}});time.sleep(.12);restored,seq=fresh(seq);assert max(abs(a-b)for a,b in zip(restored.getpixel(top),(80,120,160)))<8;results['live_persist_reset_without_restart']=True
  results[str(effect)]={'frames':len(frames),'upright_old_and_new':True,'no_unplanned_black':True}
 finally:
  if child.poll()is None and owner and identity(child.pid)==owner:
   os.killpg(child.pid,signal.SIGTERM)
   try:child.wait(8)
   except subprocess.TimeoutExpired:
    if identity(child.pid)==owner:os.killpg(child.pid,signal.SIGKILL)
    child.wait(3)
  log.close()
assert before==json.loads(subprocess.check_output([personal,'--status'],text=True)),'personal renderer changed';results['personal_renderer_preserved']=True;(out/'results.json').write_text(json.dumps(results,indent=2));print(json.dumps(results,indent=2))

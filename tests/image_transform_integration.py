#!/usr/bin/env python3
"""Native framebuffer tests: image transforms and custom slide/wipe direction and duration."""
import argparse,json,pathlib,os,subprocess,time,struct,signal
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);fixture=out/'fixture';fixture.mkdir(exist_ok=True);feed=out/'feed';feed.mkdir(exist_ok=True)
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])};personal=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=subprocess.check_output([personal,'--status']);request=feed/'request.json';profiles=out/'profiles.json';frame=feed/'frame.bin';results={};child=None;owner=None
(fixture/'project.json').write_text(json.dumps({'type':'web','title':'Image transform fixture','file':'index.html'}));(fixture/'index.html').write_text('<style>html,body{margin:0;width:100%;height:100%;overflow:hidden}body{display:grid;grid-template:1fr 1fr/1fr 1fr}div:nth-child(1){background:rgb(80,120,160)}div:nth-child(2){background:rgb(20,160,80)}div:nth-child(3){background:rgb(120,60,20)}div:nth-child(4){background:rgb(180,40,80)}</style><div></div><div></div><div></div><div></div>');assets=out/'assets';assets.mkdir(exist_ok=True)
layout=json.loads(subprocess.check_output([str(app),'--display-layout'],env=env,text=True))['renderer'];name=min(layout,key=lambda n:layout[n][0]);old=Image.new('RGB',(640,360),(230,20,180));snapshot=out/'old.bin';snapshot.write_bytes(struct.pack('=5I',0x574c4631,0,640,360,0)+old.tobytes())
def identity(pid):
 proc=pathlib.Path('/proc')/str(pid);return str((proc/'exe').readlink()),(proc/'stat').read_text().rsplit(')',1)[1].split()[19]
def write(path,value):
 tmp=path.with_suffix('.tmp');tmp.write_text(json.dumps(value));tmp.replace(path)
def stop():
 global child
 if child and child.poll()is None and identity(child.pid)==owner:os.killpg(child.pid,signal.SIGTERM);child.wait(12)
 child=None

def start(transition=None):
 global child,owner
 frame.unlink(missing_ok=True);write(profiles,{str(fixture):{'image':{}}});write(request,{'path':str(fixture),'output':name});args=[str(app),'--render','--assets-dir',str(assets),'--fps','60','--volume','0','--no-fullscreen-pause','--screen-root',name,'--bg',str(fixture),'--lab-layout',json.dumps({name:layout[name]}),'--lab-project',str(fixture),'--lab-profile-file',str(profiles),'--lab-feed-dir',str(feed)];
 if transition:args+=['--lab-transition',json.dumps({name:transition})]
 log=open(out/f'engine-{len(results)}.log','w');child=subprocess.Popen(args,env=env,stdout=log,stderr=log,start_new_session=True);owner=identity(child.pid)

def fresh(after=-1):
 end=time.monotonic()+8
 while time.monotonic()<end:
  assert child.poll()is None;write(request,{'path':str(fixture),'output':name})
  try:
   raw=frame.read_bytes();magic,pid,w,h,seq=struct.unpack('=5I',raw[:20])
   if seq>after and pid==child.pid:return Image.frombytes('RGB',(w,h),raw[20:]),seq
  except(FileNotFoundError,ValueError,struct.error):pass
  time.sleep(.025)
 raise AssertionError('frame timeout')
def pixel(im,x=.25,y=.25):return im.getpixel((int(im.width*x),int(im.height*y)))
def close(actual,expected):return max(abs(a-b)for a,b in zip(actual,expected))<9
try:
 start();base,seq=fresh();
 for _ in range(30):
  if close(pixel(base),(80,120,160)):break
  base,seq=fresh(seq)
 assert close(pixel(base),(80,120,160));native_identity=identity(child.pid)
 for key,value,expected in [('flip_x',1,(20,160,80)),('flip_y',1,(120,60,20)),('rotation',180,(180,40,80))]:
  write(profiles,{str(fixture):{'image':{key:value}}});time.sleep(.14);im,seq=fresh(seq);im,seq=fresh(seq);assert close(pixel(im),expected),(key,pixel(im));im.save(out/f'{key}.png');results[key]=True
 for key,value in [('hue',90),('temperature',.7),('gamma',2),('vignette',1)]:
  write(profiles,{str(fixture):{'image':{key:value}}});time.sleep(.14);im,seq=fresh(seq);im,seq=fresh(seq);assert max(abs(a-b)for a,b in zip(pixel(im,.08,.08),pixel(base,.08,.08)))>8,(key,pixel(im));im.save(out/f'{key}.png');results[key]=True
 write(profiles,{str(fixture):{'image':{}}});time.sleep(.14);im,seq=fresh(seq);im,seq=fresh(seq);assert close(pixel(im),(80,120,160));assert identity(child.pid)==native_identity;results['persist_reset_no_restart']=True;stop()
 for variant in range(8):
  direction=variant%4;effect=5+variant//4
  start({'file':str(snapshot),'effect':effect,'x':.5,'y':.5,'duration':1.2,'direction':direction,'curve':1,'zoom':.3});first,seq=fresh();began=time.monotonic();assert close(pixel(first),(230,20,180));mid=None
  while time.monotonic()-began<2.8:
   im,seq=fresh(seq);elapsed=time.monotonic()-began
   if .52<elapsed<.75 and mid is None:mid=im.copy()
   if all(not close(pixel(im,x,y),(230,20,180))for x,y in [(.15,.15),(.85,.15),(.15,.85),(.85,.85)]):break
  assert .85<elapsed<1.65,(direction,elapsed);assert mid is not None
  inside=[(.85,.5),(.15,.5),(.5,.85),(.5,.15)][direction];outside=[(.15,.5),(.85,.5),(.5,.15),(.5,.85)][direction]
  assert not close(pixel(mid,*inside),(230,20,180))and close(pixel(mid,*outside),(230,20,180)),direction
  mid.save(out/f'effect-{effect}-{direction}.png');results[f'effect_{effect}_direction_{direction}']={'duration_seconds':elapsed};stop()
finally:stop()
assert subprocess.check_output([personal,'--status'])==before;results['personal_renderer_preserved']=True;(out/'results.json').write_text(json.dumps(results,indent=2));print(json.dumps(results,indent=2))

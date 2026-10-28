#!/usr/bin/env python3
"""Static web scene: on-demand small live feed and in-process placement updates.
Uses isolated XDG files and stops only the exact child process group it created.
"""
import argparse,json,os,pathlib,signal,struct,subprocess,time
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args()
if not a.desktop:p.error('--desktop required')
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
def run(*args):return subprocess.check_output([str(x) for x in args],text=True)
def identity(pid):
 r=pathlib.Path('/proc')/str(pid);return str((r/'exe').readlink()),(r/'stat').read_text().rsplit(')',1)[1].split()[19]
def write(path,value):
 tmp=path.with_suffix('.tmp');tmp.write_text(json.dumps(value));tmp.replace(path)
installed=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=json.loads(run(installed,'--status'));layout=json.loads(run(app,'--display-layout'));rects=layout['renderer'];name=min(rects,key=lambda n:rects[n][0])
valid=next(i for i in json.loads(run(app,'--list')) if not i['reason']);plan=json.loads(run(app,'--plan',valid['id']));assets=plan[plan.index('--assets-dir')+1]
fixture=out/'fixture';fixture.mkdir(exist_ok=True);(fixture/'project.json').write_text(json.dumps({'type':'web','title':'Live feedback test','file':'index.html'}));(fixture/'index.html').write_text('''<!doctype html><html><style>html,body{margin:0;width:100%;height:100%;background:linear-gradient(to right,rgb(20,50,100),rgb(250,50,100))}</style></html>''')
feed=out/'feed';feed.mkdir(exist_ok=True);profiles=out/'wallpaper_profiles.json';live=out/'placement-live.json';request=feed/'request.json'
write(profiles,{str(fixture):{'position':{'x':0,'y':0}}});write(request,{'path':str(fixture),'output':name});(feed/'frame.bin').unlink(missing_ok=True)
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state')}
args=[str(app),'--render','--assets-dir',assets,'--fps','30','--volume','0','--no-fullscreen-pause','--scaling','stretch','--screen-span',','.join(rects),'--bg',str(fixture),'--lab-layout',json.dumps(rects),'--lab-project',str(fixture),'--lab-profile-file',str(profiles),'--lab-live-file',str(live),'--lab-feed-dir',str(feed)]
with (out/'renderer.log').open('w') as log:
 child=subprocess.Popen(args,env=env,stdout=log,stderr=log,start_new_session=True);owner=None
 try:
  owner=identity(child.pid)
  def frame(after=0):
   end=time.monotonic()+18
   while time.monotonic()<end:
    assert child.poll() is None,'renderer exited'
    write(request,{'path':str(fixture),'output':name})
    try:
     raw=(feed/'frame.bin').read_bytes();magic,pid,w,h,seq=struct.unpack('=5I',raw[:20])
     assert magic==0x574c4631 and pid==child.pid and 0<w<=640 and 0<h<=360
     if seq>after:return Image.frombytes('RGB',(w,h),raw[20:]),seq
    except FileNotFoundError:pass
    time.sleep(.05)
   raise AssertionError('live feed timed out')
  center,seq=frame();deadline=time.monotonic()+18
  while max(center.getpixel((center.width//4,center.height//2)))<20:
   assert time.monotonic()<deadline,'live frame stayed uninitialized'
   center,seq=frame(seq)
  center.save(out/'center.png');time.sleep(.2)
  write(live,{'path':str(fixture),'position':{'x':.8,'y':0}});shifted=None
  end=time.monotonic()+3
  while time.monotonic()<end:
   shifted,seq=frame(seq)
   if max(shifted.getpixel((shifted.width//4,shifted.height//2)))<3:break
  assert max(shifted.getpixel((shifted.width//4,shifted.height//2)))<3,'live offset did not reach existing renderer'
  shifted.save(out/'shifted.png')
  write(profiles,{str(fixture):{'position':{'x':0,'y':0}}});live.unlink();restored,seq=frame(seq);deadline=time.monotonic()+3
  while max(restored.getpixel((restored.width//4,restored.height//2)))<20 and time.monotonic()<deadline:restored,seq=frame(seq)
  assert max(restored.getpixel((restored.width//4,restored.height//2)))>20,'saved center did not update renderer'
  restored.save(out/'restored.png');assert identity(child.pid)==owner,'placement restarted renderer'
  old=(feed/'frame.bin').stat().st_mtime_ns;request.unlink();time.sleep(.4);assert (feed/'frame.bin').stat().st_mtime_ns==old,'feed did not stop after inspector closed'
 finally:
  if child.poll() is None and owner and identity(child.pid)==owner:
   os.killpg(child.pid,signal.SIGTERM)
   try:child.wait(timeout=8)
   except subprocess.TimeoutExpired:
    if identity(child.pid)==owner:os.killpg(child.pid,signal.SIGKILL)
    child.wait(timeout=3)
assert before==json.loads(run(installed,'--status')),'personal renderer changed'
result={'low_resolution_live_feed':True,'position_updated_without_restart':True,'saved_center_reloaded':True,'feed_stops_without_request':True,'personal_renderer_preserved':True};(out/'results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))

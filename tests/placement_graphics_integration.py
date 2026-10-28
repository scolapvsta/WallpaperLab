#!/usr/bin/env python3
"""Opt-in static HTML composition probe. Own isolated children only; no video decoding."""
import argparse,json,os,pathlib,signal,subprocess,time
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args()
if not a.desktop:p.error('--desktop required')
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
def run(*args,env=None):return subprocess.check_output([str(x) for x in args],text=True,env=env)
def identity(pid):
    r=pathlib.Path('/proc')/str(pid);return str((r/'exe').readlink()),(r/'stat').read_text().rsplit(')',1)[1].split()[19]
installed=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=json.loads(run(installed,'--status'))
layout=json.loads(run(app,'--display-layout'));rects=layout['renderer'];displays=layout['outputs'];assert len(displays)==2
valid=next(r for r in json.loads(run(app,'--list')) if not r['reason']);plan=json.loads(run(app,'--plan',valid['id']));assets=plan[plan.index('--assets-dir')+1]
fixture=out/'fixture';fixture.mkdir(exist_ok=True)
(fixture/'project.json').write_text(json.dumps({'type':'web','title':'Static placement fixture','file':'index.html'}))
(fixture/'index.html').write_text('''<!doctype html><html><style>html,body{margin:0;width:100%;height:100%;overflow:hidden;background:#111}canvas{width:100%;height:100%}</style><canvas></canvas><script>let c=document.querySelector('canvas');c.width=1024;c.height=512;let x=c.getContext('2d');let h=x.createLinearGradient(0,0,1024,0);h.addColorStop(0,'rgb(20,0,0)');h.addColorStop(1,'rgb(250,0,0)');x.fillStyle=h;x.fillRect(0,0,1024,512);x.globalCompositeOperation='lighter';let v=x.createLinearGradient(0,0,0,512);v.addColorStop(0,'rgb(0,20,0)');v.addColorStop(1,'rgb(0,250,0)');x.fillStyle=v;x.fillRect(0,0,1024,512)</script>''')
envbase=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state')};images={}
for label,x,y in [('center',0,0),('right',.4,0),('down',0,.4)]:
    d=out/label;d.mkdir(exist_ok=True)
    for old in d.glob('*.ppm'):old.unlink()
    env=envbase|{'WALLPAPER_LAB_SPAN_CAPTURE':str(d)}
    args=[str(app),'--render','--assets-dir',assets,'--fps','30','--volume','0','--no-fullscreen-pause','--scaling','stretch','--screen-span',','.join(rects),'--bg',str(fixture),'--lab-layout',json.dumps(rects),'--lab-position',json.dumps({'x':x,'y':y,'zoom':2})]
    with (d/'renderer.log').open('w') as log:
        child=subprocess.Popen(args,env=env,stdout=log,stderr=log,start_new_session=True);owner=None
        try:
            owner=identity(child.pid);deadline=time.monotonic()+25
            while not all((d/(name+'.ppm')).exists() for name in rects):
                assert child.poll() is None,'renderer exited'
                assert time.monotonic()<deadline,'capture timeout'
                time.sleep(.1)
            time.sleep(.2);images[label]={}
            for display in displays:
                im=Image.open(d/(display['name']+'.ppm'));im.load();assert im.size==(display['width'],display['height']);im.save(d/(display['name']+'.png'));images[label][display['name']]=im
        finally:
            if child.poll() is None and owner and identity(child.pid)==owner:
                os.killpg(child.pid,signal.SIGTERM)
                try:child.wait(timeout=8)
                except subprocess.TimeoutExpired:
                    if identity(child.pid)==owner:os.killpg(child.pid,signal.SIGKILL)
                    child.wait(timeout=3)
# Compare translated samples against the original whole-canvas coordinates.
left=min(r[0] for r in rects.values());top=min(r[1] for r in rects.values());width=max(r[0]+r[2] for r in rects.values())-left;height=max(r[1]+r[3] for r in rects.values())-top
errors={}
for mode,dx,dy in [('right',width*.2,0),('down',0,height*.2)]:
    samples=[];black=0
    for name,(rx,ry,rw,rh) in rects.items():
        im=images[mode][name]
        for ix in range(1,10):
            for iy in range(1,10):
                px=round(im.width*ix/10);py=round(im.height*iy/10);sx=rx+px/im.width*rw-dx;sy=ry+py/im.height*rh-dy;actual=im.getpixel((px,py))
                if sx<left or sy<top:
                    assert max(actual[:3])>3,(mode,name,'unexpected black padding',actual);black+=1;continue
                for base_name,(bx,by,bw,bh) in rects.items():
                    if bx<=sx<bx+bw and by<=sy<by+bh:
                        base=images['center'][base_name];expected=base.getpixel((min(base.width-1,round((sx-bx)/bw*base.width)),min(base.height-1,round((sy-by)/bh*base.height))))
                        samples.append(max(abs(actual[c]-expected[c]) for c in (0,1)));break
    assert samples and black>0,(mode,'no comparison samples')
    errors[mode]=sum(samples)/len(samples);assert errors[mode]<3,errors
assert json.loads(run(installed,'--status'))==before,'personal renderer changed'
result={'translation_mean_error':errors,'no_black_padding':True,'native_sizes_preserved':True,'personal_renderer_preserved':True}
(out/'results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))

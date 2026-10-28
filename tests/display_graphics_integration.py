#!/usr/bin/env python3
"""Opt-in native output test with a synthetic gradient and isolated config/state.
Never writes the personal renderer registry; verified child surfaces are removed
on completion and the previous wallpaper must remain alive throughout the test.
"""
import argparse,json,os,pathlib,signal,subprocess,time
from PIL import Image,ImageStat
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args()
if not a.desktop:p.error('--desktop required: briefly covers the desktop with a test gradient')
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
def run(*args,env=None):return subprocess.check_output([str(x) for x in args],env=env,text=True)
def identity(pid):
    root=pathlib.Path('/proc')/str(pid)
    return str((root/'exe').readlink()),(root/'stat').read_text().rsplit(')',1)[1].split()[19]
installed=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=json.loads(run(installed,'--status'))
layout=json.loads(run(app,'--display-layout'));displays=layout['outputs'];assert len(displays)==2 and all(d['measured'] for d in displays),'This seam test requires two measured displays'
assert abs(displays[0]['rect'][1]-displays[1]['rect'][1])<.01,'This test assumes top-aligned displays'
rows=json.loads(run(app,'--list'));valid=next(row for row in rows if not row['reason']);plan=json.loads(run(app,'--plan',valid['id']));assets=plan[plan.index('--assets-dir')+1]
fixture=out/'gradient';fixture.mkdir(exist_ok=True)
(fixture/'index.html').write_text("""<!doctype html><html><style>
html,body{margin:0;width:100%;height:100%;overflow:hidden;background:linear-gradient(to bottom,rgb(0,55,140),rgb(255,55,140))}
</style></html>""")
(fixture/'project.json').write_text(json.dumps({'title':'Physical calibration fixture','type':'web','file':'index.html'}))
config=out/'config';state=out/'state';config.mkdir(exist_ok=True);state.mkdir(exist_ok=True)
base_env=os.environ|{'XDG_CONFIG_HOME':str(config),'XDG_STATE_HOME':str(state)}
results={}
for mode in ['pixels','physical']:
    configured=json.loads(run(app,'--display-layout-save',mode,'top',env=base_env));directory=out/mode;directory.mkdir(exist_ok=True)
    for d in displays:(directory/(d['name']+'.ppm')).unlink(missing_ok=True)
    env=base_env|{'WALLPAPER_LAB_SPAN_CAPTURE':str(directory)}
    args=[str(app),'--render','--assets-dir',assets,'--fps','30','--volume','0','--no-fullscreen-pause','--scaling','fill','--screen-span',','.join(d['name'] for d in displays),'--bg',str(fixture),'--lab-layout',json.dumps(configured['renderer'])]
    with (directory/'renderer.log').open('w') as log:
        child=subprocess.Popen(args,env=env,stdout=log,stderr=log,start_new_session=True);owned=None
        try:
            owned=identity(child.pid);deadline=time.monotonic()+25
            while not all((directory/(d['name']+'.ppm')).exists() for d in displays):
                if child.poll() is not None:raise AssertionError('Native renderer exited: '+(directory/'renderer.log').read_text()[-2000:])
                if time.monotonic()>deadline:raise AssertionError('Native output capture timed out')
                time.sleep(.08)
            time.sleep(.15)
            images=[]
            for d in displays:
                image=Image.open(directory/(d['name']+'.ppm'));image.load();assert image.size==(d['width'],d['height'])
                image.save(directory/(d['name']+'.png'))
                assert ImageStat.Stat(image).stddev[0]>20,'Captured an uninitialized output'
                images.append(image)
            limit=min(d['mm_height'] for d in displays)*.9;mm=[10+(limit-10)*i/399 for i in range(400)]
            profiles=[]
            for d,image in zip(displays,images):
                profiles.append([image.getpixel((d['width']//2,round(y/d['mm_height']*d['height'])))[0] for y in mm])
            error=sum(abs(x-y) for x,y in zip(profiles[0],profiles[1]))/len(mm);results[mode]={'physical_seam_mean_error':error,'native_output_sizes_preserved':True}
        finally:
            if child.poll() is None and owned and identity(child.pid)==owned:
                os.killpg(child.pid,signal.SIGTERM)
                try:child.wait(timeout=8)
                except subprocess.TimeoutExpired:
                    if identity(child.pid)==owned:os.killpg(child.pid,signal.SIGKILL)
                    child.wait(timeout=3)
assert results['physical']['physical_seam_mean_error']<4,results
assert results['physical']['physical_seam_mean_error']<results['pixels']['physical_seam_mean_error']*.4,results
after=json.loads(run(installed,'--status'));assert before==after,'Personal renderer was replaced during isolated rendering test'
results['personal_renderer_preserved']=True
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))

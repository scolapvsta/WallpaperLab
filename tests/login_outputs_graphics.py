"""Opt-in two-output native login regression; uses only generated local fixtures.

The overlay preview lasts at most 60 seconds. No system login configuration or
user library is changed. Screenshots require grim and an active Hyprland session.
"""
from pathlib import Path
import argparse, importlib.util, json, os, subprocess, time
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('session',root/'assets/login_session.py');session=importlib.util.module_from_spec(spec);spec.loader.exec_module(session)
env=os.environ|dict(XDG_CONFIG_HOME=str(out/'config'),XDG_STATE_HOME=str(out/'state'),XDG_CACHE_HOME=str(out/'cache'),XDG_DATA_HOME=str(out/'data'),WALLPAPER_LAB_LIBRARY=str(out/'library'))
(out/'library').mkdir(exist_ok=True)
def run(*args):return subprocess.check_output(list(map(str,args)),env=env,text=True)
monitors=sorted(json.loads(run('hyprctl','monitors','-j')),key=lambda m:m['x'])
assert len(monitors)==2,'This regression requires exactly two connected outputs'
outputs=[m['name']for m in monitors];sources=[]
run(app,'--library-path','local',out/'library')
for name,color in [('red',(210,35,25)),('blue',(25,45,210))]:
    path=out/(name+'.png');Image.new('RGB',(640,360),color).save(path)
    sources.append(json.loads(run(app,'--import',path))[0])
run(app,'--login','set',json.dumps({'enabled':True,'source':sources[0],'mode':'single','output':outputs[0],'wallpapers':dict(zip(outputs,sources)),'ui_outputs':'primary','ui_primary':outputs[1],'language':'en'}))
results={}
def preview(label):
    prepared=Path(json.loads(run(app,'--login','prepare'))['prepared']);doc=json.loads(prepared.read_text());ready=out/(label+'-ready');ready.unlink(missing_ok=True)
    log=(out/(label+'.log')).open('w');child=subprocess.Popen([str(app),'--login-preview',str(prepared),'--outputs',str(ready)],env=env,stdout=log,stderr=log);identity=session.process_identity(child.pid)
    try:
        deadline=time.monotonic()+20
        while time.monotonic()<deadline and not ready.exists():
            assert child.poll()is None,(out/(label+'.log')).read_text()[-1200:];time.sleep(.1)
        assert ready.exists();time.sleep(1);colors=[]
        for monitor in monitors:
            path=out/(label+'-'+monitor['name']+'.png');run('grim','-o',monitor['name'],path);im=Image.open(path);colors.append(im.getpixel((im.width//10,im.height//2))[:3])
        return colors,doc
    finally:session.stop_child(child,identity);log.close()
colors,doc=preview('independent');assert colors[0][0]>150 and colors[0][2]<60 and colors[1][2]>150 and colors[1][0]<60,colors
assert len(doc['prepared_wallpapers'])==2;results['independent_projects_on_two_outputs']=True
run(app,'--login','set',json.dumps({'output_profiles':{outputs[1]:{sources[1]:{'brightness':-.5}}}}))
dim,_=preview('independent-profile');assert max(abs(a-b)for a,b in zip(colors[0],dim[0]))<8 and dim[1][2]<colors[1][2]-60,(colors,dim)
results['per_output_image_profile_isolation']=True
run(app,'--login','set',json.dumps({'mode':'mirror'}));colors,_=preview('mirror');assert all(c[0]>150 and c[2]<60 for c in colors),colors;results['mirror_uses_shared_source']=True
# A panoramic two-colour image must cross the physical output boundary once.
width=sum(m['width'] for m in monitors);height=max(m['height']for m in monitors);path=out/'panorama.png'
im=Image.new('RGB',(width,height),(210,35,25));im.paste((25,45,210),(monitors[0]['width'],0,width,height));im.save(path)
source=json.loads(run(app,'--import',path))[0];run(app,'--login','set',json.dumps({'mode':'span','source':source}))
colors,_=preview('span');assert colors[0][0]>150 and colors[0][2]<60 and colors[1][2]>150 and colors[1][0]<60,colors;results['panorama_crosses_output_boundary_once']=True
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results))

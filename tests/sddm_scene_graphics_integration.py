"""Opt-in native scene/greeter animation, rollback and owned-process checks."""
import argparse,copy,hashlib,importlib.util,json,os,shutil,subprocess,tempfile,time
from pathlib import Path
from PIL import Image,ImageChops,ImageStat

p=argparse.ArgumentParser();p.add_argument('--app',type=Path,required=True);p.add_argument('--project',type=Path,required=True);p.add_argument('--assets',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
app=a.app.resolve();project=a.project.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
spec=importlib.util.spec_from_file_location('sddm_helper',app.parent/'sddm_integration.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
screens=[dict(name='left',x=0,y=0,width=1280,height=720),dict(name='right',x=1280,y=0,width=1280,height=720)]
request=dict(config=str(out/'config'),state=str(out/'state'),roots=[str(project.parent)],renderer=str(app),engine_assets=str(a.assets.resolve()),screens=screens)
def action(name,value=''):
    return m.Integration(request|dict(action=name,value=value)).run()
def terminated(pid,start):
    try:
        stat=Path('/proc',str(pid),'stat').read_text().rsplit(')',1)[1].split()
        return stat[0]=='Z' or stat[19]!=start
    except OSError:return True
results={}
action('enable');state=action('wallpaper',str(project));assert state['ready'];state=action('display-mode','span');assert state['ready']
results['native_scene_selection_and_span_validation']=True
i=m.Integration(request);saved=i.config_file.read_bytes();bundle=Path(state['bundle'])
with tempfile.TemporaryDirectory(prefix='animated-',dir=out)as temporary:
    stage=Path(temporary);shutil.copytree(bundle,stage,dirs_exist_ok=True)
    first,second=out/'greeter-1.png',out/'greeter-2.png'
    views=''.join('ThemeMain { x:%d; y:180; width:640; height:360; labScreenRect:Qt.rect(%d,0,1280,720); labDesktopRect:Qt.rect(0,0,2560,720) }'%(n*640,n*1280)for n in range(2))
    (stage/'Main.qml').write_text('import QtQuick\nimport "."\nItem { Rectangle { id:canvas; width:1280; height:720; color:"black"; '+views+' } Timer { property int shot:0; interval:1300; running:true; repeat:true; onTriggered: { var path=shot===0?'+json.dumps(str(first))+':'+json.dumps(str(second))+'; canvas.grabToImage(function(result){result.saveToFile(path);console.log("SCENE_CAPTURE_DONE");});shot++;if(shot===2)stop(); } } }')
    with m.scene_runtime(stage,out/'renderer.log')as live, (out/'greeter.log').open('w')as log:
        frame0=hashlib.sha256(live['frame'].read_bytes()).hexdigest()
        env=os.environ|dict(QT_QPA_PLATFORM='offscreen',QT_QUICK_BACKEND='rhi',QSG_RHI_BACKEND='opengl',LIBGL_ALWAYS_SOFTWARE='1',QT_FORCE_STDERR_LOGGING='1',QT_ASSUME_STDERR_HAS_CONSOLE='1')
        greeter=subprocess.Popen([i.greeter,'--test-mode','--theme',str(stage)],env=env,stdout=log,stderr=log)
        try:
            deadline=time.monotonic()+15
            while time.monotonic()<deadline and (out/'greeter.log').read_text().count('SCENE_CAPTURE_DONE')<2:
                assert greeter.poll()is None
                time.sleep(.1)
            assert second.exists()
            results['greeter_frame_difference']=sum(ImageStat.Stat(ImageChops.difference(Image.open(first).convert('RGB'),Image.open(second).convert('RGB'))).mean)
            assert results['greeter_frame_difference']>.1,results
            assert hashlib.sha256(live['frame'].read_bytes()).hexdigest()!=frame0
            clients=json.loads(subprocess.check_output(['hyprctl','clients','-j'],text=True))
            assert not any(row['pid']==live['pid']for row in clients)
            results['shared_hidden_renderer_and_animated_greeter']=True
        finally:i.stop_child(greeter)
    assert terminated(live['pid'],live['start']);assert not live['frame'].exists();results['preview_cleanup']=True
text=(out/'greeter.log').read_text();assert text.count('WALLPAPER_LAB_SCENE_READY')==2,text;assert 'WALLPAPER_LAB_SHADER_ERROR'not in text
bad=out/'invalid-library/scene';bad.mkdir(parents=True,exist_ok=True);(bad/'project.json').write_text(json.dumps(dict(type='scene',file='scene.json')));(bad/'scene.pkg').write_bytes(b'not a scene package')
request['roots'].append(str(bad.parent))
try:action('wallpaper',str(bad));raise AssertionError('Invalid native scene accepted')
except ValueError:pass
assert i.config_file.read_bytes()==saved and action('status')['ready'];results['failed_scene_preserves_previous']=True
# Abrupt helper death must also stop its hidden renderer through PDEATHSIG.
with tempfile.TemporaryDirectory(prefix='owner-',dir=out)as temporary:
    stage=Path(temporary);shutil.copytree(bundle,stage,dirs_exist_ok=True);owner=out/'owner.json'
    code='import sys,json,time\nfrom pathlib import Path\nsys.path.insert(0,sys.argv[1])\nfrom sddm_scene import scene_runtime\nwith scene_runtime(Path(sys.argv[2]),Path(sys.argv[3])) as live:\n Path(sys.argv[4]).write_text(json.dumps({k:str(live[k]) for k in ("pid","start","frame")}))\n time.sleep(60)\n'
    parent=subprocess.Popen(['python','-c',code,str(app.parent),str(stage),str(out/'owner-renderer.log'),str(owner)])
    try:
        deadline=time.monotonic()+30
        while time.monotonic()<deadline and not owner.exists():assert parent.poll()is None;time.sleep(.1)
        record=json.loads(owner.read_text());parent.kill();parent.wait(timeout=5)
        deadline=time.monotonic()+8
        while time.monotonic()<deadline and not terminated(record['pid'],record['start']):time.sleep(.1)
        assert terminated(record['pid'],record['start']);results['abrupt_owner_death_stops_renderer']=True
        shutil.rmtree(Path(record['frame']).parent)
    finally:
        if parent.poll()is None:parent.terminate();parent.wait(timeout=5)
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))

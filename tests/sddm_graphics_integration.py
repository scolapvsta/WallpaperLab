"""Opt-in desktop check: isolated SDDM preview and launcher, synthetic media only."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import time

p=argparse.ArgumentParser()
p.add_argument('--app', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True, exist_ok=True)
library=out/'library';library.mkdir(exist_ok=True)
env=os.environ | dict(XDG_CONFIG_HOME=str(out/'config'), XDG_STATE_HOME=str(out/'state'), XDG_DATA_HOME=str(out/'data'),
                     WALLPAPER_LAB_LIBRARY=str(library), WALLPAPER_LAB_UI_STATE=str(out/'ui.json'),
                     QT_FORCE_STDERR_LOGGING='1', QT_ASSUME_STDERR_HAS_CONSOLE='1')

def cli(*args):
    result=subprocess.run([str(app), *args], env=env, capture_output=True, text=True, timeout=35)
    if result.returncode:raise RuntimeError(result.stdout+result.stderr)
    return json.loads(result.stdout)

def stop(process):
    if process.poll() is None:
        process.terminate()
        try:process.wait(timeout=4)
        except subprocess.TimeoutExpired:process.kill();process.wait()

def shot(name, pid):
    clients=json.loads(subprocess.check_output(['hyprctl','clients','-j']))
    window=next(w for w in clients if w['pid']==pid)
    x,y=window['at'];w,h=window['size']
    subprocess.run(['grim','-g',f'{x},{y} {w}x{h}',str(out/name)],check=True)

results={}
# A local image project retains its media path independently of its HTML wrapper.
for kind,ext in [('image','png'),('video','mp4'),('gif','gif')]:
    project=library/kind;project.mkdir(exist_ok=True)
    source=project/('media.'+ext)
    command=['ffmpeg','-v','error','-y','-f','lavfi','-i','testsrc2=size=1280x720:rate=24']
    command+=['-frames:v','1'] if kind=='image' else ['-t','2','-vf','scale=320:180'] if kind=='gif' else ['-t','3','-c:v','libx264','-pix_fmt','yuv420p']
    subprocess.run(command+[str(source)],check=True)
    (project/'project.json').write_text(json.dumps(dict(title='SDDM '+kind, type='web' if kind!='video' else 'video',
        file='index.html' if kind!='video' else source.name, lab_origin='local', lab_media_file=source.name)))

workshop=library/'workshop-video';workshop.mkdir(exist_ok=True)
import shutil
shutil.copyfile(library/'video/media.mp4',workshop/'video.mp4')
(workshop/'project.json').write_text(json.dumps(dict(title='Workshop fixture',type='video',file='video.mp4')))
state=cli('--sddm','enable');assert state['ready']
state=cli('--sddm','wallpaper',str(workshop));assert state['ready']
results['workshop_video_project']=True
results['activation']=True
for kind in ('image','video','gif'):
    state=cli('--sddm','wallpaper',str(library/kind));assert state['ready']
    state=cli('--sddm','preset','black_hole');assert state['config']['wallpaper']['source']==str(library/kind)
    log=open(out/(kind+'-greeter.log'),'w')
    proc=subprocess.Popen(['sddm-greeter-qt6','--test-mode','--theme',state['bundle']],env=env,stdout=log,stderr=log)
    try:
        time.sleep(4);assert proc.poll() is None
        shot(kind+'-greeter.png',proc.pid)
        if kind in ('video','gif'):
            time.sleep(.7);shot(kind+'-greeter-second.png',proc.pid)
            from PIL import Image, ImageChops, ImageStat
            first=Image.open(out/(kind+'-greeter.png')).convert('RGB')
            second=Image.open(out/(kind+'-greeter-second.png')).convert('RGB')
            assert sum(ImageStat.Stat(ImageChops.difference(first,second)).mean)>0.5
            results[kind+'_frame_changes']=True
    finally:stop(proc);log.close()
    text=(out/(kind+'-greeter.log')).read_text()
    assert 'WALLPAPER_LAB_GREETER_READY' in text
    if kind=='video':assert 'Video started.' in text
    results[kind+'_greeter']=True

for enabled in (True,False):
    if not enabled:cli('--sddm','disable')
    with (out/f'launcher-{enabled}.log').open('w') as log:
        proc=subprocess.Popen([str(app),'--sddm-settings'],env=env,stdout=log,stderr=log)
        try:
            deadline=time.monotonic()+20
            while time.monotonic()<deadline:
                assert proc.poll() is None
                try:
                    style=json.loads((out/'ui.json.style').read_text())
                    state=json.loads((out/'ui.json').read_text())
                    if style['sddm_ready']==enabled and state['page']==(4 if enabled else 2) and (style['integration_tab']==1 if enabled else style['settings_tab']==7):break
                except (OSError, ValueError, KeyError):pass
                time.sleep(.2)
            else:raise AssertionError('SDDM tab state did not match activation')
            time.sleep(1);shot(f'launcher-{enabled}.png',proc.pid)
        finally:stop(proc)
    results['launcher_enabled' if enabled else 'launcher_disabled']=True
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))

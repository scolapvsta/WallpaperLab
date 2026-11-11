#!/usr/bin/env python3
"""Exercise the real controller with isolated XDG directories and a fake daemon."""
import argparse
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time

p = argparse.ArgumentParser()
p.add_argument('--app', required=True)
a = p.parse_args()
app = str(Path(a.app).resolve())
with tempfile.TemporaryDirectory(prefix='wallpaper-lab-awww-') as tmp:
    root = Path(tmp)
    env = os.environ.copy()
    env.update(XDG_CONFIG_HOME=str(root/'config'), XDG_STATE_HOME=str(root/'state'),
               XDG_DATA_HOME=str(root/'data'), WALLPAPER_LAB_LIBRARY=str(root/'library'),
               AWWW_TEST_ROOT=tmp, AWWW_TEST_PYTHON=sys.executable, PATH=str(root/'bin')+':'+env['PATH'])
    (root/'bin').mkdir()
    (root/'bin/awww').write_text('''#!/usr/bin/env python3
import json,os,sys
from pathlib import Path
r=Path(os.environ['AWWW_TEST_ROOT']);a=sys.argv[1:];f=r/'daemon.json'
s=json.loads(f.read_text()) if f.exists() else {}
if a[0]=='img':
 if (r/'fail').exists():sys.exit(4)
 for o in a[a.index('--outputs')+1].split(','):s[o]=a[1]
 f.write_text(json.dumps(s));(r/'args.json').write_text(json.dumps(a))
elif a[0]=='query':
 for o,i in s.items():print(f': {o}: 1920x1080, scale: 1, currently displaying: image: {i}')
elif a[0]=='clear':
 for o in a[a.index('--outputs')+1].split(','):s.pop(o,None)
 f.write_text(json.dumps(s))
''')
    (root/'bin/hyprctl').write_text('''#!/usr/bin/env python3
import json,sys
if sys.argv[1]=='cursorpos':print('{"x":123,"y":456}')
elif sys.argv[1]=='monitors':print(json.dumps([{'name':'DP-1','description':'Test monitor','width':1920,'height':1080,'scale':1,'x':0,'y':0,'focused':True}]))
else:print('ok')
''')
    (root/'bin/python3').write_text('''#!/usr/bin/python3
import json,os,sys
from pathlib import Path
if len(sys.argv)>2 and Path(sys.argv[1]).name=='caelestia_cli.py':
 r=Path(os.environ['AWWW_TEST_ROOT']);(r/'palette-source.txt').write_text(sys.argv[2])
 palette={'name':'dynamic','flavour':'hard','mode':'dark','variant':'tonalspot','colours':{'primary':'123456','surface':'112233'}}
 target=Path(os.environ['XDG_STATE_HOME'])/'caelestia/scheme.json';target.parent.mkdir(parents=True,exist_ok=True);target.write_text(json.dumps(palette));print(json.dumps(palette))
else:os.execv(os.environ['AWWW_TEST_PYTHON'],[os.environ['AWWW_TEST_PYTHON']]+sys.argv[1:])
''')
    for file in (root/'bin').iterdir():file.chmod(0o755)
    config = root/'config/wallpaper-lab'
    config.mkdir(parents=True)
    prefs = {'mode':'single','screen':0,'wallpaper_backend':1,'awww_position':0,
             'caelestia_enabled':True,'transition_type':7}
    def save(): (config/'settings.json').write_text(json.dumps(prefs))
    def run(*args, ok=True):
        result=subprocess.run([app,*map(str,args)],env=env,capture_output=True,text=True,timeout=45)
        if ok:assert result.returncode==0,(args,result.stderr)
        return result
    def status():return json.loads(run('--status').stdout)
    def live(pid):
        try:return Path(f'/proc/{pid}/stat').read_text().rsplit(')',1)[1].split()[0]!='Z'
        except FileNotFoundError:return False
    for n in range(2):
        project=root/f'library/image{n}';project.mkdir(parents=True)
        (project/'project.json').write_text(json.dumps({'title':f'Image {n}','type':'web','file':'index.html','lab_origin':'local','lab_media_kind':'image','lab_width':1920,'lab_height':1080}))
        (project/'index.html').write_text('<img src="media.png">')
        (project/'media.png').write_bytes(b'fixture handled by fake daemon')
    save()
    (root/'config/caelestia').mkdir()
    hook='awww img "$WALLPAPER_PATH" --transition-step 2 --transition-type outer --transition-duration 2 --transition-fps 144 --transition-pos $(hyprctl cursorpos | tr -d \' \') --invert-y && printf \'%s\' "$WALLPAPER_PATH"'
    (root/'config/caelestia/cli.json').write_text(json.dumps({'wallpaper':{'postHook':hook}}))
    imported=json.loads(run('--import-awww').stdout)
    assert imported['awww_fps']==144 and imported['awww_duration']==2000
    assert (root/'config/caelestia/cli.json').read_text()==json.dumps({'wallpaper':{'postHook':hook}})
    assert list((root/'state/wallpaper-lab/backups').glob('awww-import-*/settings.json'))
    try:
        run('--apply','image0');first=status();assert first['alive'] and first['backend']=='awww'
        args=json.loads((root/'args.json').read_text())
        assert args[args.index('--transition-pos')+1]=='123,456' and '--invert-y' in args
        assert (root/'palette-source.txt').read_text()==str(root/'library/image0/media.png')
        assert json.loads((root/'state/caelestia/scheme.json').read_text())['variant']=='tonalspot'
        assert (root/'state/wallpaper-lab/post-hook.log').read_text()==str(root/'library/image0/media.png')
        run('--favorite','image0');run('--favorite','image1');run('--next')
        second=status();assert second['alive'] and second['path'].endswith('/image1')
        assert not live(first['pid'])
        (root/'fail').touch();failed=run('--apply','image0',ok=False)
        assert failed.returncode!=0 and status()['pid']==second['pid'] and status()['alive']
        (root/'fail').unlink()
        current=json.loads((config/'settings.json').read_text());current['wallpaper_post_hook']='echo hook-failed >&2; exit 7'
        (config/'settings.json').write_text(json.dumps(current))
        run('--apply','image0');third=status();assert third['alive']
        assert not json.loads((root/'state/wallpaper-lab/post-hook-status.json').read_text())['ok']
        assert 'hook-failed' in (root/'state/wallpaper-lab/post-hook.log').read_text()
        run('--restore');restored=status();assert restored['alive'] and restored['backend']=='awww'
        assert not live(third['pid'])
        run('--stop');assert not live(restored['pid']) and not json.loads((root/'daemon.json').read_text())
        # Forged state must never signal another executable or clear its image.
        other=subprocess.Popen(['sleep','60'],start_new_session=True)
        try:
            started=Path(f'/proc/{other.pid}/stat').read_text().rsplit(')',1)[1].split()[19]
            (root/'daemon.json').write_text(json.dumps({'DP-1':'/foreign.png'}))
            (root/'state/wallpaper-lab/running.json').write_text(json.dumps({'pid':other.pid,'start':started,'backend':'awww','image':'/foreign.png','awww_outputs':'DP-1'}))
            run('--stop');assert other.poll() is None and json.loads((root/'daemon.json').read_text())['DP-1']=='/foreign.png'
        finally:other.terminate();other.wait()
        run('--apply','image0');fourth=status()
        # An external wallpaper change ends only our supervisor, preserving the daemon image.
        (root/'daemon.json').write_text(json.dumps({'DP-1':'/external.png'}))
        time.sleep(1.5);assert not status()['alive'] and not live(fourth['pid'])
        run('--stop');assert json.loads((root/'daemon.json').read_text())['DP-1']=='/external.png'
        # Unsupported import fails atomically.
        before=(config/'settings.json').read_bytes()
        (root/'config/caelestia/cli.json').write_text(json.dumps({'wallpaper':{'postHook':'false'}}))
        assert run('--import-awww',ok=False).returncode!=0
        assert (config/'settings.json').read_bytes()==before
    finally:run('--stop',ok=False)
print('awww lifecycle: import, handoff, carousel, restore, failed launch, post-hook failure, Stop and unrelated process/image preservation passed')

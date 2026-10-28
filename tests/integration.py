#!/usr/bin/env python3
"""Run after installation. --desktop explicitly exercises live scene replacement."""
import argparse
import json
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time

p=argparse.ArgumentParser()
p.add_argument('--app',default=str(Path.home()/'.local/bin/wallpaper-lab'))
p.add_argument('--scene',default='3144757256')
p.add_argument('--desktop',action='store_true')
p.add_argument('--output',required=True)
a=p.parse_args();results={}
def run(*args,env=None,ok=True):
    r=subprocess.run([a.app,*args],env=env,capture_output=True,text=True,timeout=45)
    if ok and r.returncode:raise RuntimeError(r.stderr)
    return r
rows=json.loads(run('--list').stdout)
results['catalog_count']=len(rows)
assert any(x['id']==a.scene and x['type']=='scene' for x in rows)
previews=[a.scene]+[x['id'] for x in rows if x['type']=='video' and not x['reason']][:1]
results['native_previews']=[]
for item in previews:
    process=json.loads(run('--preview',item).stdout)
    try:
        assert Path(f"/proc/{process['pid']}/exe").resolve().name=='wallpaper-lab'
        time.sleep(1)
        assert Path(f"/proc/{process['pid']}").exists()
        results['native_previews'].append('passed')
    finally:
        try:
            proc=Path(f"/proc/{process['pid']}")
            start=proc.joinpath('stat').read_text().rsplit(')',1)[1].split()[19]
            if start==process['start'] and proc.joinpath('exe').resolve().name=='wallpaper-lab':
                os.killpg(process['pid'],signal.SIGTERM)
        except (ProcessLookupError, FileNotFoundError):pass
with tempfile.TemporaryDirectory(prefix='wallpaper-lab-process-test-') as tmp:
    env=os.environ.copy();env['XDG_STATE_HOME']=tmp
    state=Path(tmp)/'wallpaper-lab';state.mkdir()
    other=subprocess.Popen(['sleep','60'])
    try:
        stat=Path(f'/proc/{other.pid}/stat').read_text().rsplit(')',1)[1].split()
        (state/'running.json').write_text(json.dumps({'pid':other.pid,'start':stat[19]}))
        run('--stop',env=env)
        assert other.poll() is None
        results['foreign_process_preserved']=True
    finally:other.terminate();other.wait()
if a.desktop:
    before=json.loads(run('--status').stdout)
    run('--apply',a.scene)
    first=json.loads(run('--status').stdout);assert first['alive']
    run('--apply',a.scene)
    second=json.loads(run('--status').stdout);assert second['alive'] and first['pid']!=second['pid']
    assert not Path(f"/proc/{first['pid']}/exe").exists()
    results['scene_replacement']=True
    with tempfile.TemporaryDirectory(prefix='wallpaper-lab-bad-scene-') as tmp:
        fixture=Path(tmp)/'broken';fixture.mkdir()
        (fixture/'project.json').write_text(json.dumps({'type':'scene','title':'Broken fixture','file':'scene.json'}))
        (fixture/'scene.json').write_text('{invalid JSON')
        env=os.environ.copy();env['WALLPAPER_LAB_LIBRARY']=tmp
        failed=run('--apply','broken',env=env,ok=False)
        assert failed.returncode!=0
        after=json.loads(run('--status').stdout)
        assert after['alive'] and after['pid']==second['pid']
        results['failed_scene_preserves_previous']=True
    if before.get('path') and before['path']!=second['path']:run('--apply',before['path'])
Path(a.output).write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))

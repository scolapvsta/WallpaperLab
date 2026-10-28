#!/usr/bin/env python3
"""Explicit desktop test. Temporarily cycles scene/video; restores config and wallpaper."""
import argparse, fcntl, json, os, pathlib, subprocess, time
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--output',required=True);p.add_argument('--global-shortcut',action='store_true');p.add_argument('--app',default=str(pathlib.Path.home()/'.local/bin/wallpaper-lab'));a=p.parse_args()
if not a.desktop:p.error('--desktop is required: this test changes the live wallpaper')
def run(*args,ok=True):
    r=subprocess.run([a.app,*args],capture_output=True,text=True,timeout=45)
    assert (r.returncode==0)==ok,(args,r.stderr)
    return r

def status():return json.loads(run('--status').stdout)
config=pathlib.Path(os.environ.get('XDG_CONFIG_HOME',pathlib.Path.home()/'.config'))/'wallpaper-lab'
state=pathlib.Path(os.environ.get('XDG_STATE_HOME',pathlib.Path.home()/'.local/state'))/'wallpaper-lab'
carousel=config/'carousel.json';settings=config/'settings.json'
original_list=carousel.read_bytes() if carousel.exists() else None
original_settings=settings.read_bytes();before=status();assert before['alive']
rows=json.loads(run('--list').stdout);video=next(r for r in rows if r['type']=='video' and not r['reason'])
output=pathlib.Path(a.output);output.parent.mkdir(parents=True,exist_ok=True)
output.with_suffix('.backup.json').write_text(json.dumps({'settings':json.loads(original_settings),'carousel':json.loads(original_list) if original_list else None}))
results={}
try:
    carousel.write_text('[]\n')
    run('--next',ok=False);assert status()['pid']==before['pid'];results['empty_preserves_renderer']=True
    carousel.write_text(json.dumps([before['path'],video['path']]))
    with (state/'controller.lock').open('a') as lock:
        fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        run('--next',ok=False);assert status()['pid']==before['pid']
    results['busy_preserves_renderer']=True
    run('--next');first=status();assert first['alive'] and first['path']==video['path'] and first['pid']!=before['pid']
    run('--next');second=status();assert second['alive'] and second['path']==before['path'] and second['pid']!=first['pid']
    assert not pathlib.Path(f"/proc/{first['pid']}/exe").exists()
    run('--previous');assert status()['path']==video['path'];results['next_wrap_previous']=True
    # A missing library entry is retained on disk but skipped safely.
    carousel.write_text(json.dumps(['/missing/workshop/project',before['path'],video['path']]))
    run('--next');assert status()['path']==before['path'];results['missing_entry_skipped']=True
    if a.global_shortcut:
        old=status()['pid']
        subprocess.run(['ydotool','key','125:1','15:1','15:0','125:0'],check=True)
        for _ in range(100):
            current=status()
            if current['pid']!=old and current.get('path')==video['path'] and current['alive']:break
            time.sleep(.2)
        else:raise AssertionError('Global Super+Tab did not apply the next wallpaper')
        results['global_super_tab']=True
finally:
    settings.write_bytes(original_settings)
    if original_list is None:carousel.unlink(missing_ok=True)
    else:carousel.write_bytes(original_list)
    if status().get('path')!=before['path']:run('--restore')
    output.write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))

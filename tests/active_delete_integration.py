#!/usr/bin/env python3
"""Opt-in lifecycle test: disposable projects, isolated config/state/system Trash."""
import argparse,json,pathlib,subprocess,os,shutil
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);root=out/'library';root.mkdir(exist_ok=True);data=out/'data';data.mkdir(exist_ok=True)
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'XDG_DATA_HOME':str(data),'WALLPAPER_LAB_LIBRARY':str(root),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','deps','lib'])}
log=(out/'commands.log').open('w');result={}
def run(*args,ok=True):
 r=subprocess.run([str(app),*map(str,args)],env=env,capture_output=True,text=True,timeout=45);log.write(json.dumps({'args':args,'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr})+'\n');log.flush()
 if ok:assert r.returncode==0,(args,r.stderr)
 return r
def project(n,broken=False):
 d=root/str(n);d.mkdir(exist_ok=True)
 (d/'project.json').write_text(json.dumps({'title':str(n),'type':'scene'if broken else'web','file':'scene.json'if broken else'index.html'}));(d/('scene.pkg'if broken else'index.html')).write_text('invalid package'if broken else'<!doctype html><style>html{background:#143c51}</style>');return d
def settings():return json.loads((config/'settings.json').read_text())
def status():return json.loads(run('--status').stdout)
def favorites(ids):(config/'carousel.json').write_text(json.dumps([str(root/str(i))for i in ids]))
try:
 (config/'settings.json').write_text(json.dumps({'mode':'single','screen':0,'fps':15,'muted':True,'fullscreen_policy':0}))
 for i in (1,2,3):project(i)
 favorites([1,3,2]);run('--apply','1');old=status();assert old['alive'];run('--delete','1');now=status();assert now['alive']and now['path']==str(root/'3');assert not(root/'1').exists();assert str(root/'1')not in json.loads((config/'carousel.json').read_text());result['active_to_first_other_favorite']=True
 favorites([]);run('--delete','3');assert status()['path']==str(root/'2');result['empty_carousel_uses_first_library']=True
 run('--delete','2');assert not status()['alive'];result['last_wallpaper_safe_stop']=True;assert len(list((data/'Trash/info').glob('*.trashinfo')))==3;result['recoverable_system_trash']=True
 project(1);project(2,True);favorites([2]);run('--apply','1');old=status();r=run('--delete','1',ok=False);assert r.returncode!=0;assert(root/'1/project.json').exists();now=status();assert now['alive']and now['pid']==old['pid']and now['start']==old['start'];result['failed_replacement_preserves_active_and_files']=True
 shutil.rmtree(root/'2');favorites([]);fake=out/'bin';fake.mkdir(exist_ok=True);(fake/'gio').write_text('#!/bin/sh\nexit 1\n');(fake/'gio').chmod(0o755);env['PATH']=str(fake)+os.pathsep+os.environ['PATH'];r=run('--delete','1',ok=False);assert r.returncode!=0;assert(root/'1/project.json').exists();now=status();assert now['alive']and now['path']==str(root/'1');result['failed_trash_restores_last_wallpaper']=True
finally:
 run('--stop',ok=False);log.close();(out/'results.json').write_text(json.dumps(result,indent=2))
print(json.dumps(result,indent=2))

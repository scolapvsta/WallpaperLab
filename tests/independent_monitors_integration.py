#!/usr/bin/env python3
import argparse,pathlib,os,subprocess,json,time
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);root=out/'library';conf=out/'config/wallpaper-lab';conf.mkdir(parents=True,exist_ok=True)
for n,c in [(1,'red'),(2,'blue'),(3,'green')]:
 d=root/str(n);d.mkdir(parents=True,exist_ok=True);(d/'project.json').write_text(json.dumps({'title':str(n),'type':'web','file':'index.html'}));(d/'index.html').write_text('<style>html{background:'+c+'}</style>')
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'WALLPAPER_LAB_LIBRARY':str(root),'WALLPAPER_LAB_TRANSITION_TRACE':str(out/'transitions.log'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])}
(conf/'settings.json').write_text(json.dumps({'mode':'single','screen':0,'fps':30,'muted':True,'fullscreen_policy':0,'transition_type':7}))
def run(*args):return subprocess.check_output([str(app),*map(str,args)],env=env,text=True,stderr=subprocess.STDOUT,timeout=40)
def prefs(screen):
 d=json.loads((conf/'settings.json').read_text());d['screen']=screen;(conf/'settings.json').write_text(json.dumps(d))
def status():return json.loads(run('--status'))
try:
 run('--apply','1');prefs(1);run('--apply','2');before=status();assert len(before['outputs'])==2;unchanged=next(v for v in before['outputs'].values()if v['path']==str(root/'2'));prefs(0);run('--apply','3');time.sleep(1.2);after=status();assert next(v for v in after['outputs'].values()if v['path']==str(root/'2'))==unchanged;assert pathlib.Path('/proc',str(unchanged['pid'])).exists();assert 'started 7'in(out/'transitions.log').read_text()and'completed'in(out/'transitions.log').read_text();run('--stop');run('--restore');restored=status();assert len(restored['outputs'])==2;assert set(v['path']for v in restored['outputs'].values())=={str(root/'2'),str(root/'3')};result={'two_independent_outputs':True,'other_pid_unchanged':True,'iris_transition_gl_started_and_completed':True,'restore_both_outputs':True};(out/'results.json').write_text(json.dumps(result,indent=2));print(result)
finally:run('--stop')

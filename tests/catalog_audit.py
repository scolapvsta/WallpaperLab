#!/usr/bin/env python3
"""Bounded, sequential native launch audit; never replaces/stops the user's renderer.
--desktop authorizes temporary wallpaper surfaces for single/mirror/span. Readiness
and short survival are smoke checks, not assertions of complete visual fidelity.
"""
import argparse,json,os,pathlib,signal,subprocess,time
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--desktop',action='store_true');p.add_argument('--ids',nargs='*');p.add_argument('--modes',nargs='+',default=['single','mirror','span']);a=p.parse_args()
if not a.desktop:p.error('--desktop is required for real output tests')
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
env=os.environ|{'LD_LIBRARY_PATH':':'.join(str(app.parent/x) for x in ['.','deps','lib'])+':'+os.environ.get('LD_LIBRARY_PATH',''),'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state')}
config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True)
installed=pathlib.Path.home()/'.local/bin/wallpaper-lab'
def command(*args):return subprocess.check_output([str(app),*args],text=True,env=env,timeout=25)
def identity(pid):
 q=pathlib.Path('/proc')/str(pid);return(q.joinpath('stat').read_text().rsplit(')',1)[1].split()[19],str(q.joinpath('exe').resolve()))
def stop(child,expected):
 if child.poll() is None:
  try:
   if identity(child.pid)==expected:os.killpg(child.pid,signal.SIGTERM)
  except FileNotFoundError:pass
  try:child.wait(4)
  except subprocess.TimeoutExpired:
   if identity(child.pid)==expected:os.killpg(child.pid,signal.SIGKILL)
   child.wait(4)
before=subprocess.check_output([str(installed),'--status'],text=True)
rows=json.loads(command('--list'));results=[]
try:
 for row in rows:
  if a.ids and row['id'] not in a.ids:continue
  for mode in a.modes:
   record={'id':row['id'],'title':row['title'],'type':row['type'],'mode':mode};name=row['id']+'-'+mode;ready=out/(name+'.ready');ready.unlink(missing_ok=True);log=out/(name+'.log')
   (config/'settings.json').write_text(json.dumps({'mode':mode,'screen':0,'fps':30,'muted':True,'disable_mouse':False,'disable_particles':False,'disable_parallax':False}))
   if row['reason']:record['error']=row['reason']
   else:
    child=None;expected=None
    try:
     args=json.loads(command('--plan',row['id']));args[2:2]=['--ready-file',str(ready),'--no-fullscreen-pause']
     with log.open('w') as f:child=subprocess.Popen(args,cwd=app.parent,env=env,stdout=f,stderr=f,start_new_session=True)
     expected=identity(child.pid);start=time.monotonic();budget=15
     while child.poll() is None and not ready.exists() and time.monotonic()-start<budget:time.sleep(.08)
     if ready.exists():
      time.sleep(.6);record['ready_seconds']=round(time.monotonic()-start-.6,3);record['passed']=child.poll() is None
     else:record['passed']=False
     record['exit']=child.poll()
     if not record['passed']:record['error']=log.read_text(errors='replace')[-2400:]
    except Exception as e:record['error']=str(e);record['passed']=False
    finally:
     if child is not None and expected:stop(child,expected)
   results.append(record);(out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
   print(f"{len(results):3} {'PASS' if record.get('passed') else 'FAIL'} {mode:6} {row['title']}",flush=True)
finally:
 after=subprocess.check_output([str(installed),'--status'],text=True);(out/'desktop-before.json').write_text(before);(out/'desktop-after.json').write_text(after)
 print(json.dumps({'checks':len(results),'passed':sum(x.get('passed',False) for x in results),'same_user_renderer':json.loads(before)==json.loads(after)}),flush=True)

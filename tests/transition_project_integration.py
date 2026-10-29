#!/usr/bin/env python3
"""Opt-in real-project controller handoffs on private XDG state; never copies assets."""
import argparse,json,pathlib,os,subprocess,time,threading,struct
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--projects',nargs='+',required=True);p.add_argument('--effect',type=int,default=8);p.add_argument('--duration',type=int,default=700);p.add_argument('--curve',type=int,default=0);p.add_argument('--direction',type=int,default=0);p.add_argument('--fps',type=int,default=60);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);env=os.environ|{f'XDG_{n}_HOME':str(out/n.lower())for n in ['CONFIG','STATE','DATA','CACHE']};env['LD_LIBRARY_PATH']=':'.join(str(app.parent/x)for x in ['.','lib','deps']);env['WALLPAPER_LAB_TRANSITION_TRACE']=str(out/'trace.log');personal=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=subprocess.check_output([personal,'--status']);log=open(out/'controller.log','w');config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);results=[];worker=None;done=threading.Event();active=[None];writes=[0]
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,stderr=log,text=True)
def status():return json.loads(run(app,'--status'))
def inspector():
 while not done.is_set():
  try:
   current=active[0]
   if current:
    args=(pathlib.Path('/proc')/str(current['pid'])/'cmdline').read_bytes().decode().split('\0');feed=pathlib.Path(args[args.index('--lab-feed-dir')+1]);feed.mkdir(parents=True,exist_ok=True);tmp=feed/'inspector-test.tmp';tmp.write_text(json.dumps({'path':current['path'],'output':selected_output[0]}));tmp.replace(feed/'request.json');writes[0]+=1
  except(FileNotFoundError,ValueError,ProcessLookupError):pass
  time.sleep(.02)
monitors={m['name']:m for m in json.loads(subprocess.check_output(['hyprctl','monitors','-j'],text=True))};selected_output=[min(monitors,key=lambda n:monitors[n]['x'])]
try:
 worker=threading.Thread(target=inspector);worker.start()
 for mode in ['single','mirror','span']:
  run(app,'--stop');active[0]=None;settings={'mode':mode,'screen':0,'fps':a.fps,'muted':True,'disable_audio_processing':True,'fullscreen_policy':0,'transition_type':a.effect,'transition_duration':a.duration,'transition_curve':a.curve,'transition_direction':a.direction};(config/'settings.json').write_text(json.dumps(settings));run(app,'--apply',a.projects[0]);old=status();active[0]=old
  for project in a.projects[1:]+a.projects[:1]:
   prior_trace=(out/'trace.log').read_text()if(out/'trace.log').exists()else'';run(app,'--apply',project);current=status();active[0]=current;assert current['pid']!=old['pid'];assert not pathlib.Path('/proc',str(old['pid'])).exists(),'old renderer survived handoff';args=(pathlib.Path('/proc')/str(current['pid'])/'cmdline').read_bytes().decode().split('\0');transition=json.loads(args[args.index('--lab-transition')+1]);expected=[selected_output[0]]if mode=='single'else list(monitors);assert set(transition)==set(expected),(mode,transition.keys());gates={entry['start_file']for entry in transition.values()};assert len(gates)==1
   dimensions={}
   for name,entry in transition.items():
    raw=pathlib.Path(entry['file']).read_bytes();magic,pid,w,h,seq=struct.unpack('=5I',raw[:20]);assert magic==0x574c4631 and pid==old['pid']and len(raw)==20+w*h*3;assert(w,h)==(monitors[name]['width'],monitors[name]['height']),(name,w,h);dimensions[name]=[w,h]
   deadline=time.monotonic()+6
   while time.monotonic()<deadline:
    trace=(out/'trace.log').read_text()[len(prior_trace):]if(out/'trace.log').exists()else''
    if all(name+' started '+str(a.effect)in trace and name+' completed'in trace for name in expected):break
    time.sleep(.05)
   else:raise AssertionError(('transition incomplete',mode,trace))
   samples=[json.loads(line)for line in trace.splitlines()if line.startswith('{')];initial=[s for s in samples if s['progress']==0];assert set(s['output']for s in initial)==set(expected),('missing initial pixel samples',mode,initial);assert all(s['rgba'][3]==255 and max(abs(a-b)for a,b in zip(s['rgba'],s['old_rgb']))<=3 for s in initial),('initial snapshot differs or is transparent',initial);assert all(s['rgba'][3]==255 for s in samples),('transparent compositor frame',samples)
   timings=[{'output':s['output'],'frames':s.get('frames'),'max_gap_ms':s.get('max_gap_ms'),'elapsed_ms':s.get('elapsed_ms')}for s in samples if s['progress']==1];assert all(s['frames']>=max(2,a.duration//70)and s['elapsed_ms']<a.duration+160 for s in timings),('transition stalled',timings)
   results.append({'timings':timings,'opaque_rgba_and_initial_snapshot_match':True,'mode':mode,'title':current['title'],'outputs':expected,'native_snapshot_sizes':dimensions,'shared_start_gate':True,'old_retired':True});old=current
finally:
 done.set()
 if worker:worker.join(3)
 run(app,'--stop');log.close();(out/'results.json').write_text(json.dumps({'handoffs':results,'inspector_writes':writes[0]},indent=2,ensure_ascii=False))
assert before==subprocess.check_output([personal,'--status']),'personal renderer changed';assert writes[0]>100;print(json.dumps({'real_project_handoffs':len(results),'modes':3,'inspector_writes':writes[0],'personal_renderer_preserved':True}))

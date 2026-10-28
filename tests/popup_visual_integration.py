#!/usr/bin/env python3
"""Live graphical checks with isolated XDG state; no keyboard injection or rendering."""
import argparse,json,os,pathlib,subprocess,tempfile,time

p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--capture-canvas',action='store_true');p.add_argument('--output',required=True);p.add_argument('--app',type=pathlib.Path,default=pathlib.Path.home()/'.local/bin/wallpaper-lab');args=p.parse_args()
if not args.desktop:p.error('--desktop required: briefly opens popup windows')
out=pathlib.Path(args.output);out.mkdir(parents=True,exist_ok=True)
app=args.app.resolve()
def run(*cmd):return subprocess.check_output([str(x) for x in cmd],text=True)
def windows():return [w for w in json.loads(run('hyprctl','clients','-j')) if w['title']=='Wallpaper Lab Carousel']
def wait(fn,timeout=5):
    until=time.monotonic()+timeout
    while time.monotonic()<until:
        value=fn()
        if value:return value
        time.sleep(.01)
    raise AssertionError('Popup condition timed out')
def close(w):
    proc=pathlib.Path('/proc')/str(w['pid'])
    if not proc.exists():return
    identity=(os.readlink(proc/'exe'),(proc/'stat').read_text().rsplit(')',1)[1].split()[19])
    assert identity[0]==str(app if args.app != pathlib.Path.home()/'.local/bin/wallpaper-lab' else pathlib.Path.home()/'.local/share/wallpaper-lab/runtime/wallpaper-lab')
    assert identity==(os.readlink(proc/'exe'),(proc/'stat').read_text().rsplit(')',1)[1].split()[19])
    run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.close({{window="pid:{w["pid"]}"}}))')
def shot(w,name):
    x,y=w['at'];width,height=w['size'];run('grim','-g',f'{x},{y} {width}x{height}',out/(name+'.png'))

valid=[r['path'] for r in json.loads(run(app,'--list')) if not r['reason']]
personal=json.loads((pathlib.Path.home()/'.config/wallpaper-lab/carousel.json').read_text())
favorites=[path for path in personal if path in valid][:4]
assert favorites
workers=[];owned=set();results={}
with tempfile.TemporaryDirectory(prefix='wallpaper-motion-') as tmp, (out/'test.log').open('w') as log:
    root=pathlib.Path(tmp);config=root/'config/wallpaper-lab';config.mkdir(parents=True)
    env=os.environ|{'XDG_CONFIG_HOME':str(root/'config'),'XDG_STATE_HOME':str(root/'state')}
    def event(session,steps,action):
        child_env=env.copy()
        if args.capture_canvas:
            capture=(out/f'canvas-{session}').resolve();capture.mkdir(exist_ok=True)
            child_env['WALLPAPER_LAB_POPUP_CAPTURE']=str(capture)
        worker=subprocess.Popen([str(app),'--carousel-event','visual',str(session),str(steps),action],env=child_env,stdout=log,stderr=log)
        workers.append(worker);owned.add(worker.pid);return worker
    def state():return json.loads((root/'state/wallpaper-lab/popup/event.json').read_text())
    try:
        (config/'carousel.json').write_text(json.dumps(favorites))
        for mode in range(4):
            (config/'settings.json').write_text(json.dumps({'carousel_layout':mode}))
            worker=event(mode+1,1,'step')
            w=wait(lambda:next((w for w in windows() if w['pid']==worker.pid),None))
            shot(w,f'entry-{mode}');time.sleep(.5);shot(w,f'selected-first-{mode}')
            event(mode+1,2,'step').wait(timeout=3);time.sleep(.04);shot(w,f'zoom-mid-{mode}')
            time.sleep(.5);shot(w,f'selected-next-{mode}')
            if args.capture_canvas:wait(lambda:(out/f'canvas-{mode+1}/step-2-2.png').exists())
            event(mode+1,5,'step').wait(timeout=3);time.sleep(.15);shot(w,f'forward-wrap-mid-{mode}')
            time.sleep(.55);shot(w,f'forward-wrap-settled-{mode}')
            # Reproduce held Super+Tab / fast repeat without committing a wallpaper.
            for step in range(6,38):
                event(mode+1,step,'step').wait(timeout=3)
                if step in (12,22,37):shot(w,f'rapid-{mode}-{step}')
            time.sleep(.55);shot(w,f'rapid-settled-{mode}')
            latest=next(client for client in windows() if client['pid']==w['pid'])
            assert latest['size']==w['size'],'rapid repeat resized the native carousel surface'
            close(w);time.sleep(.04);shot(w,f'exit-{mode}')
            worker.wait(timeout=3);assert state()['done']
            results[str(mode)]={'size':w['size'],'cancelled':True}
        # Empty carousel makes commit harmless while exercising the fade handoff.
        (config/'carousel.json').write_text('[]')
        first=event(5,1,'step');wait(lambda:next((w for w in windows() if w['pid']==first.pid),None));time.sleep(.2)
        event(5,1,'commit').wait(timeout=3);time.sleep(.03)
        event(6,1,'step').wait(timeout=3)
        second=wait(lambda:next((w for w in windows() if w['pid']!=first.pid and pathlib.Path('/proc',str(w['pid']),'environ').exists() and b'XDG_STATE_HOME='+str(root/'state').encode()+b'\0' in pathlib.Path('/proc',str(w['pid']),'environ').read_bytes()),None))
        owned.add(second['pid']);time.sleep(.2);assert state()['session']==6 and not state()['done']
        close(second);wait(lambda:state()['done']);wait(lambda:not any(w['pid'] in owned for w in windows()))
        results['new_session_during_fade_out']=True
        assert not (root/'state/wallpaper-lab/running.json').exists(),'Visual test unexpectedly started a renderer'
    finally:
        for w in windows():
            if w['pid'] in owned:close(w)
        for worker in workers:worker.wait(timeout=5)
        (out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))

#!/usr/bin/env python3
"""Desktop-only drag/persist regression; own disposable library and XDG state."""
import argparse,json,os,pathlib,subprocess,tempfile,time
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args()
if not a.desktop:p.error('--desktop required: moves the pointer inside a disposable app window')
a.output.mkdir(parents=True,exist_ok=True);app=a.app.resolve();results={}
def run(*args,env=None):return subprocess.check_output([str(x) for x in args],env=env,text=True)
def wait(fn):
    end=time.monotonic()+8
    while time.monotonic()<end:
        found=fn()
        if found:return found
        time.sleep(.05)
    raise AssertionError('Window/input condition timed out')
before=json.loads(run(pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'))
with tempfile.TemporaryDirectory(prefix='display-ui-',dir=a.output.resolve()) as tmp,(a.output/'test.log').open('w') as log:
    root=pathlib.Path(tmp);fixture=root/'library/1';fixture.mkdir(parents=True)
    (fixture/'project.json').write_text(json.dumps({'title':'Display UI fixture','type':'web','file':'index.html'}));(fixture/'index.html').write_text('<html></html>')
    socket=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'display-ui-{os.getpid()}.sock'
    env=os.environ|{'XDG_CONFIG_HOME':str(root/'config'),'XDG_STATE_HOME':str(root/'state'),'WALLPAPER_LAB_LIBRARY':str(root/'library'),'WALLPAPER_LAB_UI_STATE':str(root/'ui.json'),'YDOTOOL_SOCKET':str(socket)}
    daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log);ui=None
    try:
        wait(socket.exists);time.sleep(1.5);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log)
        def window():return next((w for w in json.loads(run('hyprctl','clients','-j')) if w['pid']==ui.pid),None)
        wait(window);wait(lambda:(root/'ui.json').exists());time.sleep(.5)
        def focus():run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.2)
        def move(x,y):
            w=window();run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={int(w["at"][0]+x)},y={int(w["at"][1]+y)}}}))')
        def click(x,y):
            focus();w=window();move(x(w) if callable(x) else x,y(w) if callable(y) else y);time.sleep(.1);run('ydotool','click','0xC0',env=env);time.sleep(.25)
        def state():return json.loads((root/'ui.json').read_text())
        click(lambda w:w['size'][0]-50,112)
        assert state()['settings'],'Display dialog did not receive the click; desktop may be locked'
        def layout():return json.loads(run(app,'--display-layout',env=env))
        def drag_left(down):
            focus();w=window();d=layout()['outputs'];left,right=d[0]['rect'],d[1]['rect']
            minx=min(r['rect'][0] for r in d);miny=min(r['rect'][1] for r in d)
            maxx=max(r['rect'][0]+r['rect'][2] for r in d);maxy=max(r['rect'][1]+r['rect'][3] for r in d)
            width=w['size'][0]-128;height=max(160,min(240,w['size'][1]-489))
            scale=min((width-80)/(maxx-minx),(height-60)/(maxy-miny))
            x=64+(width-(maxx-minx)*scale)/2+(left[0]-minx+left[2]/2)*scale
            y=328+(height-(maxy-miny)*scale)/2+(left[1]-miny+left[3]/2)*scale
            delta=65 if down else right[1]-left[1]
            move(x,y);time.sleep(.1);run('ydotool','click','0x40',env=env);time.sleep(.12)
            for step in range(1,13):move(x,y+delta*scale*step/12);time.sleep(.03)
            run('ydotool','click','0x80',env=env);time.sleep(.3)
        drag_left(True);click(lambda w:w['size'][0]-190,lambda w:w['size'][1]-111)
        changed=layout();assert changed['saved'] and changed['outputs'][0]['rect'][1]>50,changed
        results['drag_and_save_physical_position']=True
        drag_left(False);click(lambda w:w['size'][0]-190,lambda w:w['size'][1]-111)
        restored=layout();assert abs(restored['outputs'][0]['rect'][1]-restored['outputs'][1]['rect'][1])<1,restored
        results['drag_back_and_edge_snap']=True
        plan=json.loads(run(app,'--plan','1',env=env));assert json.loads(plan[plan.index('--lab-layout')+1])==restored['renderer']
        results['saved_positions_reach_renderer_plan']=True
        assert list((root/'state/wallpaper-lab/backups').glob('display-*/settings.json'));results['preference_backups_created']=True
        assert not (root/'state/wallpaper-lab/running.json').exists();results['no_renderer_started']=True
    finally:
        if ui and ui.poll() is None:
            w=window()
            if w:run('grim','-g',f'{w["at"][0]},{w["at"][1]} {w["size"][0]}x{w["size"][1]}',a.output/'last.png')
            ui.terminate();ui.wait(timeout=5)
        daemon.terminate();daemon.wait(timeout=5);socket.unlink(missing_ok=True)
        (a.output/'results.json').write_text(json.dumps(results,indent=2)+'\n')
assert json.loads(run(pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'))==before
results['personal_renderer_preserved']=True;(a.output/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))

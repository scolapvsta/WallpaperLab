#!/usr/bin/env python3
"""Explicit live test: hold/step/release, singleton, cancellation and saved layouts.
Uses an existing ydotool daemon (YDOTOOL_SOCKET) and the installed Hyprland binding.
Temporarily applies one favorite and restores the previous wallpaper/settings.
"""
import argparse,json,os,pathlib,subprocess,time,tempfile
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--output',required=True);a=p.parse_args()
if not a.desktop:p.error('--desktop required: exercises real keyboard input and changes the wallpaper')
home=pathlib.Path.home();app=home/'.local/bin/wallpaper-lab';config=home/'.config/wallpaper-lab';out=pathlib.Path(a.output);out.parent.mkdir(parents=True,exist_ok=True)
log=(out.parent/'popup-test.log').open('w');processes=[];results={}
def run(*args):return subprocess.check_output([str(x) for x in args],text=True)
def status():return json.loads(run(app,'--status'))
def popups():return [w for w in json.loads(run('hyprctl','clients','-j')) if w['class']=='wallpaper-lab' and w['title']=='Wallpaper Lab Carousel']
def wait(fn,timeout=12):
    end=time.monotonic()+timeout
    while time.monotonic()<end:
        value=fn()
        if value:return value
        time.sleep(.035)
    raise AssertionError('Condition timed out')
def key(*keys):run('ydotool','key','--key-delay','12',*keys)
def sequence(*keys):
    # Send the entire held chord from one client, with explicit pauses for
    # observing the popup before the release event.
    return subprocess.Popen(['ydotool','key','--key-delay','120',*keys],stdout=log,stderr=log)
def focus_main():
    mains=[w for w in json.loads(run('hyprctl','clients','-j')) if w['class']=='wallpaper-lab' and w['title']=='Wallpaper Lab']
    if mains:
        w=mains[0];x=w['at'][0]+w['size'][0]//2;y=w['at'][1]+w['size'][1]//2
        run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{w["pid"]}"}})); hl.dispatch(hl.dsp.cursor.move({{x={x},y={y}}}))')
def state():return json.loads((home/'.local/state/wallpaper-lab/popup/event.json').read_text())
def event(epoch,session,steps,action,env=None):
    p=subprocess.Popen([str(app),'--carousel-event',epoch,str(session),str(steps),action],stdout=log,stderr=log,env=env);processes.append(p);return p

def close_popup(w):
    proc=pathlib.Path('/proc')/str(w['pid'])
    if not proc.exists():return
    identity=(os.readlink(proc/'exe'),(proc/'stat').read_text().rsplit(')',1)[1].split()[19])
    if identity[0]!=str(home/'.local/share/wallpaper-lab/runtime/wallpaper-lab'):return
    if identity==(os.readlink(proc/'exe'),(proc/'stat').read_text().rsplit(')',1)[1].split()[19]):run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.close({{window="pid:{w["pid"]}"}}))')
original_settings=(config/'settings.json').read_bytes();original_carousel=(config/'carousel.json').read_bytes();before=status();assert before['alive']
rows=json.loads(run(app,'--list'));valid={r['path'] for r in rows if not r['reason']};paths=[x for x in json.loads(original_carousel) if x in valid];assert len(paths)>=2
out.with_suffix('.backup.json').write_text(json.dumps({'settings':json.loads(original_settings),'carousel':json.loads(original_carousel)}))
try:
    focus_main();begin=time.monotonic()
    keyboard=sequence('125:1',*['15:1','15:0']*5,*['0']*20,'125:0')
    w=wait(lambda:popups());assert len(w)==1;pid=w[0]['pid'];assert w[0]['floating']
    results['cold_popup_ms']=round((time.monotonic()-begin)*1000)
    assert status()['pid']==before['pid']
    # Four additional taps without releasing Super, then settle one animation.
    wait(lambda:state().get('steps')==5);time.sleep(.3)
    assert len(popups())==1 and popups()[0]['pid']==pid
    assert status()['pid']==before['pid'];results['hold_no_render_and_single_popup']=True
    w=popups()[0];x,y=w['at'];sw,sh=w['size'];run('grim','-g',f'{x},{y} {sw}x{sh}',out.parent/'popup-held.png')
    initial=paths.index(before['path']) if before['path'] in paths else -1
    expected=paths[(initial+5)%len(paths)]
    wait(lambda:state().get('commit'));released=time.monotonic();wait(lambda:not popups(),3);keyboard.wait(timeout=3)
    results['release_hide_ms']=round((time.monotonic()-released)*1000)
    wait(lambda:status().get('path')==expected and status()['alive'],35)
    results['five_taps_final_selection']=True
    # A new hold followed by Escape must keep the confirmed renderer unchanged.
    stable=status();focus_main();keyboard=sequence('125:1','15:1','15:0',*['0']*10,'1:1','1:0',*['0']*3,'125:0')
    w=wait(lambda:popups())[0]
    run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{w["pid"]}"}}))')
    wait(lambda:not popups(),5);keyboard.wait(timeout=5);time.sleep(.3)
    assert status()['pid']==stable['pid'];results['escape_cancels']=True
    epoch='layouts-'+str(time.time_ns())
    results['layout_sizes']=[]
    for mode in range(4):
        prefs=json.loads(original_settings);prefs['carousel_layout']=mode;(config/'settings.json').write_text(json.dumps(prefs))
        event(epoch,mode+1,1,'step');w=wait(lambda:popups())[0];time.sleep(.55)
        x,y=w['at'];sw,sh=w['size'];run('grim','-g',f'{x},{y} {sw}x{sh}',out.parent/f'popup-layout-{mode}.png')
        results['layout_sizes'].append([sw,sh])
        if len(paths)==4 and mode==0:assert [sw,sh]==[392,392]
        if mode==1:assert sh<=232
        if mode==2:assert sw<=232
        if mode==3:assert sh<=325
        event(epoch,mode+1,2,'step');time.sleep(.04)
        run('grim','-g',f'{x},{y} {sw}x{sh}',out.parent/f'popup-selection-mid-{mode}.png')
        time.sleep(.2)
        run('grim','-g',f'{x},{y} {sw}x{sh}',out.parent/f'popup-selection-settled-{mode}.png')
        close_popup(w);time.sleep(.035)
        run('grim','-g',f'{x},{y} {sw}x{sh}',out.parent/f'popup-fade-out-{mode}.png')
        wait(lambda:not popups(),3)
        assert status()['pid']==stable['pid']
    results['four_layouts_and_close_cancels']=True
    # Exercise grid sizing without writing the user's favorites or settings.
    with tempfile.TemporaryDirectory(prefix='wallpaper-popup-geometry-') as tmp:
        root=pathlib.Path(tmp);prefs=root/'config/wallpaper-lab';prefs.mkdir(parents=True)
        (prefs/'settings.json').write_text(json.dumps({'carousel_layout':0}))
        env=os.environ|{'XDG_CONFIG_HOME':str(root/'config'),'XDG_STATE_HOME':str(root/'state')}
        fixtures=[r['path'] for r in rows if not r['reason']][:9]
        assert len(fixtures)==9
        for n,size in [(0,[320,80]),(1,[232,232]),(3,[392,392]),(9,[552,552])]:
            (prefs/'carousel.json').write_text(json.dumps(fixtures[:n]))
            worker=event('geometry',n+1,1,'step',env);w=wait(lambda:popups())[0]
            time.sleep(.2);assert w['size']==size
            x,y=w['at'];sw,sh=w['size'];run('grim','-g',f'{x},{y} {sw}x{sh}',out.parent/f'grid-{n}.png')
            close_popup(w);wait(lambda:not popups(),3);worker.wait(timeout=5)
            assert status()['pid']==stable['pid']
    results['empty_single_three_nine_grid']=True
    # A second shortcut arriving during fade-out must get its own live popup.
    handoff='fade-handoff-'+str(time.time_ns())
    event(handoff,1,len(paths),'step');first=wait(lambda:popups())[0];time.sleep(.2)
    event(handoff,1,len(paths),'commit');wait(lambda:state().get('commit'));time.sleep(.025)
    event(handoff,2,1,'step')
    second=wait(lambda:next((w for w in popups() if w['pid']!=first['pid']),None))
    time.sleep(.2);assert state()['session']==2 and not state()['done']
    close_popup(second);wait(lambda:not popups(),3)
    assert status()['pid']==stable['pid'];results['new_session_during_fade_out']=True
    # Release delivered first: no stuck popup and exactly the final absolute step.
    current=status();base=paths.index(current['path']) if current['path'] in paths else -1
    count=2;target=paths[(base+count)%len(paths)]
    event('early-'+epoch,1,count,'commit');event('early-'+epoch,1,1,'step')
    wait(lambda:status().get('path')==target and status()['alive'],35)
    wait(lambda:not popups(),3);results['release_before_open']=True
finally:
    if 'keyboard' in locals():keyboard.wait(timeout=10)
    key('15:0','125:0')
    for w in popups():close_popup(w)
    wait(lambda:not popups(),3)
    for p in processes:
        try:p.wait(timeout=35)
        except subprocess.TimeoutExpired:raise RuntimeError('Popup worker did not exit')
    (config/'settings.json').write_bytes(original_settings)
    assert (config/'carousel.json').read_bytes()==original_carousel,'User favorites changed'
    if status().get('path')!=before['path']:run(app,'--restore')
    out.write_text(json.dumps(results,indent=2)+'\n');log.close()
print(json.dumps(results,indent=2))

"""Opt-in launcher check for the panorama switch and native final capture."""
import argparse,json,os,subprocess,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--app',type=Path,required=True);p.add_argument('--scene',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True)
socket=Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-panorama-{os.getpid()}.sock'
env=os.environ|dict(XDG_CONFIG_HOME=str(out/'config'),XDG_STATE_HOME=str(out/'state'),XDG_DATA_HOME=str(out/'data'),XDG_CACHE_HOME=str(out/'cache'),WALLPAPER_LAB_LIBRARY=str(a.scene.resolve().parent),WALLPAPER_LAB_UI_STATE=str(out/'ui.json'),WALLPAPER_LAB_UI_READ_ONLY='1',YDOTOOL_SOCKET=str(socket))
(config/'settings.json').write_text(json.dumps(dict(language='en',theme_scoped_v3=True,ui_design=2,ui_theme=3,caelestia_enabled=True)))
(config/'sddm.json').write_text(json.dumps(dict(enabled=True,wallpaper=dict(source=str(a.scene.resolve())),ui=dict(preset='astronaut'))))
def run(*args):return subprocess.check_output([str(a)for a in args],env=env,text=True)
assert json.loads(run(app,'--sddm','check'))['ready']
def g():return json.loads((out/'ui.json.geometry').read_text())
def s():return json.loads((out/'ui.json.style').read_text())
def window():return next(w for w in json.loads(run('hyprctl','clients','-j'))if w['pid']==ui.pid)
def wait(fn,timeout=40):
    deadline=time.monotonic()+timeout
    while time.monotonic()<deadline:
        assert ui.poll()is None,('Launcher exited',ui.returncode,(out/'launcher.log').read_text()[-2000:])
        try:
            value=fn()
            if value:return value
        except(OSError,ValueError,KeyError,StopIteration):pass
        time.sleep(.1)
    raise AssertionError('UI condition timed out')
def click(point):
    for attempt in range(4):
        run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.2)
        w=window();x,y=round(w['at'][0]+point[0]),round(w['at'][1]+point[1]);run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={x},y={y},relative=false}}))');time.sleep(.15)
        cursor=json.loads(run('hyprctl','cursorpos','-j'))
        if json.loads(run('hyprctl','activewindow','-j')).get('pid')==ui.pid and abs(cursor['x']-x)<2 and abs(cursor['y']-y)<2:break
    else:raise AssertionError('Test window lost focus; no click sent')
    run('ydotool','click','0xC0');time.sleep(.35)
with(out/'launcher.log').open('w')as log:
    daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log)
    ui=subprocess.Popen([str(app),'--sddm-settings'],env=env,stdout=log,stderr=log)
    try:
        wait(window);wait(lambda:s()['sddm_ready']);wait(socket.exists)
        run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.float({{action="on",window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x=1264,y=900,relative=false}}));hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}}))');time.sleep(1)
        click(g()['sddm_tabs'][2]);wait(lambda:g()['sddm_view']==2)
        assert g()['choices']['94']['kind']=='toggle'and g()['choices']['94']['value']==0
        click(g()['choices']['94']['control']);wait(lambda:not s()['sddm_busy']and g()['choices']['94']['value']==1)
        prefs=json.loads((config/'sddm.json').read_text());assert prefs['wallpaper']['display_mode']=='span'
        status=json.loads(run(app,'--sddm','status'));wait(lambda:Path(status['final_preview']).is_file());time.sleep(1)
        w=window();x,y=w['at'];width,height=w['size'];run('grim','-g',f'{x},{y} {width}x{height}',out/'panorama.png')
        assert s()['sddm_ok']and s()['sddm_ready']
        (out/'results.json').write_text(json.dumps(dict(panorama_switch=True,native_combined_capture=True,size=w['size']),indent=2)+'\n');print((out/'results.json').read_text())
    finally:
        for child in [ui,daemon]:
            if child.poll()is None:child.terminate();child.wait(timeout=30)
        socket.unlink(missing_ok=True)

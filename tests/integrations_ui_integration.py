"""Opt-in Hyprland UI checks for integration activation, routing and saved choices."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import time

p=argparse.ArgumentParser()
p.add_argument('--app',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True)
library=out/'library';library.mkdir(exist_ok=True)
socket=Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-integrations-{os.getpid()}.sock'
helper=out/'caelestia-fixture.py'
# UI-only dependency preparation. Isolated state has no running wallpaper and
# cannot publish a palette; the real backend publication has separate CTest coverage.
helper.write_text('import json,sys\nprint(json.dumps({"python":sys.executable}))\n')
env=os.environ|dict(XDG_CONFIG_HOME=str(out/'config'),XDG_STATE_HOME=str(out/'state'),XDG_DATA_HOME=str(out/'data'),
    XDG_CACHE_HOME=str(out/'cache'),WALLPAPER_LAB_LIBRARY=str(library),WALLPAPER_LAB_UI_STATE=str(out/'ui.json'),
    WALLPAPER_LAB_UI_READ_ONLY='1',WALLPAPER_LAB_CAELESTIA_HELPER=str(helper),YDOTOOL_SOCKET=str(socket))
(config/'settings.json').write_text(json.dumps(dict(theme_scoped_v3=True,ui_design=0,ui_theme=14,language='en',caelestia_enabled=False)))
log=(out/'ui.log').open('w');ui=None;daemon=None;results={}
def run(*args):return subprocess.check_output([str(a) for a in args],env=env,text=True)
def read(name):return json.loads((out/name).read_text())
def g():return read('ui.json.geometry')
def s():return read('ui.json.style')
def state():return read('ui.json')
def wait(fn,timeout=25):
    deadline=time.monotonic()+timeout
    while time.monotonic()<deadline:
        try:
            value=fn()
            if value:return value
        except (OSError,ValueError,KeyError,StopIteration):pass
        assert not ui or ui.poll() is None,(out/'ui.log').read_text()[-2000:]
        time.sleep(.1)
    raise AssertionError('UI condition timed out')
def window():return next(w for w in json.loads(run('hyprctl','clients','-j')) if w['pid']==ui.pid)
def click(point):
    w=window();x,y=round(w['at'][0]+point[0]),round(w['at'][1]+point[1])
    run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.cursor.move({{x={x},y={y},relative=false}}))')
    time.sleep(.12)
    assert json.loads(run('hyprctl','activewindow','-j')).get('pid')==ui.pid
    run('ydotool','click','0xC0');time.sleep(.25)
def toggle(id):click(g()['choices'][str(id)]['control'])
def shot(name):
    w=window();x,y=w['at'];width,height=w['size'];run('grim','-g',f'{x},{y} {width}x{height}',out/(name+'.png'))
def settings():
    if not state()['settings']:click(g()['header']['settings'])
    click(g()['settings_tabs'][7]);wait(lambda:s()['settings_tab']==7)
def close_settings():
    # Click the modal's close control with the same focus checks as other inputs.
    w,h=window()['size'];mw,mh=min(860,w-48),min(650,h-64)
    click(((w-mw)/2+mw-40,(h-mh)/2+33))
    wait(lambda:not state()['settings'])
def hub():
    if state()['settings']:close_settings()
    click(g()['header']['integrations']);wait(lambda:state()['page']==4)
def choose(id,index):
    click(g()['choices'][str(id)]['control']);wait(lambda:g()['choice_open']==id)
    click(g()['choices'][str(id)]['options'][index]);wait(lambda:g()['choice_open']!=id)
def stop(process):
    if process and process.poll() is None:process.terminate();process.wait(timeout=15)
try:
    daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log);wait(socket.exists)
    ui=subprocess.Popen([str(app),'--integrations'],env=env,stdout=log,stderr=log);wait(window)
    run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.float({{action="on",window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x=1400,y=900,relative=false}}));hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))')
    wait(lambda:s()['settings_tab']==7 and not s()['sddm_busy']);time.sleep(.5)
    assert not s()['integrations_visible'];assert g()['header']['integrations']==[0,0]
    assert len(g()['settings_tabs'])==8
    shot('activation-settings');results['none_hides_tab']=True
    toggle(73);wait(lambda:s()['caelestia_enabled'] and s()['integrations_visible'])
    hub();wait(lambda:s()['integration_tab']==0)
    assert g()['integration_tabs'][1]==[0,0];assert g()['header']['integrations'][0]>g()['header']['workshop'][0]
    run('ydotool','key','29:1','38:1','38:0','29:0');time.sleep(.3)
    assert not s()['search_focus'];results['hub_blocks_library_search_shortcut']=True
    choose(78,2);wait(lambda:json.loads((config/'settings.json').read_text()).get('caelestia_palette')==2)
    shot('caelestia-only');results['caelestia_only_and_saved_palette']=True
    settings();toggle(88);wait(lambda:s()['sddm_enabled'] and s()['sddm_ready'] and not s()['sddm_busy'])
    hub();click(g()['integration_tabs'][1]);wait(lambda:s()['integration_tab']==1)
    assert all(point[0]>0 for point in g()['integration_tabs'])
    choose(90,1);wait(lambda:not s()['sddm_busy'])
    preset=json.loads((config/'sddm.json').read_text())['ui']['preset']
    shot('both-sddm');click(g()['integration_tabs'][0]);wait(lambda:s()['integration_tab']==0)
    assert json.loads((config/'settings.json').read_text())['caelestia_palette']==2
    results['both_panels_and_independent_choices']=True
    # Disabling the visible panel switches to the remaining integration.
    settings();toggle(73);wait(lambda:not s()['caelestia_enabled']);close_settings()
    wait(lambda:s()['integration_tab']==1 and state()['page']==4)
    assert g()['integration_tabs'][0]==[0,0]
    assert json.loads((config/'sddm.json').read_text())['ui']['preset']==preset
    shot('sddm-only');results['disable_current_routes_to_remaining']=True
    settings();toggle(88);wait(lambda:not s()['sddm_enabled'] and not s()['sddm_busy'])
    wait(lambda:not s()['integrations_visible'] and state()['page']==0)
    assert state()['settings'];assert g()['header']['integrations']==[0,0]
    results['disable_last_hides_tab_preserves_settings_modal']=True
    toggle(73);wait(lambda:s()['caelestia_enabled']);hub();wait(lambda:state()['page']==4)
    assert json.loads((config/'settings.json').read_text())['caelestia_palette']==2
    results['reenable_preserves_preferences']=True
    (out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
finally:
    stop(ui);stop(daemon);log.close();socket.unlink(missing_ok=True)

"""Opt-in native login editor checks with isolated local-media fixtures."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import time
from PIL import Image

p=argparse.ArgumentParser()
p.add_argument('--app',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True)
library=out/'library';library.mkdir(exist_ok=True)
socket=Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-integrations-{os.getpid()}.sock'
env=os.environ|dict(XDG_CONFIG_HOME=str(out/'config'),XDG_STATE_HOME=str(out/'state'),XDG_DATA_HOME=str(out/'data'),
    XDG_CACHE_HOME=str(out/'cache'),WALLPAPER_LAB_LIBRARY=str(library),WALLPAPER_LAB_UI_STATE=str(out/'ui.json'),
    WALLPAPER_LAB_UI_READ_ONLY='1',YDOTOOL_SOCKET=str(socket))
(config/'settings.json').write_text(json.dumps(dict(theme_scoped_v3=True,ui_design=0,ui_theme=14,language='en',caelestia_enabled=False)))
(config/'login-screen.json').unlink(missing_ok=True)
log=(out/'ui.log').open('w');ui=None;daemon=None;results={}
def run(*args):
    if args[:2] in [('ydotool','key'),('ydotool','type')] and ui:
        subprocess.check_output(['hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))'],env=env,text=True);time.sleep(.15)
        active=json.loads(subprocess.check_output(['hyprctl','activewindow','-j'],env=env,text=True));assert active.get('pid')==ui.pid,'Test window lost focus; no keyboard input sent'
    if args[:2]==('ydotool','key'):
        for key in args[2:]:subprocess.check_output(['ydotool','key',str(key)],env=env,text=True);time.sleep(.09)
        return ''
    if args[:2]==('ydotool','type'):args=(*args[:2],'--key-delay','60',*args[2:])
    return subprocess.check_output([str(a) for a in args],env=env,text=True)
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
        assert not ui or ui.poll() is None,(ui.returncode,(out/'ui.log').read_text()[-2000:])
        time.sleep(.1)
    raise AssertionError('UI condition timed out')
def window():return next(w for w in json.loads(run('hyprctl','clients','-j')) if w['pid']==ui.pid)
def click(point):
    for attempt in range(4):
        run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.2)
        w=window();x,y=round(w['at'][0]+point[0]),round(w['at'][1]+point[1])
        run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={x},y={y},relative=false}}))');time.sleep(.15)
        cursor=json.loads(run('hyprctl','cursorpos','-j'))
        if json.loads(run('hyprctl','activewindow','-j')).get('pid')==ui.pid and abs(cursor['x']-x)<4 and abs(cursor['y']-y)<4:break
    else:raise AssertionError(('Test window could not retain focus; no click sent',point,w['at'],cursor,json.loads(run('hyprctl','activewindow','-j')).get('pid'),ui.pid))
    run('ydotool','click','0x40');time.sleep(.15);run('ydotool','click','0x80');time.sleep(.25)
def toggle(id):click(g()['choices'][str(id)]['control'])
def shot(name):
    w=window();x,y=w['at'];width,height=w['size'];run('grim','-g',f'{x},{y} {width}x{height}',out/(name+'.png'))
def choose(id,index):
    click(g()['choices'][str(id)]['control']);wait(lambda:g()['choice_open']==id)
    click(g()['choices'][str(id)]['options'][index]);wait(lambda:g()['choice_open']!=id)
def stop(process):
    if process and process.poll() is None:process.terminate();process.wait(timeout=15)
# Build a small local catalog without requiring Workshop assets.
run(app,'--library-path','local',library)
for name,color,ext in [('Blue','#246bc0','png'),('Gold','#d09240','png'),('Motion','#609040','gif')]:
    path=out/(name+'.'+ext);im=Image.new('RGB',(640,360),color);im.save(path,save_all=True,append_images=[Image.new('RGB',(640,360),'#80b050')],duration=300,loop=0) if ext=='gif' else im.save(path)
    run(app,'--import',path)
catalog=json.loads(run(app,'--list'));source=next(v['path']for v in catalog if v['title']=='Blue')
run(app,'--login','set',json.dumps(dict(enabled=True,source=source)))
def draft():return json.loads((config/'login-screen.json').read_text())
def scroll_inspector(amount):
    x,y,w,h=g()['login_inspector'];win=window();run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={round(win["at"][0]+x+w/2)},y={round(win["at"][1]+y+h/2)},relative=false}}))');time.sleep(.1);run('ydotool','mousemove','--wheel','--','0',str(amount));time.sleep(.4)
def visible(get):
    for _ in range(28):
        point=get();x,y,w,h=g()['login_inspector']
        if y+108<point[1]<y+h-30:return point
        scroll_inspector(-2 if point[1]>=y+h-30 else 2)
    raise AssertionError(('Control outside inspector',get(),g()['login_inspector']))
try:
    daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log);wait(socket.exists)
    ui=subprocess.Popen([str(app),'--login-editor'],env=env,stdout=log,stderr=log);wait(window)
    run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.float({{action="on",window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x=1264,y=900,relative=false}}));hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}}))')
    wait(lambda:g()['login_available']==3);time.sleep(3);click(g()['login_search']);wait(lambda:g()['login_search_focus']);run('ydotool','type','Blue');wait(lambda:g()['login_available']==1);shot('search')
    run('ydotool','key','29:1','30:1','30:0','29:0','14:1','14:0');wait(lambda:g()['login_available']==3)
    click(g()['login_filter']);wait(lambda:g()['login_filters_open']);choose(51,2);wait(lambda:g()['login_available']==0);choose(51,5);wait(lambda:g()['login_available']==3)
    choose(58,1);wait(lambda:g()['login_available']==0);choose(58,2);wait(lambda:g()['login_available']==3);shot('filters')
    click(g()['header']['library']);wait(lambda:state()['page']==0 and state()['results']==3);click(g()['login_header']);wait(lambda:state()['page']==5 and g()['login_available']==3)
    click(g()['login_filter']);wait(lambda:not g()['login_filters_open']);results['search_and_shared_type_source_filters']=True
    click(g()['inspector_sections'][0]);time.sleep(.3)
    # Shared image controls store login-specific adjustments, without desktop writes.
    def contrast():
        track=g()['image_tracks'][1];return [track[0]+track[2]*.7,track[1]+2]
    click(visible(contrast));wait(lambda:draft()['profiles'].get(source,{}).get('contrast',1)>1.3)
    assert not(config/'wallpaper_profiles.json').exists();results['image_controls_are_login_only']=True;shot('image-inspector')
    click(g()['login_tabs'][1]);wait(lambda:g()['login_tab']==1)
    assert len(g()['login_tabs'])==2
    monitor=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']==draft()['ui_primary']);assert g()['login_position_size']==[monitor['width'],monitor['height']],g()['login_position_size']
    track=g()['number_tracks']['152'];click((track[0]+track[2],track[1]+2));wait(lambda:abs(draft()['widgets']['login']['y']+draft()['widgets']['login']['height']-1)<.002)
    results['pixel_position_slider_reaches_monitor_edge']=True
    click(g()['login_reset'][0]);wait(lambda:abs(draft()['widgets']['login']['y']-.39)<.001)
    # The label is an actual caret/selection editor, not a button.
    click(visible(lambda:g()['login_text_fields'][0]));run('ydotool','type','Welcome home');wait(lambda:draft()['widgets']['login']['welcome_text']=='Welcome home');shot('text-edit')
    click(g()['login_reset'][0]);wait(lambda:draft()['widgets']['login']['welcome_text']=='');results['text_entry_and_widget_reset']=True
    toggle(108);wait(lambda:draft()['auto_palette'] and draft()['widgets']['login']['background_opacity']==.96);dark=draft()['widgets']['login']['button_color'];shot('palette-dark')
    toggle(109);wait(lambda:draft()['light'] and draft()['widgets']['login']['button_color']!=dark);shot('palette-light');results['automatic_light_dark_palette']=True
    click(g()['login_inspector_tabs'][1]);wait(lambda:g()['choices']['96']['control'][0]>0)
    # Pick the text color, using the first color button below opacity.
    def text_color():
        t=g()['number_tracks']['163'];return [t[0]+t[2]/2,t[1]+60]
    click(visible(text_color));wait(lambda:g()['login_color']['open']==2053)
    point=visible(lambda:[g()['login_color']['square'][0]+g()['login_color']['square'][2]*.8,g()['login_color']['square'][1]+g()['login_color']['square'][3]*.3]);click(point);wait(lambda:not draft()['auto_palette']);shot('visual-color-picker');results['visual_picker_and_manual_override']=True
    kept={k:draft()[k]for k in ['source','profiles','mode','fps']};click(g()['login_reset'][1]);wait(lambda:draft()['widgets']['login']['color']=='#f5f5f5');assert all(draft()[k]==v for k,v in kept.items());assert list((out/'state/wallpaper-lab/backups').glob('login-ui-*'));results['full_ui_reset_preserves_wallpaper_and_backups']=True
    run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x=1000,y=740,relative=false}}))');time.sleep(.5);shot('compact-editor');click(g()['login_tabs'][0]);time.sleep(.5);shot('compact-wallpapers')
    (out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
finally:
    stop(ui);stop(daemon);log.close();socket.unlink(missing_ok=True)

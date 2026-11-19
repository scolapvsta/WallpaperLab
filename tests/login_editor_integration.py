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
def read(name):
    # Geometry/style diagnostics may be rewritten while the UI is drawing.
    for attempt in range(20):
        try:return json.loads((out/name).read_text())
        except (FileNotFoundError,json.JSONDecodeError):
            if attempt==19:raise
            time.sleep(.02)
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
desktop_profiles={source:dict(position=dict(x=-.4,y=.25,zoom=1.6),image=dict(brightness=.2,contrast=1.15,saturation=.8,flip_x=1))}
(config/'wallpaper_profiles.json').write_text(json.dumps(desktop_profiles))
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
    assert json.loads((config/'wallpaper_profiles.json').read_text())==desktop_profiles;results['image_controls_are_login_only']=True;shot('image-inspector')
    before_copy=draft();click(visible(lambda:g()['login_copy_library']));wait(lambda:draft()['profiles'][source].get('zoom')==1.6)
    copied=draft();assert all(copied['profiles'][source][k]==v for k,v in dict(x=-.4,y=.25,zoom=1.6,brightness=.2,contrast=1.15,saturation=.8,flip_x=1).items())
    assert copied['widgets']==before_copy['widgets'] and copied['mode']==before_copy['mode'] and copied['fps']==before_copy['fps']
    assert json.loads((config/'wallpaper_profiles.json').read_text())==desktop_profiles and list((out/'state/wallpaper-lab/backups').glob('login-library-copy-*'))
    results['copy_library_adjustments_without_desktop_or_ui_changes']=True;shot('copy-library')
    click(g()['login_tabs'][1]);wait(lambda:g()['login_tab']==1)
    assert len(g()['login_tabs'])==2
    monitor=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']==draft()['ui_primary']);assert g()['login_position_size']==[monitor['width'],monitor['height']],g()['login_position_size']
    track=g()['number_tracks']['152'];click((track[0]+track[2],track[1]+2));wait(lambda:abs(draft()['widgets']['login']['y']+draft()['widgets']['login']['height']-1)<.002)
    results['pixel_position_slider_reaches_monitor_edge']=True
    click(g()['login_reset'][0]);wait(lambda:abs(draft()['widgets']['login']['y']-.39)<.001)
    # The label is an actual caret/selection editor, not a button.
    click(visible(lambda:g()['login_text_fields'][0]));run('ydotool','type','Welcome home');wait(lambda:draft()['widgets']['login']['welcome_text']=='Welcome home');shot('text-edit')
    click(g()['login_reset'][0]);wait(lambda:draft()['widgets']['login']['welcome_text']=='');results['text_entry_and_widget_reset']=True
    click(g()['login_inspector_tabs'][1]);time.sleep(.3)
    before=draft();click(g()['login_palette'][0]);wait(lambda:draft()['widgets']['login']['button_color']!=before['widgets']['login']['button_color'])
    def without_colors(value):
        return {k:without_colors(v)for k,v in value.items()if k not in {'color','accent','background','border_color','effect_color','field_color','button_color','button_text_color','button_effect_color','password_color','password_background','placeholder_color','user_icon_color','password_icon_color','header_color','system_color','session_color','keyboard_color','warning_color','dropdown_color','dropdown_background','dropdown_selected','highlight'}} if isinstance(value,dict)else value
    assert without_colors(draft())==without_colors(before)
    shot('color-suggestion');click(g()['login_palette'][1]);wait(lambda:draft()==before);results['color_suggestion_preserves_all_non_colors_and_undoes']=True
    assert 'auto_palette'not in draft()and'light'not in draft()
    click(visible(lambda:g()['login_color_buttons'][5]));wait(lambda:g()['login_color']['open']==2053)
    # Exact channel editing uses real RGB values, no hexadecimal knowledge needed.
    for channel,value in enumerate([40,160,220]):
        click(visible(lambda:g()['color_rgb_inputs'][channel]));run('ydotool','key','29:1','30:1','30:0','29:0');run('ydotool','type',str(value));run('ydotool','key','28:1','28:0')
    wait(lambda:draft()['widgets']['login']['color']=='#28a0dc');shot('rgb-inputs');results['numeric_rgb']=True
    click(visible(lambda:g()['color_rgb_inputs'][0]));run('ydotool','key','29:1','30:1','30:0','29:0');run('ydotool','type','300');run('ydotool','key','28:1','28:0');time.sleep(.5);assert draft()['widgets']['login']['color']=='#28a0dc';results['invalid_rgb_preserves_color']=True
    if subprocess.run(['which','hyprpicker'],stdout=subprocess.DEVNULL).returncode==0:
        # Sample a solid part of the app chrome; compare with its actual composited pixel.
        point=visible(lambda:g()['color_eye']);win=window();px,py=win['at'][0]+40,win['at'][1]+160
        run('grim','-g',f'{px},{py} 1x1',out/'sample.png');expected=Image.open(out/'sample.png').convert('RGB').getpixel((0,0))
        click(point);wait(lambda:g()['color_picking']>=0);time.sleep(.7);run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={px},y={py},relative=false}}))');time.sleep(.3);run('ydotool','click','0x40');time.sleep(.15);run('ydotool','click','0x80')
        wait(lambda:draft()['widgets']['login']['color']!='#28a0dc');actual=tuple(bytes.fromhex(draft()['widgets']['login']['color'][1:]));assert max(abs(a-b)for a,b in zip(actual,expected))<=3,(actual,expected);results['desktop_eyedropper']=True
        saved_color=draft()['widgets']['login']['color'];click(visible(lambda:g()['color_eye']));wait(lambda:g()['color_picking']>=0);time.sleep(.5)
        for key in ['1:1','1:0']:subprocess.run(['ydotool','key',key],env=env,check=True,stdout=subprocess.DEVNULL);time.sleep(.1)
        wait(lambda:g()['color_picking']<0);assert draft()['widgets']['login']['color']==saved_color;results['eyedropper_cancel_preserves_color']=True
    run('ydotool','key','1:1','1:0');time.sleep(.3)
    # Button text and effect have their own section and independently persist.
    click(visible(lambda:g()['login_sections'][6]));time.sleep(.4)
    click(visible(lambda:g()['login_text_fields'][4]));run('ydotool','type','Enter my desktop');wait(lambda:draft()['widgets']['login']['button_label']=='Enter my desktop')
    click(visible(lambda:g()['choices']['113']['control']));wait(lambda:g()['choice_open']==113);click(g()['choices']['113']['options'][2]);wait(lambda:draft()['widgets']['login']['button_effect']=='extrude')
    choose(113,0);wait(lambda:draft()['widgets']['login']['button_effect']=='none');results['independent_sign_in_label_and_effect']=True;shot('button-controls')
    # UI-only presets preserve the selected image and its crop settings.
    saved=draft();click(g()['login_presets'][0]);run('ydotool','type','Evening UI');click(g()['login_presets'][2]);wait(lambda:len(list((config/'login-presets/ui').glob('*.json')))==1)
    click(g()['login_reset'][1]);wait(lambda:draft()['widgets']['login']['button_label']=='');click(g()['login_presets'][1]);wait(lambda:draft()['widgets']==saved['widgets']);assert draft()['source']==source and draft()['profiles']==saved['profiles'];results['ui_preset_save_load']=True
    kept={k:draft()[k]for k in ['source','profiles','mode','fps']};click(g()['login_reset'][1]);wait(lambda:draft()['widgets']['login']['color']=='#f5f5f5');assert all(draft()[k]==v for k,v in kept.items());assert list((out/'state/wallpaper-lab/backups').glob('login-ui-*'));results['full_ui_reset_preserves_wallpaper_and_backups']=True
    # A complete preset restores both wallpaper edits and the UI, surviving an app restart.
    click(g()['login_tabs'][0]);wait(lambda:g()['login_tab']==0);saved=draft();click(g()['login_presets'][0]);run('ydotool','type','Blue scene');click(g()['login_presets'][2]);wait(lambda:len(list((config/'login-presets/complete').glob('*.json')))==1)
    gold=next(i for i,v in enumerate(catalog)if v['title']=='Gold');card=next(c['rect']for c in g()['login_cards']if c['index']==gold);click([card[0]+card[2]/2,card[1]+30]);wait(lambda:draft()['source']!=source)
    click(g()['login_presets'][1]);wait(lambda:draft()['source']==source);assert draft()['profiles']==saved['profiles']and draft()['widgets']==saved['widgets'] and draft()['project_profiles']==saved['project_profiles'];results['complete_preset_restores_wallpaper_and_ui']=True
    stop(ui);ui=subprocess.Popen([str(app),'--login-editor'],env=env,stdout=log,stderr=log);wait(window);time.sleep(3)
    choose(111,1);click(g()['login_presets'][1]);wait(lambda:draft()['source']==source);results['presets_survive_restart']=True
    imported=json.loads(next((config/'login-presets/complete').glob('*.json')).read_text());imported['name']='Imported blue';imported['data']['widgets']['login']['welcome_text']='Imported';(config/'login-presets/complete/imported.json').write_text(json.dumps(imported))
    choose(111,2);click(g()['login_presets'][1]);wait(lambda:draft()['widgets']['login']['welcome_text']=='Imported');results['external_json_preset_discovery']=True
    run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.float({{action="on",window="pid:{ui.pid}"}}));hl.dispatch(hl.dsp.window.resize({{window="pid:{ui.pid}",x=1000,y=740,relative=false}}));hl.dispatch(hl.dsp.window.center({{window="pid:{ui.pid}"}}))');time.sleep(.5);shot('compact-wallpapers');click(g()['login_tabs'][1]);time.sleep(.5);shot('compact-editor')
    # Load every shipped design through the actual preset selector. The second
    # Sakura intentionally has identical UI settings and a separate entry.
    shipped=sorted((json.loads(p.read_text())for p in (app.parent/'login-assets/astronaut/presets').glob('*.json')),key=lambda p:p['name'])
    assert len(shipped)==10
    kept={k:draft()[k]for k in ['source','profiles','mode','fps']}
    for index,preset in enumerate(shipped):
        click(g()['choices']['111']['control']);wait(lambda:g()['choice_open']==111)
        for attempt in range(16):
            point=g()['choices']['111']['options'][index+1]
            if point[0]>0 and point[1]>0:break
            options=g()['choices']['111']['options'];first=next(n for n,p in enumerate(options)if p[0]>0 and p[1]>0);direction='2' if index+1<first else '-2';point=options[first];win=window();run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={round(win["at"][0]+point[0])},y={round(win["at"][1]+point[1])},relative=false}}))');run('ydotool','mousemove','--wheel','--','0',direction);time.sleep(.3)
        else:raise AssertionError('Shipped preset did not become visible')
        click(point);wait(lambda:g()['choice_open']!=111);click(g()['login_presets'][1]);wait(lambda:draft()['widgets']['login']['font']==preset['data']['font'] and draft()['widgets']['login']['layout']=='astronaut')
        assert all(draft()[k]==v for k,v in kept.items())
        assert draft()['widgets']['login']['astronaut']==preset['data']['widgets']['login']['astronaut']
        click(g()['login_inspector_tabs'][0]);time.sleep(.3)
        if not draft()['widgets']['login']['welcome_enabled']:
            click(visible(lambda:g()['choices']['100']['control']));wait(lambda:draft()['widgets']['login']['welcome_enabled'])
        click(visible(lambda:g()['login_text_fields'][0]));run('ydotool','key','29:1','30:1','30:0','29:0');run('ydotool','type','Welcome home');wait(lambda:draft()['widgets']['login']['welcome_text']=='Welcome home')
        run('ydotool','key','28:1','28:0');time.sleep(.3);shot('preset-'+str(index+1)+'-welcome')
    results['ten_shipped_presets_load_in_editor_without_wallpaper_changes']=True
    results['welcome_can_be_enabled_and_edited_on_all_ten_presets']=True
    (out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
finally:
    stop(ui);stop(daemon);log.close();socket.unlink(missing_ok=True)

#!/usr/bin/env python3
"""Actual animated switcher demo in isolated XDG, without applying wallpaper."""
import argparse,json,os,pathlib,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--theme',type=int,default=0);p.add_argument('--language',default='it');p.add_argument('--font-family',default='');a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
def run(*args):return subprocess.check_output([str(x) for x in args],text=True)
installed=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=json.loads(run(installed,'--status'))
favorites=json.loads((pathlib.Path.home()/'.config/wallpaper-lab/carousel.json').read_text());assert favorites
results={}
with tempfile.TemporaryDirectory(prefix='switcher-demo-') as temp:
    root=pathlib.Path(temp);config=root/'config/wallpaper-lab';config.mkdir(parents=True);(config/'carousel.json').write_text(json.dumps(favorites))
    for mode in range(4):
        (config/'settings.json').write_text(json.dumps({'carousel_layout':mode,'ui_theme':a.theme,'language':a.language,'font_family':a.font_family}));capture=out/str(mode);capture.mkdir(exist_ok=True)
        env=os.environ|{'XDG_CONFIG_HOME':str(root/'config'),'XDG_STATE_HOME':str(root/'state'),'WALLPAPER_LAB_POPUP_CAPTURE':str(capture)}
        with (capture/'demo.log').open('w') as log:
            child=subprocess.Popen([str(app),'--switcher-preview'],env=env,stdout=log,stderr=log)
            try:assert child.wait(timeout=12)==0,'demo failed'
            finally:
                if child.poll() is None:child.terminate();child.wait(timeout=3)
        frames=list(capture.glob('step-*.png'));steps={f.name.split('-')[1] for f in frames}
        assert len(steps)>=4,(mode,'no animation progression',steps)
        assert not (root/'state/wallpaper-lab/running.json').exists(),'demo started desktop renderer'
        assert not (root/'state/wallpaper-lab/popup/event.json').exists(),'demo took shortcut session ownership'
        results[str(mode)]={'captured_steps':len(steps),'automatic_close':True,'no_apply':True}
assert before==json.loads(run(installed,'--status')),'personal renderer changed'
results['personal_renderer_preserved']=True;(out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))

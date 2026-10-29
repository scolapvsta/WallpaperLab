#!/usr/bin/env python3
"""Read-only launch-plan checks for global AND per-wallpaper playback flags.
Requires local Wallpaper Engine assets; never launches or stops a renderer.
"""
import argparse,json,os,pathlib,subprocess
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);root=out/'library/1000';root.mkdir(parents=True,exist_ok=True);(root/'project.json').write_text(json.dumps({'type':'web','title':'Playback options fixture','file':'index.html'}));(root/'index.html').write_text('<style>html{background:#345678}</style>');config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True);env=os.environ|{'WALLPAPER_LAB_LIBRARY':str(root.parent),'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])}
keys=['audio','reactive','mouse','parallax','particles'];negative=['muted','disable_audio_processing','disable_mouse','disable_parallax','disable_particles'];flags=['--no-audio-processing','--disable-mouse','--disable-parallax','--disable-particles'];results=[]
for global_on in [False,True]:
 for local_on in [False,True]:
  (config/'settings.json').write_text(json.dumps(dict(zip(negative,[not global_on]*5))));(config/'wallpaper_profiles.json').write_text(json.dumps({str(root):{'playback':dict(zip(keys,[local_on]*5))}}));r=subprocess.run([str(app),'--plan','1000'],env=env,capture_output=True,text=True,check=True);args=json.loads(r.stdout);effective=global_on and local_on;assert args[args.index('--volume')+1]==('100'if effective else'0');assert all((flag not in args)==effective for flag in flags);results.append({'global':global_on,'wallpaper':local_on,'effective':effective})
(out/'results.json').write_text(json.dumps(results,indent=2));print('Five playback switches: all four global/local combinations passed')

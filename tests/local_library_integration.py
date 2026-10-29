#!/usr/bin/env python3
"""Local image/video import, origin, metadata and optional rendering without WE assets."""
import argparse,json,pathlib,os,subprocess,hashlib
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);p.add_argument('--desktop',action='store_true');a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);source=out/'originals';source.mkdir(exist_ok=True);env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'XDG_DATA_HOME':str(out/'data'),'XDG_CACHE_HOME':str(out/'cache'),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])}
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,text=True)
image=source/'A photo with spaces.png';Image.new('RGB',(1920,1080),(60,120,190)).save(image);video=source/'A local video.mp4';subprocess.run(['ffmpeg','-v','error','-f','lavfi','-i','color=c=red:s=320x180:d=2','-c:v','libx264','-pix_fmt','yuv420p','-y',str(video)],check=True);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest()for p in [image,video]}
paths=json.loads(run(app,'--import',image,video));again=json.loads(run(app,'--import',image));assert again==paths[:1];rows=json.loads(run(app,'--list'));items=[i for i in rows if i['path']in paths];assert len(items)==2 and all(i['origin']=='local'and not i['reason']for i in items);photo=next(i for i in items if i['type']=='image');movie=next(i for i in items if i['type']=='video');assert(photo['width'],photo['height'])==(1920,1080);assert(movie['width'],movie['height'])==(320,180);assert pathlib.Path(photo['preview']).is_file();results={'image_video_import':True,'dimensions_titles_origins':True,'deduplicated_reimport':True}
steam=out/'custom-workshop';steam.mkdir(exist_ok=True);run(app,'--library-path','steam',steam);assert len(json.loads(run(app,'--list')))==2;local=out/'custom-local';local.mkdir(exist_ok=True);run(app,'--library-path','local',local);assert not json.loads(run(app,'--list'));run(app,'--import',image);new=json.loads(run(app,'--list'));assert len(new)==1 and pathlib.Path(new[0]['path']).parent==local;results['configurable_paths_no_data_moved']=True
# Return to the first local library. Explicitly test native rendering against an empty asset dir.
run(app,'--library-path','local',pathlib.Path(paths[0]).parent)
if a.desktop:
 before=json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True));config=out/'config/wallpaper-lab/settings.json';prefs=json.loads(config.read_text());prefs.update({'mode':'single','screen':0,'fps':30,'muted':True,'transition_type':0});config.write_text(json.dumps(prefs))
 try:
  for item in [photo,movie]:
   args=json.loads(run(app,'--plan',item['id']));assert not list(pathlib.Path(args[args.index('--assets-dir')+1]).iterdir());run(app,'--apply',item['id']);status=json.loads(run(app,'--status'));assert status['alive'] and status['path']==item['path']
  results['native_image_and_video_without_WE_assets']=True
 finally:run(app,'--stop')
 assert before==json.loads(subprocess.check_output([pathlib.Path.home()/'.local/bin/wallpaper-lab','--status'],text=True))
assert all(hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()==h for p,h in hashes.items());results['original_files_preserved']=True;(out/'results.json').write_text(json.dumps(results,indent=2));print(json.dumps(results,indent=2))

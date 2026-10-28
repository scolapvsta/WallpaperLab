#!/usr/bin/env python3
"""Opt-in library selection, live inspector and persistence with a static fixture."""
import argparse,json,os,pathlib,shutil,subprocess,time
p=argparse.ArgumentParser();p.add_argument('--desktop',action='store_true');p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args()
if not a.desktop:p.error('--desktop required')
app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);project=out/'library/1';project.mkdir(parents=True,exist_ok=True)
(project/'project.json').write_text(json.dumps({'title':'Live selection fixture','type':'web','file':'index.html'}));(project/'index.html').write_text('<!doctype html><html><style>html,body{margin:0;width:100%;height:100%;background:linear-gradient(45deg,#216ca0,#c5ebad)}</style></html>')
env=os.environ|{'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'WALLPAPER_LAB_LIBRARY':str(out/'library'),'WALLPAPER_LAB_TEST_APPLY':'1','WALLPAPER_LAB_TEST_OUTPUT':str(out)}
installed=pathlib.Path.home()/'.local/bin/wallpaper-lab';before=json.loads(subprocess.check_output([str(installed),'--status'],text=True))
with (out/'ui.log').open('w') as log:
 ui=subprocess.Popen([str(app),'--ui-test'],env=env,stdout=log,stderr=log)
 try:
  assert ui.wait(timeout=45)==0,'native library failed'
  evidence=json.loads((out/'persistent-window.json').read_text());assert evidence['applied'] and evidence['frames_after_apply']>30,evidence
  status=json.loads(subprocess.check_output([str(app),'--status'],env=env,text=True));assert status['alive'] and status['path']==str(project)
  assert len(list(out.glob('view-*.png')))>=10,'missing canvas evidence'
  bad=out/'broken/99';bad.mkdir(parents=True,exist_ok=True);(bad/'project.json').write_text(json.dumps({'title':'Broken fixture','type':'scene','file':'scene.json'}));(bad/'scene.json').write_text('{invalid')
  failure=subprocess.run([str(app),'--apply','99'],env=env|{'WALLPAPER_LAB_LIBRARY':str(out/'broken')},stdout=log,stderr=log)
  assert failure.returncode!=0,'broken fixture unexpectedly rendered'
  after=json.loads(subprocess.check_output([str(app),'--status'],env=env,text=True));assert after==status,'failed scene replaced the previous renderer'
 finally:
  if ui.poll() is None:ui.terminate();ui.wait(timeout=5)
  subprocess.run([str(app),'--stop'],env=env,check=True,stdout=log,stderr=log)
assert before==json.loads(subprocess.check_output([str(installed),'--status'],text=True)),'personal renderer replaced'
result={'automatic_selection_applied':True,'library_stayed_open':True,'failed_scene_keeps_previous':True,'personal_renderer_preserved':True};(out/'results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))

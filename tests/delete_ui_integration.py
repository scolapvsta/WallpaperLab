#!/usr/bin/env python3
"""A real click on the victim during asynchronous active deletion must not stick the UI."""
import argparse,pathlib,json,subprocess,time,os,shutil
p=argparse.ArgumentParser();p.add_argument('--app',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();app=a.app.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);root=out/'library';config=out/'config/wallpaper-lab';config.mkdir(parents=True,exist_ok=True)
for n,title in [(1,'Alpha'),(2,'Beta')]:
 d=root/str(n);d.mkdir(parents=True,exist_ok=True);(d/'project.json').write_text(json.dumps({'title':title,'type':'web','file':'index.html'}));(d/'index.html').write_text('<style>html{background:#456}</style>')
(config/'settings.json').write_text(json.dumps({'mode':'single','screen':0,'muted':True,'fps':15,'fullscreen_policy':0}));(config/'carousel.json').write_text(json.dumps([str(root/'1'),str(root/'2')]))
bin_=out/'bin';bin_.mkdir(exist_ok=True);(bin_/'gio').write_text('#!/usr/bin/env python3\nimport subprocess,sys,time\ntime.sleep(1.8)\nsys.exit(subprocess.call(["'+shutil.which('gio')+'",*sys.argv[1:]]))\n');(bin_/'gio').chmod(0o755)
socket=pathlib.Path(os.environ['XDG_RUNTIME_DIR'])/f'lab-delete-{os.getpid()}.sock';statepath=out/'ui.json';env=os.environ|{'PATH':str(bin_)+os.pathsep+os.environ['PATH'],'XDG_CONFIG_HOME':str(out/'config'),'XDG_STATE_HOME':str(out/'state'),'XDG_DATA_HOME':str(out/'data'),'WALLPAPER_LAB_LIBRARY':str(root),'WALLPAPER_LAB_UI_STATE':str(statepath),'YDOTOOL_SOCKET':str(socket),'LD_LIBRARY_PATH':':'.join(str(app.parent/x)for x in ['.','lib','deps'])};(out/'data').mkdir(exist_ok=True)
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,text=True)
def wait(fn,timeout=20):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  try:v=fn()
  except(FileNotFoundError,json.JSONDecodeError,StopIteration):v=None
  if v:return v
  time.sleep(.08)
 raise AssertionError('UI condition timed out')
def state():return json.loads(statepath.read_text())
def style():return json.loads(pathlib.Path(str(statepath)+'.style').read_text())
log=open(out/'ui.log','w');ui=None;daemon=None
try:
 run(app,'--apply','1');daemon=subprocess.Popen(['ydotoold','--socket-path='+str(socket)],stdout=log,stderr=log);wait(socket.exists);time.sleep(1);ui=subprocess.Popen([str(app)],env=env,stdout=log,stderr=log)
 def window():return next(w for w in json.loads(run('hyprctl','clients','-j'))if w['pid']==ui.pid)
 wait(window);wait(lambda:state()['results']==2);m=next(m for m in json.loads(run('hyprctl','monitors','-j'))if m['name']=='DP-6');run('hyprctl','eval',f'hl.dispatch(hl.dsp.window.move({{window="pid:{ui.pid}",workspace="{m["activeWorkspace"]["id"]}",follow=false}}))');time.sleep(.5)
 def click(x,y):
  run('hyprctl','eval',f'hl.dispatch(hl.dsp.focus({{window="pid:{ui.pid}"}}))');time.sleep(.15);w=window();run('hyprctl','eval',f'hl.dispatch(hl.dsp.cursor.move({{x={int(w["at"][0]+x)},y={int(w["at"][1]+y)}}}))');time.sleep(.06);run('ydotool','click','0xC0');time.sleep(.18)
 click(44,208);wait(lambda:state()['delete_confirmation']);w=window();click((w['size'][0]-560)/2+368,(w['size'][1]-350)/2+308);wait(lambda:style()['applying']);click(100,300);assert style()['applying'];wait(lambda:not style()['applying']and state()['results']==1);s=json.loads(run(app,'--status'));assert s['alive']and s['path']==str(root/'2');assert not(root/'1').exists();assert state()['selection']==1;result={'same_victim_click_while_deleting':True,'ui_completed':True,'replacement':s['title']};(out/'results.json').write_text(json.dumps(result,indent=2));print(result)
finally:
 try:run('ydotool','click','0x80')
 except Exception:pass
 if ui and ui.poll()is None:ui.terminate();ui.wait(12)
 subprocess.run([str(app),'--stop'],env=env,stdout=log,stderr=log)
 if daemon:daemon.terminate();daemon.wait(5)
 socket.unlink(missing_ok=True)

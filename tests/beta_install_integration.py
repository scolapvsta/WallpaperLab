#!/usr/bin/env python3
"""Opt-in installer smoke in isolated HOME/XDG; uses host/extracted Arch dependency libraries."""
import argparse,json,os,pathlib,subprocess,zipfile
p=argparse.ArgumentParser();p.add_argument('--installer',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--dependency-libs',type=pathlib.Path,required=True);p.add_argument('--active-upgrade',action='store_true',help='Opt-in synthetic desktop renderer handoff, independent of personal state.');a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);home=out/'home';home.mkdir(exist_ok=True);bin_=out/'bin';bin_.mkdir(exist_ok=True)
# The host has these same Arch packages extracted rather than registered in pacman.
# Stub only pacman queries, never installs; runtime ldd/CLI and network checks remain real.
(bin_/'pacman').write_text('#!/bin/sh\n[ "$1" = "-Q" ] || exit 99\nexit 0\n');(bin_/'pacman').chmod(0o755)
env=os.environ|{'HOME':str(home),'XDG_CONFIG_HOME':str(home/'config'),'XDG_DATA_HOME':str(home/'data'),'XDG_STATE_HOME':str(home/'state'),'XDG_CACHE_HOME':str(home/'cache'),'PATH':str(bin_)+':'+os.environ['PATH'],'LD_LIBRARY_PATH':str(a.dependency_libs.resolve())}
installer=a.installer.resolve();launcher=home/'.local/bin/wallpaper-lab';results={}
def run(*args):return subprocess.check_output([str(x)for x in args],env=env,text=True,stderr=subprocess.STDOUT,timeout=180)
(out/'install.log').write_text(run('bash',installer,'--yes','--no-launch'))
receipt=home/'data/wallpaper-lab/installation.json';first=json.loads(receipt.read_text());runtime=pathlib.Path(first['runtime']);results['clean_install_cli']=('0.24.0'in run(launcher,'--version') and json.loads(run(launcher,'--list'))==[])
results['workshop_runtime_hash_verified']=(runtime/'lib/libsteam_api.so').exists()
prefs=home/'config/wallpaper-lab/settings.json';prefs.parent.mkdir(parents=True,exist_ok=True);prefs.write_text(json.dumps({'ui_design':3,'theme_scoped_v3':True,'ui_theme':17,'muted':True,'beta_sentinel':'keep'}));before=prefs.read_bytes()
old_renderer=None;personal_before=None
if a.active_upgrade:
 from PIL import Image
 real_launcher=pathlib.Path.home()/'.local/bin/wallpaper-lab';personal_before=subprocess.check_output([str(real_launcher),'--status'],text=True)
 image=out/'synthetic.png';Image.new('RGB',(640,360),(23,47,81)).save(image)
 imported=json.loads(run(launcher,'--import',image));rows=json.loads(run(launcher,'--list'));item=next(i for i in rows if i['path']==imported[0])
 settings=json.loads(prefs.read_text());settings.update({'mode':'single','screen':0,'fps':30,'transition_type':0});prefs.write_text(json.dumps(settings))
 run(launcher,'--apply',item['id']);old_renderer=json.loads(run(launcher,'--status'));assert old_renderer['alive'];before=prefs.read_bytes()
try:
 (out/'upgrade.log').write_text(run('bash',installer,'--yes','--no-launch'))
 if old_renderer:
  import time
  fresh=json.loads(run(launcher,'--status'));assert fresh['alive'] and fresh['path']==old_renderer['path'] and fresh['pid']!=old_renderer['pid']
  end=time.monotonic()+10
  while pathlib.Path('/proc',str(old_renderer['pid'])).exists() and time.monotonic()<end:time.sleep(.1)
  assert not pathlib.Path('/proc',str(old_renderer['pid'])).exists();results['actual_active_renderer_handoff']=True
  launcher_before=launcher.read_bytes();menu=home/'data/applications/wallpaper-lab.desktop';menu_before=menu.read_bytes();receipt_before=receipt.read_bytes();good_settings=prefs.read_bytes()
  bad=json.loads(good_settings);bad['path']='/missing-beta-fixture';bad['monitor_wallpapers']={k:'/missing-beta-fixture' for k in bad.get('monitor_wallpapers',{})};prefs.write_text(json.dumps(bad));bad_before=prefs.read_bytes()
  failed=subprocess.run(['bash',str(installer),'--yes','--no-launch','--skip-steam'],env=env,text=True,capture_output=True,timeout=180);(out/'failed-upgrade.log').write_text(failed.stdout+failed.stderr)
  assert failed.returncode and launcher.read_bytes()==launcher_before and menu.read_bytes()==menu_before and receipt.read_bytes()==receipt_before and prefs.read_bytes()==bad_before
  assert json.loads(run(launcher,'--status'))['pid']==fresh['pid'] and pathlib.Path('/proc',str(fresh['pid'])).exists();results['actual_failed_upgrade_rollback']=True;prefs.write_bytes(good_settings)
finally:
 if old_renderer:
  run(launcher,'--stop');assert subprocess.check_output([str(real_launcher),'--status'],text=True)==personal_before

second=json.loads(receipt.read_text());results['upgrade_keeps_preferences_and_backup']=prefs.read_bytes()==before and first['runtime']!=second['runtime'] and runtime.exists() and (pathlib.Path(second['backup'])/'launcher').exists()
(out/'check.log').write_text(run('bash',installer,'--check'));results['check_mode']=True
runtime=pathlib.Path(second['runtime']);linked=run('ldd',runtime/'wallpaper-lab');(out/'ldd.log').write_text(linked)
results['runtime_no_build_rpath']=not any(x in run('readelf','-d',runtime/'wallpaper-lab')+run('readelf','-d',runtime/'liblinux-wallpaperengine-lib.so') for x in ['/home/','build-engine','local/sysroot'])
results['actual_steam_query']=False
# Steam IPC needs the actual user's home. Keep all app XDG directories isolated.
steam_env=env|{'HOME':os.environ['HOME']}
reply=subprocess.run([str(launcher),'--workshop-search','{"text":"landscape","tags":[],"any":false}'],env=steam_env,text=True,capture_output=True,timeout=60,check=True);query=reply.stdout;(out/'steam-stderr.log').write_text(reply.stderr)
(out/'workshop.log').write_text(query);j=json.loads(query);assert not j.get('error'),j.get('error');results['actual_steam_query']=True
(out/'runtime-path.txt').write_text(str(runtime))
results['uninstall_keeps_user_data']=False
# Keep the runtime for the separate graphical tests; uninstall launcher/menu only.
(out/'uninstall.log').write_text(run('bash',installer,'--uninstall','--yes'));results['uninstall_keeps_user_data']=not launcher.exists() and prefs.read_bytes()==before and runtime.exists()
(out/'results.json').write_text(json.dumps(results,indent=2));print(json.dumps(results,indent=2));assert all(results.values())

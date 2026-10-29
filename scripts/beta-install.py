#!/usr/bin/env python3
"""Bootstrap embedded in the beta .run. Installs only allowlisted, verified payload files."""
import argparse, ctypes, fcntl, hashlib, io, json, os, pathlib, platform, shlex
import shutil, signal, subprocess, sys, tarfile, tempfile, time, urllib.request, zipfile
P=pathlib.Path

def sha(data):return hashlib.sha256(data).hexdigest()
def ask(question,yes=False):
 if yes:return True
 if not sys.stdin.isatty():raise RuntimeError('Run in a terminal, or pass --yes for unattended setup.')
 return input(question+' [Y/n] ').strip().lower() in ('','y','yes','s','si','sì')
def atomic(path,data,mode=0o644):
 path.parent.mkdir(parents=True,exist_ok=True);tmp=path.with_name(path.name+'.new')
 with tmp.open('w') as f:f.write(data);f.flush();os.fsync(f.fileno())
 tmp.chmod(mode);os.replace(tmp,path)
def library_env(runtime):
 return os.environ|{'LD_LIBRARY_PATH':str(runtime)+':'+str(runtime/'lib')+(':'+os.environ['LD_LIBRARY_PATH'] if os.environ.get('LD_LIBRARY_PATH') else '')}
def output(runtime,*args):return subprocess.check_output([str(runtime/'wallpaper-lab'),*args],env=library_env(runtime),text=True,timeout=60)
def process_nodes(state):return list(state['outputs'].values()) if 'outputs'in state else [state] if 'pid'in state else []
def identity(node,exe):
 try:
  proc=P('/proc',str(node['pid']));args=(proc/'cmdline').read_bytes().split(b'\0')
  return node['pid']>1 and (proc/'stat').read_text().rsplit(')',1)[1].split()[19]==node['start'] and os.readlink(proc/'exe')==str(exe) and b'--render'in args
 except (OSError,KeyError):return False

def verify(z):
 m=json.loads(z.read('manifest.json'));expected=set(m['files'])|{'manifest.json'}
 if len(z.namelist())!=len(expected) or set(z.namelist())!=expected:raise RuntimeError('Archive has unexpected/missing entries.')
 for name,digest in m['files'].items():
  path=P(name)
  if path.is_absolute() or '..'in path.parts or sha(z.read(name))!=digest:raise RuntimeError('Invalid payload: '+name)
 return m

def steam_runtime(stage,config):
 print('Downloading optional Valve Steamworks runtime (steamworks.js '+config['version']+').')
 print('Valve component terms: '+config['terms'])
 with urllib.request.urlopen(config['url'],timeout=60) as response:
  data=response.read(64*1024*1024+1)
 if len(data)>64*1024*1024 or sha(data)!=config['archive_sha256']:raise RuntimeError('Steam archive checksum mismatch; installation cancelled.')
 with tarfile.open(fileobj=io.BytesIO(data),mode='r:gz') as archive:
  member=archive.getmember(config['member'])
  if not member.isfile() or member.size>4*1024*1024:raise RuntimeError('Invalid Steam library entry.')
  lib=archive.extractfile(member).read()
 if sha(lib)!=config['library_sha256']:raise RuntimeError('Steam library checksum mismatch.')
 target=stage/'lib/libsteam_api.so';target.write_bytes(lib)
 handle=ctypes.CDLL(str(target))
 for name in ['SteamAPI_InitFlat','SteamAPI_SteamUGC_v020','SteamAPI_ManualDispatch_Init']:getattr(handle,name)
 (stage/'licenses/STEAMWORKS.txt').write_text(config['notice']+'\n'+config['terms']+'\nSource: '+config['url']+'\n')

def prerequisite_check(deps,install=False,yes=False):
 if platform.system()!='Linux' or platform.machine()!='x86_64':raise RuntimeError('This beta requires Arch Linux x86_64.')
 osrelease=platform.freedesktop_os_release()
 if osrelease.get('ID')!='arch' and 'arch'not in osrelease.get('ID_LIKE','').split():raise RuntimeError('This installer targets Arch Linux and Arch derivatives only.')
 required=tuple(map(int,deps['minimum_glibc'].split('.')))
 libc=platform.libc_ver()[1]
 if tuple(map(int,libc.split('.')[:2]))<required:raise RuntimeError('Update Arch first (sudo pacman -Syu). Required glibc >= '+deps['minimum_glibc'])
 missing=[pkg for pkg in deps['packages'] if subprocess.run(['pacman','-Q',pkg],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode]
 if missing:
  cmd=['sudo','pacman','-S','--needed',*missing]
  print('Missing Arch packages:\n  '+shlex.join(cmd))
  if not install:raise RuntimeError('Install these packages, or run this installer without --check.')
  if not ask('Install these dependencies with pacman? (no full system upgrade)',yes):raise RuntimeError('Dependencies not installed.')
  subprocess.run(cmd,check=True)
 for cmd in deps['required_commands']:
  if not shutil.which(cmd):raise RuntimeError('Required command missing: '+cmd)
 print(deps['desktop'])
 if not os.environ.get('HYPRLAND_INSTANCE_SIGNATURE'):print('Notice: full desktop support has been tested on Hyprland only.')

def validate_runtime(stage,version):
 # No development RPATH fallback: CMake installation rewrites app and engine paths.
 env=library_env(stage)
 check=subprocess.run(['ldd',str(stage/'wallpaper-lab')],env=env,text=True,capture_output=True,check=True)
 if 'not found'in check.stdout:raise RuntimeError('Unresolved runtime libraries:\n'+check.stdout)
 if version not in output(stage,'--version'):raise RuntimeError('Runtime version mismatch.')
 output(stage,'--list')

def uninstall(base,launcher,data,yes):
 receipt=base/'installation.json'
 if not receipt.is_file():raise RuntimeError('No beta installer receipt found. Existing developer installs are left untouched.')
 state=json.loads(receipt.read_text());runtime=P(state['runtime'])
 if runtime.parent!=base/'releases':raise RuntimeError('Invalid installation receipt.')
 if not ask('Remove Wallpaper Lab? Your wallpapers and preferences will be kept.',yes):return
 subprocess.run([str(launcher),'--stop'],check=True)
 for path in [launcher,data/'applications/wallpaper-lab.desktop',data/'icons/hicolor/scalable/apps/wallpaper-lab.svg']:
  if path.exists():path.unlink()
 # Older release directories are backups and may still be used by open UI processes.
 receipt.unlink();print('Uninstalled menu/launcher. Settings and release backups kept in '+str(base))

def install(z,m,args):
 home=P.home();data=P(os.environ.get('XDG_DATA_HOME',home/'.local/share')).resolve();base=data/'wallpaper-lab';launcher=home/'.local/bin/wallpaper-lab'
 base.mkdir(parents=True,exist_ok=True)
 with (base/'installer.lock').open('a') as lock:
  try:fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
  except BlockingIOError:raise RuntimeError('Another installer is running.')
  if args.uninstall:return uninstall(base,launcher,data,args.yes)
  deps=json.loads(z.read('arch-dependencies.json'));prerequisite_check(deps,install=not args.check,yes=args.yes)
  with tempfile.TemporaryDirectory(prefix='.beta-stage-',dir=base) as tmp:
   extracted=P(tmp)
   for name in m['files']:
    if not name.startswith('runtime/'):continue
    dest=extracted/P(name).relative_to('runtime');dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(z.read(name))
    if name=='runtime/wallpaper-lab':dest.chmod(0o755)
   if not args.skip_steam and not args.check:
    if ask('Prepare Steam Workshop support by downloading the separate Valve runtime?',args.yes):steam_runtime(extracted,json.loads(z.read('steam-runtime.json')))
    else:print('Workshop search/download will need libsteam_api.so; local media remain available.')
   validate_runtime(extracted,m['version'])
   if args.check:print('Integrity, dependencies and runtime smoke checks passed. No installation changed.');return
   if not ask('Install Wallpaper Lab '+m['release']+' in your user profile?',args.yes):return
   old_launcher=launcher.read_text() if launcher.is_file() else None
   old={};old_exe=None
   if old_launcher:
    # Running state contains no unverified executable path: resolve via proc and validate below.
    result=subprocess.run([str(launcher),'--status'],text=True,capture_output=True,timeout=60)
    if result.returncode:raise RuntimeError('Existing installation cannot report status. Resolve that before upgrading.')
    old=json.loads(result.stdout)
    if old.get('alive'):
     for node in process_nodes(old):
      if not P('/proc',str(node['pid'])).exists():continue
      exe=P(os.readlink(P('/proc',str(node['pid']),'exe')))
      if not (exe==base/'runtime/wallpaper-lab' or exe.parent.parent==base/'releases') or not identity(node,exe):raise RuntimeError('Cannot safely identify the existing renderer.')
      if old_exe and exe!=old_exe:raise RuntimeError('Mixed active runtimes; close them before upgrading.')
      old_exe=exe
   releases=base/'releases';releases.mkdir(exist_ok=True);runtime=releases/(m['release']+'-'+str(time.time_ns()));extracted.rename(runtime)
   backups=base/'installer-backups'/str(time.time_ns());backups.mkdir(parents=True)
   if old_launcher:(backups/'launcher').write_text(old_launcher)
   state_dir=P(os.environ.get('XDG_STATE_HOME',home/'.local/state'))/'wallpaper-lab';running=state_dir/'running.json';running_before=running.read_bytes() if running.exists() else None
   config_file=P(os.environ.get('XDG_CONFIG_HOME',home/'.config'))/'wallpaper-lab/settings.json';config_before=config_file.read_bytes() if config_file.exists() else None
   new_live=False;restore_attempted=False;resources_written=False
   try:
    if old.get('alive'):
     restore_attempted=True
     print('Starting the replacement wallpaper before stopping the previous renderer…')
     with (backups/'restore.log').open('w') as log:subprocess.run([str(runtime/'wallpaper-lab'),'--restore'],env=library_env(runtime),stdout=log,stderr=log,check=True,timeout=90)
     fresh=json.loads(output(runtime,'--status'));new_live=fresh.get('alive',False)
     if not new_live or {n['path']for n in process_nodes(fresh)}!={n['path']for n in process_nodes(old)}:raise RuntimeError('Replacement renderer not ready on the same wallpapers.')
    launcher_text='#!/bin/sh\nruntime_dir='+shlex.quote(str(runtime))+'\nexport LD_LIBRARY_PATH="$runtime_dir:$runtime_dir/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"\ncd "$runtime_dir" || exit 1\nexec "$runtime_dir/wallpaper-lab" "$@"\n'
    atomic(launcher,launcher_text,0o755)
    desktop=data/'applications/wallpaper-lab.desktop';icon=data/'icons/hicolor/scalable/apps/wallpaper-lab.svg'
    # Preserve prior menu resources before replacing them.
    for f in [desktop,icon]:
     if f.exists():shutil.copy2(f,backups/f.name)
    desktop_exec=str(launcher).replace('\\','\\\\').replace('"','\\"').replace('`','\\`').replace('$','\\$').replace('%','%%')
    resources_written=True
    atomic(desktop,'[Desktop Entry]\nType=Application\nName=Wallpaper Lab (Open Beta)\nComment=Animated wallpapers and themed carousel\nExec="'+desktop_exec+'"\nIcon=wallpaper-lab\nTerminal=false\nCategories=Settings;DesktopSettings;\nStartupWMClass=wallpaper-lab\n')
    atomic(icon,z.read('wallpaper-lab.svg').decode())
    atomic(base/'installation.json',json.dumps({'version':m['release'],'runtime':str(runtime),'backup':str(backups)},indent=2)+'\n')
    # Stop only renderers whose PID, start time, executable and --render still match.
    if old_exe:
     for node in process_nodes(old):
      if identity(node,old_exe):
       try:os.kill(node['pid'],signal.SIGTERM)
       except OSError as error:print('Previous renderer could not be retired: '+str(error),file=sys.stderr)
   except Exception:
    # Keep old processes and restore their state record if the replacement failed.
    if restore_attempted:subprocess.run([str(runtime/'wallpaper-lab'),'--stop'],env=library_env(runtime),stdout=subprocess.DEVNULL)
    if running_before is not None:running.parent.mkdir(parents=True,exist_ok=True);running.write_bytes(running_before)
    elif running.exists():running.unlink()
    if old_launcher is not None:atomic(launcher,old_launcher,0o755)
    elif launcher.exists():launcher.unlink()
    if config_before is not None:config_file.write_bytes(config_before)
    if resources_written:
     for f in [data/'applications/wallpaper-lab.desktop',data/'icons/hicolor/scalable/apps/wallpaper-lab.svg']:
      if (backups/f.name).exists():shutil.copy2(backups/f.name,f)
      elif f.exists():f.unlink()
    raise
   print('Installed '+m['release']+'. Open Wallpaper Lab from the applications menu.\nLauncher: '+str(launcher))
   print('Previous runtime backups and all preferences kept. Autostart and global shortcuts unchanged.')
   if not args.no_launch:
    log_dir=state_dir;log_dir.mkdir(parents=True,exist_ok=True)
    with (log_dir/'beta-launch.log').open('a') as log:subprocess.Popen([str(launcher)],stdout=log,stderr=log,start_new_session=True)

def main():
 parser=argparse.ArgumentParser(description='Wallpaper Lab Open Beta — Arch Linux x86_64 installer')
 for flag,help_ in [('yes','Accept installer prompts (pacman may still request authentication/confirmation).'),('check','Verify payload, dependencies and CLI without installing.'),('no-launch','Do not open the UI after installation.'),('skip-steam','Do not download the optional Valve Workshop runtime.'),('uninstall','Remove beta launcher/menu; retain data and runtime backups.')]:parser.add_argument('--'+flag,action='store_true',help=help_)
 args=parser.parse_args(sys.argv[2:])
 if os.geteuid()==0:raise RuntimeError('Run as your desktop user, without sudo. Only dependency installation uses sudo.')
 with zipfile.ZipFile(sys.argv[1]) as z:
  print('Checking installer integrity…');m=verify(z);install(z,m,args)
if __name__=='__main__':
 try:main()
 except (RuntimeError,OSError,ValueError,subprocess.SubprocessError,zipfile.BadZipFile) as e:print('Installation failed: '+str(e),file=sys.stderr);sys.exit(1)

#!/usr/bin/env python3
"""One-time, reversible migration after the new application's renderer is verified."""
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import time

home=Path.home()
root=Path(__file__).resolve().parents[1]
backup=root/'local'/('before-takeover-'+time.strftime('%Y%m%d-%H%M%S'))
backup.mkdir(parents=True)
old=home/'.config/Linux Wallpaper Engine/active-wallpapers.json'
state=json.loads(old.read_text())
active=list(state['activeWallpapers'].values())
paths={x['backgroundId'] for x in active}
if len(paths)!=1:raise SystemExit('Multiple different active wallpapers: migrate manually to preserve the arrangement.')
path=next(iter(paths))
settings={'path':path,'mode':'span' if len(active)>1 else 'single','screen':0,'fps':active[0].get('fps',60),'muted':True,'disable_mouse':True,'disable_parallax':True,'disable_particles':True}
gui=[];engines=[]
def identity(proc):
    return ((proc/'stat').read_text().rsplit(')',1)[1].split()[19], str((proc/'exe').resolve()))
for proc in Path('/proc').iterdir():
    if not proc.name.isdigit():continue
    try:args=(proc/'cmdline').read_bytes().decode().strip('\0').split('\0')
    except (OSError,UnicodeError):continue
    if not args:continue
    if args[0].startswith(str(home/'.local/share/wallpaperx/')):
        if args[0].endswith('/linux-wallpaperengine'):
            engines.append((int(proc.name),identity(proc)))
            if '--bg' in args and args[args.index('--bg')+1]==path:
                for key,flag in [('disable_mouse','--disable-mouse'),('disable_parallax','--disable-parallax'),('disable_particles','--disable-particles')]:settings[key]=flag in args
                if '--volume' in args:settings['muted']=args[args.index('--volume')+1]=='0'
        elif args[0].endswith('/linux-wallpaper-engine') and not any(x.startswith('--type=') for x in args):gui.append((int(proc.name),identity(proc)))
config=home/'.config/wallpaper-lab/settings.json'
config.parent.mkdir(parents=True,exist_ok=True)
if config.exists():shutil.copy2(config,backup/'settings.json')
shutil.copy2(old,backup/'wallpaperx-active.json')
autostart=home/'.config/autostart/Linux Wallpaper Engine.desktop'
if autostart.exists():shutil.copy2(autostart,backup/'Linux Wallpaper Engine.desktop')
config.write_text(json.dumps(settings,indent=2)+'\n')
launcher=home/'.local/bin/wallpaper-lab'
result=subprocess.run([str(launcher),'--restore'])
if result.returncode:
    if (backup/'settings.json').exists():shutil.copy2(backup/'settings.json',config)
    else:config.unlink(missing_ok=True)
    raise SystemExit('New renderer failed; WallpaperX was left running.')
for pid,expected in gui+engines:
    try:
        if identity(Path('/proc')/str(pid))==expected:os.kill(pid,signal.SIGTERM)
    except (ProcessLookupError,FileNotFoundError):pass
if autostart.exists():autostart.rename(backup/'original-autostart.desktop')
new_autostart=home/'.config/autostart/wallpaper-lab.desktop'
new_autostart.write_text(f'''[Desktop Entry]
Type=Application
Name=Wallpaper Lab background
Exec="{launcher}" --restore
Terminal=false
X-GNOME-Autostart-enabled=true
''')
(backup/'rollback.txt').write_text('Stop Wallpaper Lab with ~/.local/bin/wallpaper-lab --stop. Remove ~/.config/autostart/wallpaper-lab.desktop. Copy original-autostart.desktop back to ~/.config/autostart/Linux Wallpaper Engine.desktop. Launch ~/.local/bin/wallpaperx.\n')
print(f'Migrated {path}. Local rollback backup: {backup}')

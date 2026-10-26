#!/usr/bin/env python3
"""Install this application's own runtime. Never installs or invokes the upstream executables."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

root = Path(__file__).resolve().parents[1]
home = Path.home()
base = home / '.local/share/wallpaper-lab'
runtime = base / 'runtime'
launcher = home / '.local/bin/wallpaper-lab'
restore = False
if launcher.exists():
    status = subprocess.run([str(launcher), '--status'], capture_output=True, text=True)
    if status.returncode == 0:
        restore = json.loads(status.stdout).get('alive', False)
stage = base / f'stage-{time.time_ns()}'
stage.mkdir(parents=True)
subprocess.run(['cmake', '--install', str(root/'build'), '--prefix', str(stage), '--component', 'app'], check=True)
subprocess.run(['cmake', '--install', str(root/'build-engine'), '--prefix', str(stage), '--component', 'engine-library'], check=True)
output = root / 'build-engine/output'
for name in ['libcef.so','libEGL.so','libGLESv2.so','libvk_swiftshader.so','v8_context_snapshot.bin','vk_swiftshader_icd.json','chrome_100_percent.pak','chrome_200_percent.pak','resources.pak','icudtl.dat','locales']:
    source=output/name
    if source.is_dir(): shutil.copytree(source,stage/name)
    else: shutil.copy2(source,stage/name)
(stage/'lib').mkdir()
for source in (root/'build-engine/lib').glob('libkissfft*'):
    shutil.copy2(source,stage/'lib'/source.name,follow_symlinks=True)
portable=root/'local/sysroot/usr/lib'
if portable.exists():
    (stage/'deps').mkdir()
    for source in portable.iterdir():
        if source.is_file() and '.so' in source.name: shutil.copy2(source,stage/'deps'/source.name,follow_symlinks=True)
for source in [stage/'wallpaper-lab',stage/'liblinux-wallpaperengine-lib.so',stage/'libcef.so']:
    subprocess.run(['strip','--strip-unneeded',str(source)],check=True)
license_dir=stage/'licenses';license_dir.mkdir()
shutil.copy2(root/'LICENSE',license_dir/'GPL-3.0.txt')
shutil.copy2(root/'THIRD_PARTY.md',license_dir/'THIRD_PARTY.md')
for name in ['LICENSE.hellpaper','LICENSE.raylib']:shutil.copy2(root/'assets'/name,license_dir/name)
cef=next((root/'build-engine/cef').glob('cef_binary_*_linux64_minimal'))
for name in ['LICENSE.txt','README.txt']:
    if (cef/name).exists():shutil.copy2(cef/name,license_dir/('CEF-'+name))
# Verify the staged binary and all its dynamic libraries before stopping the active wallpaper.
env=os.environ.copy()
env['LD_LIBRARY_PATH']=':'.join(str(stage/x) for x in ['.','lib','deps'])
subprocess.run([str(stage/'wallpaper-lab'),'--list'],env=env,stdout=subprocess.DEVNULL,check=True)
if launcher.exists():subprocess.run([str(launcher),'--stop'],check=True)
if runtime.exists():runtime.rename(base/f'previous-{time.time_ns()}')
stage.rename(runtime)
launcher.parent.mkdir(parents=True,exist_ok=True)
launcher.write_text('''#!/bin/sh
runtime_dir="$HOME/.local/share/wallpaper-lab/runtime"
export LD_LIBRARY_PATH="$runtime_dir:$runtime_dir/lib:$runtime_dir/deps${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$runtime_dir" || exit 1
exec "$runtime_dir/wallpaper-lab" "$@"
''')
launcher.chmod(0o755)
applications=home/'.local/share/applications';applications.mkdir(parents=True,exist_ok=True)
(applications/'wallpaper-lab.desktop').write_text(f'''[Desktop Entry]
Type=Application
Name=Wallpaper Lab
Comment=Selettore animato e scene Steam Workshop
Exec="{launcher}"
Icon=preferences-desktop-wallpaper
Terminal=false
Categories=Settings;DesktopSettings;
StartupWMClass=wallpaper-lab
''')
if restore: subprocess.run([str(launcher),'--restore'],check=True)
print(f'Installed: {launcher}')

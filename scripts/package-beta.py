#!/usr/bin/env python3
"""Build an allowlisted Arch beta installer from build outputs, never personal runtime data."""
import argparse, hashlib, json, os, pathlib, shutil, subprocess, tempfile, zipfile
ROOT = pathlib.Path(__file__).resolve().parents[1]
CEF_FILES = ['libcef.so','libEGL.so','libGLESv2.so','libvk_swiftshader.so',
 'v8_context_snapshot.bin','vk_swiftshader_icd.json','chrome_100_percent.pak',
 'chrome_200_percent.pak','resources.pak','icudtl.dat','locales']

def run(*args): subprocess.run([str(a) for a in args], check=True)
def digest(path): return hashlib.file_digest(path.open('rb'), 'sha256').hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=pathlib.Path,default=ROOT/'local/releases');p.add_argument('--app-build',type=pathlib.Path,default=ROOT/'build');p.add_argument('--engine-build',type=pathlib.Path,default=ROOT/'build-engine');a=p.parse_args()
 manifest=json.loads((ROOT/'build-manifest.json').read_text());version=manifest['version'];tag=f'{version}-beta.1';a.output.mkdir(parents=True,exist_ok=True)
 with tempfile.TemporaryDirectory(prefix='wallpaper-lab-package-') as tmp:
  stage=pathlib.Path(tmp);runtime=stage/'runtime'
  run('cmake','--install',a.app_build,'--prefix',runtime,'--component','app')
  run('cmake','--install',a.engine_build,'--prefix',runtime,'--component','engine-library')
  for name in CEF_FILES:
   source=a.engine_build/'output'/name
   if source.is_dir(): shutil.copytree(source,runtime/name)
   else: shutil.copy2(source,runtime/name)
  (runtime/'lib').mkdir()
  # Only the FFT library built from the pinned source. System dependencies come from pacman.
  for source in (a.engine_build/'lib').glob('libkissfft*.so*'):
   if source.is_file():shutil.copy2(source,runtime/'lib'/source.name,follow_symlinks=True)
  for name in ['wallpaper-lab','liblinux-wallpaperengine-lib.so','libcef.so']:run('strip','--strip-unneeded',runtime/name)
  licenses=runtime/'licenses';licenses.mkdir()
  for source,dest in [(ROOT/'LICENSE','GPL-3.0.txt'),(ROOT/'THIRD_PARTY.md','THIRD_PARTY.md'),(ROOT/'assets/LICENSE.hellpaper','LICENSE.hellpaper'),(ROOT/'assets/LICENSE.raylib','LICENSE.raylib')]:shutil.copy2(source,licenses/dest)
  cef=next((a.engine_build/'cef').glob('cef_binary_*_linux64_minimal'))
  for name in ['LICENSE.txt','CREDITS.html','README.txt']:shutil.copy2(cef/name,licenses/('CEF-'+name))
  shutil.copytree(ROOT/'vendor/engine/src/External/kissfft/LICENSES',licenses/'kissfft')
  shutil.copy2(ROOT/'assets/wallpaper-lab.svg',stage/'wallpaper-lab.svg')
  for name in ['arch-dependencies.json','steam-runtime.json']:shutil.copy2(ROOT/'packaging'/name,stage/name)
  shutil.copy2(ROOT/'scripts/beta-install.py',stage/'beta-install.py')
  shutil.copy2(ROOT/'docs/BETA.md',stage/'BETA.md')
  files={str(f.relative_to(stage)):digest(f) for f in sorted(stage.rglob('*')) if f.is_file()}
  release={'version':version,'release':tag,'platform':'archlinux-x86_64','source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'files':files,'build':manifest}
  (stage/'manifest.json').write_text(json.dumps(release,indent=2)+'\n')
  target=a.output/f'Wallpaper-Lab-{tag}-Arch-x86_64.run'
  # zipfile supports a shell preamble; no offset calculations or shell extraction.
  preamble=b'''#!/bin/sh
command -v python3 >/dev/null 2>&1 || { echo "Install Python first: sudo pacman -S python" >&2; exit 1; }
exec python3 -c 'import sys,zipfile; z=zipfile.ZipFile(sys.argv[1]); exec(compile(z.read("beta-install.py"), "beta-install.py", "exec"))' "$0" "$@"
'''
  target.write_bytes(preamble)
  with zipfile.ZipFile(target,'a',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
   for file in sorted(stage.rglob('*')):
    if file.is_file():z.write(file,str(file.relative_to(stage)))
  target.chmod(0o755)
  (a.output/'SHA256SUMS').write_text(f'{digest(target)}  {target.name}\n')
  print(f'Installer: {target} ({target.stat().st_size//1024//1024} MiB)')
if __name__=='__main__':main()

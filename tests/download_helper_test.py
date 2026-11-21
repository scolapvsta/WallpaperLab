import os,pathlib,subprocess,tempfile,hashlib,json
script=pathlib.Path(__file__).resolve().parents[1]/'scripts/download-install.sh'
version=json.loads((script.parent.parent/'build-manifest.json').read_text())['version']
name='Wallpaper-Lab-'+version+'-Arch-x86_64.run'
with tempfile.TemporaryDirectory() as d:
 p=pathlib.Path(d);payload=b'#!/bin/sh\necho helper-installer-invoked\n';(p/'payload').write_bytes(payload)
 (p/'SHA256SUMS').write_text(hashlib.sha256(payload).hexdigest()+'  '+name+'\n')
 (p/'curl').write_text('''#!/usr/bin/env python3
import os,sys,shutil
from pathlib import Path
args=sys.argv[1:];dest=args[args.index('-o')+1];source='SHA256SUMS' if dest=='SHA256SUMS' else 'payload'
expected=os.environ['RELEASE_URL']+'/'+('SHA256SUMS' if dest=='SHA256SUMS' else os.environ['INSTALLER_NAME'])
assert args[-1]==expected, args[-1]
shutil.copyfile(Path(os.environ['FIXTURE'])/source,dest)
''');(p/'curl').chmod(0o755)
 env=os.environ|{'PATH':d+':'+os.environ['PATH'],'FIXTURE':d,'RELEASE_URL':'https://github.com/scolapvsta/WallpaperLab/releases/download/v'+version,'INSTALLER_NAME':name}
 r=subprocess.run(['sh',str(script),'--no-launch'],env=env,text=True,capture_output=True);assert r.returncode==0 and 'helper-installer-invoked' in r.stdout
 (p/'payload').write_bytes(payload+b'# tampered\n')
 r=subprocess.run(['sh',str(script)],env=env,text=True,capture_output=True);assert r.returncode!=0 and 'helper-installer-invoked' not in r.stdout
 (p/'payload').write_bytes(payload);(p/'SHA256SUMS').write_text('0'*64+'  unrelated.run\n')
 r=subprocess.run(['sh',str(script)],env=env,text=True,capture_output=True);assert r.returncode!=0 and 'helper-installer-invoked' not in r.stdout
 print('PASS: helper downloads/verifies/executes; tampered installer rejected before execution')

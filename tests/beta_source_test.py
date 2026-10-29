#!/usr/bin/env python3
"""Opt-in export audit: full source, safe paths, standalone patch application and exact patched inputs."""
import argparse,hashlib,json,pathlib,re,subprocess,tarfile,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--archive',type=pathlib.Path,required=True);a=p.parse_args()
with tempfile.TemporaryDirectory(prefix='wallpaper-beta-source-')as d:
 with tarfile.open(a.archive)as t:
  names=t.getnames();assert not any('/local/'in x or '/.git/'in x or x.startswith('/')or '..'in pathlib.PurePosixPath(x).parts for x in names);t.extractall(d,filter='data')
 root=next(pathlib.Path(d).iterdir());m=json.loads((root/'build-manifest.json').read_text());assert (root/'.source-release.json').exists();assert not (root/'.git').exists()
 for name,h in m['patches'].items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==h,name
 patches=[k for k in m['patches'] if 'raylib'not in k];cmd=['python3',str(root/'scripts/apply-patch-series.py'),str(root/'vendor/engine')]+[str(root/x)for x in patches]
 r=subprocess.run(cmd,capture_output=True,text=True);assert r.returncode==0,r.stdout+r.stderr
 r=subprocess.run(['git','apply',str(root/'patches/raylib-app-id.patch')],cwd=root/'vendor/raylib',capture_output=True,text=True);assert r.returncode==0,r.stderr
 checked=0
 for name in m['patches']:
  vendor='raylib'if'raylib'in name else'engine'
  for relative in set(re.findall(r'^\+\+\+ b/([^\t\n]+)',(root/name).read_text(),re.M)):
   exported=root/'vendor'/vendor/relative;local=ROOT/'vendor'/vendor/relative
   assert exported.read_bytes()==local.read_bytes(),str(exported);checked+=1
 print(json.dumps({'archive_members':len(names),'safe_paths':True,'standalone_patch_application':True,'patched_inputs_identical_to_built_worktree':checked},indent=2))

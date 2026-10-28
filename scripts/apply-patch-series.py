#!/usr/bin/env python3
"""Validate an overlapping patch series in a disposable tree before changing files.
Already-applied series are checked in reverse order, preserving local edits.
"""
import argparse,pathlib,shutil,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');p.add_argument('directory',type=pathlib.Path);p.add_argument('patches',nargs='+',type=pathlib.Path);a=p.parse_args();directory=a.directory.resolve();patches=[p.resolve() for p in a.patches]
paths=set()
for patch in patches:
 for line in patch.read_text().splitlines():
  if line.startswith(('--- a/','+++ b/')):
   name=line[6:].split('\t')[0];rel=pathlib.Path(name)
   if rel.is_absolute() or '..' in rel.parts:raise SystemExit('Unsafe patch path')
   paths.add(rel)
def apply(root,patch,reverse=False,check=False):
 return subprocess.run(['git','apply',*(['--reverse'] if reverse else []),*(['--check'] if check else []),str(patch)],cwd=root,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
def copy_sources(root):
 for path in paths:
  source=directory/path
  if source.is_symlink():raise SystemExit('Refusing patched symbolic link: '+str(path))
  if source.is_file():dest=root/path;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,dest)
with tempfile.TemporaryDirectory(prefix='wallpaper-patches-') as temp:
 root=pathlib.Path(temp);copy_sources(root)
 reversed_ok=True
 for patch in reversed(patches):
  if apply(root,patch,True).returncode:reversed_ok=False;break
 if reversed_ok:print('Patch series already applied and verified');raise SystemExit(0)
with tempfile.TemporaryDirectory(prefix='wallpaper-patches-') as temp:
 root=pathlib.Path(temp);copy_sources(root);needed=[]
 for patch in patches:
  if apply(root,patch,check=True).returncode==0:
   result=apply(root,patch);assert result.returncode==0;needed.append(patch)
  elif apply(root,patch,reverse=True,check=True).returncode:
   raise SystemExit('Patch series conflicts with local source; files preserved: '+str(patch))
 for patch in reversed(patches):
  result=apply(root,patch,reverse=True)
  if result.returncode:raise SystemExit('Patch series is inconsistent; files preserved: '+str(patch)+'\n'+result.stderr.decode())
 if not a.check:
  for patch in needed:
   result=apply(directory,patch,check=True)
   if result.returncode:raise SystemExit('Source changed during validation; stopped: '+result.stderr.decode())
   result=apply(directory,patch)
   if result.returncode:raise SystemExit(result.stderr.decode())
 print(('Validated' if a.check else 'Applied')+' '+str(len(needed))+' patches; local edits preserved')

#!/usr/bin/env python3
"""Full source archive: tracked app files + pristine pinned submodules + application patches."""
import argparse,hashlib,io,json,pathlib,subprocess,tarfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
def records(repo):
 for item in subprocess.check_output(['git','ls-files','--stage','-z'],cwd=repo).split(b'\0'):
  if item:
   meta,name=item.decode().split('\t',1);mode,oid,stage=meta.split();yield mode,oid,name

def normalized(info,name):
 info.name=name;info.uid=info.gid=0;info.uname=info.gname='';info.mtime=0;info.pax_headers={};return info

def pinned(t,repo,prefix,expected):
 if not (repo/'.git').exists():
  if repo.name=='test262':return # Unused upstream conformance corpus, not a build input.
  raise RuntimeError('Initialize required submodule: '+str(repo))
 actual=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
 if actual!=expected:raise RuntimeError('Submodule pin mismatch: '+str(repo))
 # Archive Git objects rather than the patched worktree: patch-created untracked
 # files are reconstructed by build.sh, keeping the exported source reproducible.
 data=subprocess.check_output(['git','archive','--format=tar',actual],cwd=repo)
 with tarfile.open(fileobj=io.BytesIO(data))as source:
  for info in source:
   stream=source.extractfile(info)if info.isfile()else None
   t.addfile(normalized(info,prefix+'/'+info.name),stream)
 for mode,oid,name in records(repo):
  if mode=='160000':pinned(t,repo/name,prefix+'/'+name,oid)

def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=pathlib.Path,default=ROOT/'local/releases');a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
 v=json.loads((ROOT/'build-manifest.json').read_text())['version']+'-beta.1';target=a.output/f'Wallpaper-Lab-{v}-source.tar.xz';prefix='Wallpaper-Lab-'+v
 with tarfile.open(target,'w:xz',preset=6)as t:
  metadata=json.dumps({'release':v,'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'patches_already_applied':False}).encode();info=tarfile.TarInfo(prefix+'/.source-release.json');info.size=len(metadata);info.mode=0o644;t.addfile(info,io.BytesIO(metadata))
  for mode,oid,name in records(ROOT):
   if mode=='160000':pinned(t,ROOT/name,prefix+'/'+name,oid)
   else:
    source=ROOT/name;info=normalized(t.gettarinfo(str(source)),prefix+'/'+name)
    with source.open('rb')if info.isfile()else io.BytesIO()as f:t.addfile(info,f if info.isfile()else None)
 with (a.output/'SHA256SUMS').open('a')as sums:sums.write(hashlib.file_digest(target.open('rb'),'sha256').hexdigest()+'  '+target.name+'\n')
 print(target)
if __name__=='__main__':main()

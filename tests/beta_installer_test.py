#!/usr/bin/env python3
"""Installer contract tests. No system packages, personal state or renderers are touched."""
import importlib.util,io,json,pathlib,tempfile,unittest,zipfile
from unittest.mock import patch
ROOT=pathlib.Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('installer',ROOT/'scripts/beta-install.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
def archive(files,extra=None):
 data=io.BytesIO();manifest={'version':'0.24.0','release':'0.24.0-beta.1','files':{k:mod.sha(v) for k,v in files.items()}}
 with zipfile.ZipFile(data,'w')as z:
  z.writestr('manifest.json',json.dumps(manifest))
  for name,content in files.items():z.writestr(name,content)
  if extra:z.writestr(*extra)
 data.seek(0);return zipfile.ZipFile(data)
class InstallerTest(unittest.TestCase):
 def test_valid_manifest(self):
  with archive({'runtime/wallpaper-lab':b'fixture'})as z:self.assertEqual(mod.verify(z)['version'],'0.24.0')
 def test_tampered_payload_rejected(self):
  with archive({'runtime/wallpaper-lab':b'fixture'},('runtime/wallpaper-lab',b'tampered'))as z:
   with self.assertRaises(RuntimeError):mod.verify(z)
 def test_unknown_entry_rejected(self):
  with archive({'runtime/wallpaper-lab':b'fixture'},('private-data',b'no'))as z:
   with self.assertRaises(RuntimeError):mod.verify(z)
 def test_traversal_rejected(self):
  with archive({'../outside':b'no'})as z:
   with self.assertRaises(RuntimeError):mod.verify(z)
 def test_wrong_process_not_owned(self):
  self.assertFalse(mod.identity({'pid':1,'start':'wrong'},pathlib.Path('/bin/false')))
 def test_atomic_update(self):
  with tempfile.TemporaryDirectory()as d:
   file=pathlib.Path(d)/'launcher';mod.atomic(file,'first',0o755);mod.atomic(file,'second',0o755)
   self.assertEqual(file.read_text(),'second');self.assertEqual(file.stat().st_mode&0o777,0o755)
 def test_failed_upgrade_preserves_previous_renderer_and_launcher(self):
  from argparse import Namespace
  with tempfile.TemporaryDirectory()as d:
   home=pathlib.Path(d);data=home/'data';base=data/'wallpaper-lab';oldexe=base/'runtime/wallpaper-lab';oldexe.parent.mkdir(parents=True);oldexe.write_text('old executable')
   launcher=home/'.local/bin/wallpaper-lab';mod.atomic(launcher,'old launcher',0o755)
   state=home/'state/wallpaper-lab/running.json';state.parent.mkdir(parents=True);state.write_text('{"previous":"state"}');before=state.read_bytes()
   desktop=data/'applications/wallpaper-lab.desktop';mod.atomic(desktop,'original menu')
   old={'pid':__import__('os').getpid(),'start':'start','path':'fixture','alive':True}
   def fake_run(cmd,**kwargs):
    from subprocess import CompletedProcess,CalledProcessError
    if '--status'in cmd:return CompletedProcess(cmd,0,json.dumps(old),'')
    if '--restore'in cmd:
     state.write_text('{"partial":"new renderer state"}');raise CalledProcessError(1,cmd)
    return CompletedProcess(cmd,0,'','')
   files={'runtime/wallpaper-lab':b'new fixture','arch-dependencies.json':b'{}','wallpaper-lab.svg':b'<svg/>'}
   with archive(files)as z,patch.dict(mod.os.environ,{'XDG_DATA_HOME':str(data),'XDG_STATE_HOME':str(home/'state')}),patch.object(mod.P,'home',return_value=home),patch.object(mod,'prerequisite_check'),patch.object(mod,'validate_runtime'),patch.object(mod.subprocess,'run',side_effect=fake_run),patch.object(mod.os,'kill')as kill,patch.object(mod.os,'readlink',return_value=str(oldexe)),patch.object(mod,'identity',return_value=True):
    m=mod.verify(z)
    with self.assertRaises(mod.subprocess.CalledProcessError):mod.install(z,m,Namespace(uninstall=False,check=False,yes=True,skip_steam=True,no_launch=True))
    self.assertEqual(launcher.read_text(),'old launcher');self.assertEqual(state.read_bytes(),before);self.assertEqual(desktop.read_text(),'original menu');kill.assert_not_called()
if __name__=='__main__':unittest.main()

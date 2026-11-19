"""Exercise publication and rollback without changing the host display manager."""
from pathlib import Path
import contextlib,hashlib,importlib.util,io,json,os,subprocess,tempfile,unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
d=module('deploy',ROOT/'assets/login_deploy.py');s=module('session',ROOT/'assets/login_session.py')
class LoginDeployment(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(prefix='login-tests-');self.root=Path(self.temp.name)
    def tearDown(self):self.temp.cleanup()
    def test_copy_boundaries(self):
        source=self.root/'source';source.mkdir();(source/'file').write_text('safe');hashes={'file':hashlib.sha256(b'safe').hexdigest()}
        d.copy_manifest(source,self.root/'valid',hashes);self.assertEqual((self.root/'valid/file').read_text(),'safe')
        for mapping in [{'../escape':hashes['file']},{'/escape':hashes['file']},{'file':'0'*64},{'file/child':hashes['file']}]:
            with self.assertRaises((ValueError,OSError)):d.copy_manifest(source,self.root/'bad',mapping)
            self.assertFalse((self.root/'bad').exists())
        (source/'link').symlink_to(source/'file')
        with self.assertRaises(OSError):d.copy_manifest(source,self.root/'bad',{'link':hashes['file']})
        (source/'directory').symlink_to(source,target_is_directory=True)
        with self.assertRaises(OSError):d.copy_manifest(source,self.root/'bad',{'directory/file':hashes['file']})
        parent=self.root/'parent-link';parent.symlink_to(source,target_is_directory=True)
        with self.assertRaises(OSError):d.copy_manifest(parent,self.root/'bad',hashes)
        with self.assertRaises(FileExistsError):d.copy_manifest(source,self.root/'valid',hashes)
        self.assertEqual((self.root/'valid/file').read_text(),'safe')
        os.mkfifo(source/'pipe')
        with self.assertRaises(ValueError):d.copy_manifest(source,self.root/'bad',{'pipe':hashes['file']})
    def test_publication_and_rollback(self):
        runtime=self.root/'runtime';runtime.mkdir()
        for name in ['wallpaper-lab','liblinux-wallpaperengine-lib.so','login_session.py','login_deploy.py']:(runtime/name).write_text('fixture')
        project=self.root/'project';project.mkdir();(project/'project.json').write_text('{"type":"video","file":"image.ppm"}');(project/'image.ppm').write_text('fixture')
        profiles=self.root/'profiles.json';profiles.write_text(json.dumps({str(project):{'position':{'x':.4,'y':0,'zoom':1},'image':{'brightness':.2}}}))
        font=self.root/'font';font.write_text('font fixture')
        file=self.root/'login.json';file.write_text(json.dumps({'renderer':str(runtime/'wallpaper-lab'),'project':str(project),'source':str(project),'assets':str(project),'profile_file':str(profiles),'font':'Example','profiles':{str(project):{'x':.4}},'owner':os.getpid()}))
        base=self.root/'system';config=self.root/'greetd/config.toml';alias=self.root/'display-manager.service';alias.symlink_to('/usr/lib/systemd/system/sddm.service');config.parent.mkdir();config.write_text('old config')
        with patch.multiple(d,BASE=base,STATE=self.root/'state',CONFIG=config,ALIAS=alias),patch.object(d.subprocess,'check_output',return_value=str(font)),contextlib.redirect_stdout(io.StringIO()):
            prepared=d.prepare(file);package=Path(prepared['package']);manifest=json.loads((package/'manifest.json').read_text());bundle=package/'bundle';doc=json.loads((bundle/'login.json').read_text())
            self.assertNotIn('owner',doc);self.assertEqual(doc['source'],str(base/'releases'/manifest['release']/'project'));self.assertEqual(doc['profiles'][doc['source']],{'x':.4});self.assertTrue((bundle/'font.ttf').exists());self.assertFalse((bundle/'runtime/sddm-assets').exists())
            calls=[]
            def run(args,**kwargs):calls.append(args);return subprocess.CompletedProcess(args,0)
            with patch.object(d.os,'geteuid',return_value=0),patch.object(d.pwd,'getpwnam',return_value=type('User',(),{'pw_uid':os.getuid(),'pw_gid':os.getgid()})()),patch.object(d.shutil,'which',side_effect=lambda x:'/usr/bin/'+x),patch.object(d.subprocess,'run',side_effect=run):
                d.install(package);installed=json.loads((base/'installed.json').read_text());self.assertIn('greeter',config.read_text());self.assertTrue((Path(installed['bundle'])/'assets').is_dir());self.assertEqual(calls[0],['systemctl','enable','--force','greetd.service']);self.assertNotIn('--now',str(calls))
                alias.unlink();alias.symlink_to('/usr/lib/systemd/system/greetd.service')
                second=self.root/'second';second.mkdir();(second/'input.json').write_bytes(file.read_bytes());published=d.prepare(second/'input.json');d.install(Path(published['package']));updated=json.loads((base/'installed.json').read_text());self.assertEqual(updated['backup'],installed['backup']);self.assertNotEqual(updated['bundle'],installed['bundle'])
                d.restore(Path(updated['backup']));self.assertEqual(config.read_text(),'old config');self.assertEqual(os.readlink(alias),'/usr/lib/systemd/system/sddm.service');self.assertFalse((base/'installed.json').exists())
    def test_compositor_geometry(self):
        bundle=self.root/'bundle';bundle.mkdir();(bundle/'login.json').write_text(json.dumps({'monitors':[{'name':'DP-1','width':2560,'height':1440,'x':1920,'y':0,'scale':1,'transform':0}]}))
        text=s.compositor_config(bundle,'/usr/bin/example');self.assertIn('2560x1440',text);self.assertIn('1920x0',text);self.assertIn('enforce_permissions=true',text);self.assertIn('"screencopy", "deny"',text)
        (bundle/'login.json').write_text(json.dumps({'monitors':[{'name':'DP-1','width':-1,'height':1,'x':0,'y':0}]}))
        with self.assertRaises(ValueError):s.compositor_config(bundle,'example')
    def test_pid_guard(self):
        child=subprocess.Popen(['/usr/bin/sleep','20'])
        try:
            identity=s.process_identity(child.pid)
            with self.assertRaises(ValueError):s.stop_child(child,('wrong',identity[1]))
            self.assertIsNone(child.poll());s.stop_child(child,identity);self.assertIsNotNone(child.poll())
        finally:
            if child.poll()is None:child.terminate();child.wait()
if __name__=='__main__':unittest.main()

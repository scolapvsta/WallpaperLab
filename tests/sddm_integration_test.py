"""SDDM integration boundaries: media resolution, isolation, validation and failed replacement."""
import importlib.util
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('sddm_integration', ROOT/'assets/sddm_integration.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

class SddmIntegrationTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='wallpaper-lab-sddm-')
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.theme = self.root/'theme'
        (self.theme/'Backgrounds').mkdir(parents=True)
        (self.theme/'Themes').mkdir()
        (self.theme/'Themes/astronaut.conf').write_text('[General]\nBackground=Backgrounds/test.png\n')
        # A real decodable PNG, generated without external image dependencies.
        import struct,zlib
        self.media = self.theme/'Backgrounds/test.png'
        def chunk(kind,data):return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data))
        self.media.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',16,16,8,2,0,0,0))+chunk(b'IDAT',zlib.compress((b'\0'+b'\x70\x90\xb0'*16)*16))+chunk(b'IEND',b''))
        self.library = self.root/'library'
        self.project = self.library/'local'
        self.project.mkdir(parents=True)
        (self.project/'project.json').write_text(json.dumps(dict(title='Local test', type='web', lab_origin='local', lab_media_file='media.png')))
        (self.project/'media.png').write_bytes(self.media.read_bytes())
        scene = self.library/'scene'
        scene.mkdir()
        (scene/'project.json').write_text(json.dumps(dict(title='Scene test', type='scene', file='scene.pkg', preview='preview.png')))
        (scene/'preview.png').write_bytes(self.media.read_bytes())
        self.request = dict(config=str(self.root/'config'), state=str(self.root/'state'), roots=[str(self.library)], theme=str(self.theme), action='enable')
        # Unit tests use a fake greeter; real shader rendering is checked separately.
        assets=self.root/'assets';assets.mkdir()
        (assets/'LabBackground.qml').write_text((ROOT/'assets/sddm/LabBackground.qml').read_text())
        (assets/'greeter.frag.qsb').write_bytes(b'fake shader for process boundary tests')
        self.request['assets']=str(assets)
        self.fake = self.root/'greeter'
        self.fake.write_text('#!/usr/bin/python3\nimport time\nprint("WALLPAPER_LAB_GREETER_READY",flush=True)\ntime.sleep(10)\n')
        self.fake.chmod(0o755)
        self.addCleanup(patch.stopall)
        patch.object(m.shutil, 'which', return_value=str(self.fake)).start()
        original_sleep = m.time.sleep
        patch.object(m.time, 'sleep', side_effect=lambda _: original_sleep(.15)).start()

    def instance(self, action='enable', value=''):
        return m.Integration(dict(self.request, action=action, value=value))

    def test_catalog_does_not_promote_scene_preview_or_escape_project(self):
        rows = m.catalog([self.library], self.theme)
        self.assertEqual(len(rows), 3)
        self.assertFalse(next(r for r in rows if r.get('kind')=='scene')['supported'])
        (self.project/'project.json').write_text(json.dumps(dict(type='video', file='../../theme/Backgrounds/test.png')))
        rows = m.catalog([self.library], self.theme)
        self.assertEqual(len(rows), 2)

    def test_preparation_independent_preferences_and_failed_replacement(self):
        initial = self.instance().run()
        self.assertTrue(initial['ready'])
        before = self.instance('wallpaper', str(self.project.resolve())).run()
        self.assertTrue(before['ready'])
        changed = self.instance('preset', 'black_hole').run()
        self.assertEqual(changed['config']['wallpaper'], before['config']['wallpaper'])
        integration = self.instance('wallpaper', str(self.library/'scene'))
        original_config = integration.config_file.read_bytes()
        original_background = (integration.bundle/'Backgrounds/wallpaper.png').read_bytes()
        with self.assertRaisesRegex(ValueError, 'renderer'):
            integration.run()
        self.assertEqual(integration.config_file.read_bytes(), original_config)
        self.assertEqual((integration.bundle/'Backgrounds/wallpaper.png').read_bytes(), original_background)
        self.assertTrue(integration.ready())
        # Selected media is copied; the prepared greeter survives source removal.
        (self.project/'media.png').unlink()
        self.assertTrue(self.instance('status').run()['ready'])
        self.assertFalse(self.instance('disable').run()['ready'])

    def test_dependency_failure_does_not_publish_and_tamper_invalidates_health(self):
        integration = self.instance()
        integration.run()
        original = integration.config_file.read_bytes()
        self.fake.write_text('#!/usr/bin/python3\nprint("module is not installed",flush=True)\n')
        with self.assertRaisesRegex(ValueError, 'validation failed'):
            self.instance('preset', 'black_hole').run()
        self.assertEqual(integration.config_file.read_bytes(), original)
        self.assertFalse(integration.ready())

    def test_publication_failure_restores_prior_bundle_and_preferences(self):
        integration = self.instance()
        integration.run()
        original = integration.config_file.read_bytes()
        config = (integration.bundle/'theme.conf').read_bytes()
        save = m.atomic_json
        def fail_receipt(path, data):
            if path.name == 'verified.json':
                raise OSError('receipt write failed')
            return save(path, data)
        with patch.object(m, 'atomic_json', side_effect=fail_receipt):
            with self.assertRaisesRegex(OSError, 'receipt'):
                self.instance('preset', 'black_hole').run()
        self.assertEqual(integration.config_file.read_bytes(), original)
        self.assertEqual((integration.bundle/'theme.conf').read_bytes(), config)
        self.assertTrue(integration.ready())

    def test_missing_prerequisites_never_enable(self):
        with patch.object(m.shutil, 'which', return_value=None):
            integration = self.instance()
            with self.assertRaisesRegex(ValueError, 'Install'):
                integration.run()
            self.assertFalse(integration.config_file.exists())
            self.assertFalse(integration.ready())

    def test_image_profiles_are_separate_from_login_and_desktop(self):
        original=self.instance().run()['config']['wallpaper']['source']
        values=dict(brightness=.3,contrast=1.3,saturation=.4,rotation=35,flip_x=1,zoom=1.5,x=.2,crop=False)
        changed=self.instance('edit',json.dumps(dict(source=original,values=values))).run()
        general=m.ini(Path(changed['bundle'])/'theme.conf')['General']
        self.assertEqual(general['LabBrightness'],'0.3')
        self.assertEqual(general['LabFlipX'],'1')
        self.assertEqual(general['CropBackground'],'false')
        self.assertTrue((Path(changed['bundle'])/'greeter.frag.qsb').is_file())
        local=self.instance('wallpaper',str(self.project)).run()
        self.assertEqual(local['image']['brightness'],0)
        self.assertEqual(local['image']['zoom'],1)
        returned=self.instance('wallpaper',original).run()
        self.assertEqual(returned['image']['brightness'],.3)
        preset=self.instance('preset','black_hole').run()
        self.assertEqual(preset['config']['wallpaper'],returned['config']['wallpaper'])
        self.assertFalse((self.root/'config/wallpaper_profiles.json').exists())

    def test_invalid_edits_preserve_verified_selection(self):
        state=self.instance().run();source=state['config']['wallpaper']['source']
        original=(self.root/'config/sddm.json').read_bytes()
        for values in [{'brightness':float('nan')},{'zoom':0},{'brightness':True},{'crop':1},{'unknown':1}]:
            with self.assertRaises(ValueError):self.instance('edit',json.dumps(dict(source=source,values=values))).run()
            self.assertEqual((self.root/'config/sddm.json').read_bytes(),original)
        with self.assertRaisesRegex(ValueError,'changed'):
            self.instance('edit',json.dumps(dict(source='stale-selection',values={}))).run()
        self.assertTrue(self.instance('status').run()['ready'])

    def test_posters_and_favorites_do_not_change_greeter_selection(self):
        state=self.instance().run();integration=self.instance('status')
        config=integration.config_file.read_bytes();receipt=(integration.state/'verified.json').read_bytes()
        row=next(row for row in state['backgrounds'] if row['id']==str(self.library/'scene'))
        self.assertFalse(row['supported']);self.assertTrue(row['thumbnail'])
        integration.request.update(action='thumbnail',value=json.dumps(dict(kind=0,id=row['id'])))
        result=integration.thumbnail();self.assertTrue(Path(result['thumbnail']).is_file())
        with patch.object(m.subprocess,'run',side_effect=AssertionError('cached poster decoded again')):
            self.assertEqual(integration.thumbnail(),result)
        favorites=self.instance('favorite',row['id']).run()
        self.assertTrue(next(r for r in favorites['backgrounds'] if r['id']==row['id'])['favorite'])
        self.assertEqual(integration.config_file.read_bytes(),config)
        self.assertEqual((integration.state/'verified.json').read_bytes(),receipt)
        self.assertTrue(favorites['ready'])
        self.assertFalse(next(r for r in self.instance('favorite',row['id']).run()['backgrounds'] if r['id']==row['id'])['favorite'])

    def test_missing_shader_does_not_enable(self):
        Path(self.request['assets'],'greeter.frag.qsb').unlink()
        with self.assertRaisesRegex(ValueError,'Shader Tools'):self.instance().run()
        self.assertFalse((self.root/'config/sddm.json').exists())

    def test_preview_normal_close_and_greeter_signal_handler(self):
        # Exercise actual child exits, including SDDM's handled SIGTERM status.
        for output,code,accepted in [('ready',0,True),('WALLPAPER_LAB_GREETER_READY\nSignal received: SIGTERM',255,True),('WALLPAPER_LAB_GREETER_READY',255,False),('Signal received: SIGTERM',255,False)]:
            self.fake.write_text('#!/usr/bin/python3\nimport sys\nprint('+repr(output)+')\nsys.exit('+str(code)+')\n')
            integration=self.instance('preview')
            with patch.object(integration,'ready',return_value=True):
                if accepted:self.assertTrue(integration.run()['ok'])
                else:
                    with self.assertRaisesRegex(ValueError,'preview failed'):integration.run()

if __name__ == '__main__':
    unittest.main()

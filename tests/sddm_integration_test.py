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
        # A valid one-pixel image; no copyrighted wallpaper fixtures.
        import base64
        self.media = self.theme/'Backgrounds/test.png'
        self.media.write_bytes(base64.b64decode('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAusB9Y9Zl1sAAAAASUVORK5CYII='))
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

if __name__ == '__main__':
    unittest.main()

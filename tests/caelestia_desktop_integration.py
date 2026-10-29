#!/usr/bin/env python3
"""Real Caelestia CLI, isolated XDG palette publish/restore and own local renderer."""
import argparse, hashlib, json, os, pathlib, subprocess
from PIL import Image

p = argparse.ArgumentParser()
p.add_argument('--app', required=True, type=pathlib.Path)
p.add_argument('--output', required=True, type=pathlib.Path)
a = p.parse_args()
app, out = a.app.resolve(), a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
env = os.environ | {f'XDG_{kind}_HOME': str(out / kind.lower()) for kind in ['CONFIG', 'STATE', 'DATA', 'CACHE']}
env['LD_LIBRARY_PATH'] = ':'.join(str(app.parent / x) for x in ['.', 'lib', 'deps'])

def run(*args):
    return subprocess.check_output([str(x) for x in args], env=env, text=True)

personal_scheme = pathlib.Path(os.environ.get('XDG_STATE_HOME', pathlib.Path.home() / '.local/state')) / 'caelestia/scheme.json'
def fingerprint():
    return hashlib.sha256(personal_scheme.read_bytes()).hexdigest() if personal_scheme.exists() else None

before = fingerprint()
personal = subprocess.check_output([pathlib.Path.home() / '.local/bin/wallpaper-lab', '--status'], text=True)
blue, red = out / 'blue.png', out / 'red.png'
Image.new('RGB', (1280, 720), '#246dc0').save(blue)
Image.new('RGB', (1280, 720), '#df3455').save(red)
original = json.loads(run('caelestia', 'wallpaper', '--print', blue))
scheme = out / 'state/caelestia/scheme.json'
scheme.write_text(json.dumps(original))
config = out / 'config/wallpaper-lab/settings.json'
config.parent.mkdir(parents=True, exist_ok=True)
steam = out / 'steam'
steam.mkdir(exist_ok=True)
config.write_text(json.dumps({'mode': 'single', 'screen': 0, 'fps': 30, 'muted': True,
                              'transition_type': 0, 'workshop_path': str(steam), 'caelestia_enabled': True}))
path = json.loads(run(app, '--import', red))[0]
# Deliberately wrong creator thumbnail: the palette must use the rendered red media.
Image.new('RGB', (640, 360), '#246dc0').save(pathlib.Path(path) / 'preview.png')
expected = json.loads(run('python3', app.parent / 'caelestia_palette.py', red))['colours']['primary']
try:
    run(app, '--apply', path)
    generated = json.loads(scheme.read_text())
    assert generated['colours']['primary'] != original['colours']['primary']
    assert generated['colours']['primary'] == expected, 'thumbnail used instead of rendered wallpaper'
    assert len(generated['colours']) > 30
    assert json.loads((out / 'state/wallpaper-lab/integration-status.json').read_text())['ok']
    assert not (out / 'state/caelestia/wallpaper').exists(), 'Caelestia wallpaper command was used'
    run(app, '--ui-option', 'caelestia_enabled', 0)
    assert json.loads(scheme.read_text()) == original
    run(app, '--ui-option', 'caelestia_enabled', 1)
    assert json.loads(scheme.read_text())['colours']['primary'] == generated['colours']['primary']
    run(app, '--ui-option', 'caelestia_enabled', 0)
    assert json.loads(scheme.read_text()) == original
finally:
    run(app, '--stop')
assert fingerprint() == before, 'personal Caelestia scheme changed'
assert subprocess.check_output([pathlib.Path.home() / '.local/bin/wallpaper-lab', '--status'], text=True) == personal
results = {'real_caelestia_palette_generated': True, 'rendered_frame_preferred_over_wrong_thumbnail': True, 'own_renderer_changes_shell_scheme_only': True,
           'disable_restores_previous_scheme': True, 'personal_scheme_and_renderers_preserved': True}
(out / 'results.json').write_text(json.dumps(results, indent=2))
print(json.dumps(results, indent=2))

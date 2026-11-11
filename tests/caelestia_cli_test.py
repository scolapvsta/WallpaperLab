#!/usr/bin/env python3
"""Compare real CLI wallpaper generation and the app adapter in isolated XDGs."""
import importlib.util
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile

if not shutil.which('caelestia') or importlib.util.find_spec('caelestia') is None:
    print('Installed Caelestia CLI is required for palette parity checks')
    sys.exit(77)
from PIL import Image, ImageDraw

helper = Path(__file__).resolve().parents[1]/'assets/caelestia_cli.py'
with tempfile.TemporaryDirectory(prefix='wallpaper-lab-cli-palette-') as tmp:
    root = Path(tmp)
    env = os.environ | {f'XDG_{kind}_HOME': str(root/kind.lower())
                        for kind in ('CONFIG', 'STATE', 'CACHE', 'DATA')}
    config = root/'config/caelestia/cli.json'
    config.parent.mkdir(parents=True)
    theme_flags = ('Term', 'Hypr', 'Discord', 'Spicetify', 'Pandora', 'Fuzzel',
                   'Btop', 'Nvtop', 'Htop', 'Gtk', 'Qt', 'Warp', 'Chromium', 'Zed', 'Cava')
    config.write_text(json.dumps({
        'theme': dict({f'enable{x}': False for x in theme_flags},
                      postHook='echo theme-applied; printf x >> '+shlex.quote(str(root/'theme-hook'))),
        'wallpaper': {'postHook': 'printf x >> '+shlex.quote(str(root/'wallpaper-hook'))}}))
    original = config.read_bytes()
    scheme_file = root/'state/caelestia/scheme.json'
    scheme_file.parent.mkdir(parents=True)
    for n, (flavour, color) in enumerate((('default', '#913bbb'), ('hard', '#a1e1c3'))):
        image = root/f'image{n}.png'
        im = Image.new('RGB', (640, 360), color)
        ImageDraw.Draw(im).rectangle((30, 20, 280, 320), fill='#d5a354')
        im.save(image)
        baseline = {'name': 'dynamic', 'flavour': flavour, 'mode': 'dark',
                    'variant': 'fidelity', 'colours': {}}
        scheme_file.write_text(json.dumps(baseline))
        subprocess.run(['caelestia', 'wallpaper', '-f', str(image)], env=env,
                       check=True, capture_output=True, text=True, timeout=30)
        expected = json.loads(scheme_file.read_text())
        scheme_file.write_text(json.dumps(baseline))
        result = subprocess.run([sys.executable, str(helper), str(image)], env=env,
                                check=True, capture_output=True, text=True, timeout=30)
        actual = json.loads(result.stdout)
        assert actual == expected, (flavour, 'CLI and app adapter generated different schemes')
        assert json.loads(scheme_file.read_text()) == expected
        assert actual['flavour'] == flavour
        assert config.read_bytes() == original, 'CLI config changed'
        assert (root/'wallpaper-hook').read_text() == 'x'*(n+1), 'Wallpaper hook ran twice'
        assert (root/'theme-hook').read_text() == 'x'*(2*(n+1)), 'Theme hook was not preserved'
        assert (root/'state/caelestia/wallpaper/current').resolve() == image
        assert (root/'state/wallpaper-lab/integrations/caelestia-cli.log').exists()
print('Full CLI color parity, smart mode/variant, default/hard flavours, theme hooks, wallpaper-hook suppression and config preservation passed')

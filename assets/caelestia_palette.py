#!/usr/bin/env python3
"""Optional Material You palette generator. No shell, dotfiles or services installed."""
import argparse, collections, json, os, pathlib, subprocess, sys


def imports():
    from PIL import Image
    from materialyoucolor.hct import Hct
    from materialyoucolor.score.score import Score
    from materialyoucolor.dynamiccolor.material_dynamic_colors import MaterialDynamicColors
    return Image, Hct, Score, MaterialDynamicColors


def prepare():
    base = pathlib.Path(os.environ.get('XDG_DATA_HOME', pathlib.Path.home() / '.local/share')) / 'wallpaper-lab/integrations/caelestia'
    base.mkdir(parents=True, exist_ok=True)
    presets = {'vibrant': 'Vivace', 'tonalspot': 'Toni morbidi', 'expressive': 'Espressiva', 'fidelity': 'Fedele al wallpaper'}
    (base / 'presets.json').write_text(json.dumps(presets))
    try:
        imports()
        return sys.executable
    except ImportError:
        root = base / 'venv'
        python = root / 'bin/python'
        if not python.exists():
            subprocess.run([sys.executable, '-m', 'venv', str(root)], check=True)
        subprocess.run([str(python), '-m', 'pip', 'install', '--disable-pip-version-check', 'materialyoucolor==3.0.4', 'Pillow>=10,<13'], check=True, stdout=sys.stderr)
        subprocess.run([str(python), __file__, '--check'], check=True, stdout=subprocess.DEVNULL)
        return str(python)


def palette(file, variant, mode, tint):
    Image, Hct, Score, MaterialDynamicColors = imports()
    from materialyoucolor.scheme.scheme_vibrant import SchemeVibrant
    from materialyoucolor.scheme.scheme_tonal_spot import SchemeTonalSpot
    from materialyoucolor.scheme.scheme_expressive import SchemeExpressive
    from materialyoucolor.scheme.scheme_fidelity import SchemeFidelity
    with Image.open(file) as source:
        source = source.convert('RGB')
        source.thumbnail((96, 96))
        pixels = source.get_flattened_data() if hasattr(source, 'get_flattened_data') else source.getdata()
        colors = collections.Counter((255 << 24) | (r << 16) | (g << 8) | b for r, g, b in pixels)
    seed = Score.score(colors)[0]
    hct = Hct.from_int(seed)
    # Auto mode is driven by the wallpaper, not the app's UI theme.
    if mode == 'auto':
        mean = sum((.2126*((c>>16)&255)+.7152*((c>>8)&255)+.0722*(c&255))*n for c,n in colors.items()) / sum(colors.values())
        mode = 'light' if mean > 155 else 'dark'
    generator = {'vibrant': SchemeVibrant, 'tonalspot': SchemeTonalSpot, 'expressive': SchemeExpressive, 'fidelity': SchemeFidelity}[variant]
    scheme = generator(hct, mode == 'dark', 0.0)
    dynamic = MaterialDynamicColors()
    roles = dynamic.all_colors if hasattr(dynamic, 'all_colors') else [v for v in vars(MaterialDynamicColors).values() if hasattr(v, 'get_hct')]
    colours = {role.name: f'{role.get_hct(scheme).to_int() & 0xffffff:06x}' for role in roles}
    # Caelestia uses these neutral surfaces for its bars. A restrained chromatic
    # wash makes the wallpaper hue visible while retaining light/dark contrast.
    seed_rgb = [(seed >> shift) & 255 for shift in (16, 8, 0)]
    for name in list(colours):
        if name.startswith('surface') or name == 'background':
            rgb = [int(colours[name][x:x+2], 16) for x in (0, 2, 4)]
            colours[name] = ''.join(f'{round(v*(1-tint)+s*tint):02x}' for v,s in zip(rgb,seed_rgb))
    aliases = {'base':'surface','mantle':'surfaceContainerLow','crust':'surfaceContainerLowest','text':'onSurface','subtext1':'onSurfaceVariant','subtext0':'outline','surface0':'surfaceContainer','surface1':'surfaceContainerHigh','surface2':'surfaceContainerHighest','overlay0':'outlineVariant','overlay1':'outline','overlay2':'onSurfaceVariant'}
    for name, role in aliases.items():
        if role in colours: colours[name] = colours[role]
    return {'name':'dynamic','flavour':'default','mode':mode,'variant':variant,'colours':colours}


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--prepare', action='store_true')
    p.add_argument('--check', action='store_true')
    p.add_argument('--variant', choices=['vibrant','tonalspot','expressive','fidelity'], default='vibrant')
    p.add_argument('--mode', choices=['auto','dark','light'], default='auto')
    p.add_argument('--tint', type=float, default=.18)
    p.add_argument('image', nargs='?')
    a = p.parse_args()
    if a.prepare: print(json.dumps({'python':prepare()}))
    elif a.check: imports();print('ready')
    elif a.image: print(json.dumps(palette(a.image,a.variant,a.mode,max(0,min(.3,a.tint)))))
    else: p.error('image is required')

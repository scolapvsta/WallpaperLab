#!/usr/bin/env python3
"""Use the installed Caelestia wallpaper pipeline without its wallpaper hook."""
import argparse
import json
import os
import pathlib
import sys


def apply(image):
    from caelestia.utils import wallpaper
    from caelestia.utils.paths import c_state_dir
    from caelestia.utils.scheme import get_scheme

    # Keep theme settings and hooks intact. Wallpaper Lab already owns awww's
    # transition and runs the separately imported wallpaper post-hook afterward.
    config = wallpaper.get_config()
    config = dict(config, wallpaper=dict(config.get('wallpaper', {})))
    config['wallpaper'].pop('postHook', None)
    wallpaper.get_config = lambda: config
    log = c_state_dir.parent / 'wallpaper-lab/integrations/caelestia-cli.log'
    log.parent.mkdir(parents=True, exist_ok=True)
    # CLI theme hooks can print to OS stdout; keep the result channel JSON-only.
    with log.open('a') as output:
        stdout, stderr = os.dup(1), os.dup(2)
        try:
            sys.stdout.flush()
            sys.stderr.flush()
            os.dup2(output.fileno(), 1)
            os.dup2(output.fileno(), 2)
            wallpaper.set_wallpaper(pathlib.Path(image), False)
        finally:
            sys.stdout.flush()
            sys.stderr.flush()
            os.dup2(stdout, 1)
            os.dup2(stderr, 2)
            os.close(stdout)
            os.close(stderr)
    scheme = get_scheme()
    return {key: getattr(scheme, key) for key in ('name', 'flavour', 'mode', 'variant', 'colours')}


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('image')
    args = parser.parse_args()
    print(json.dumps(apply(args.image)))

"""Canonical release identity shared by the source and runtime packagers."""
import re


def release_version(manifest):
    version = manifest.get('version', '')
    if manifest.get('channel') != 'beta' or not re.fullmatch(r'0\.\d+\.\d+-beta\.[1-9]\d*', version):
        raise ValueError('Wallpaper Lab requires a complete pre-1.0 beta version (0.x.y-beta.N).')
    return version

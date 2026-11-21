"""Prevent different executable, package, helper and documented release identities."""
import json
from pathlib import Path
import re
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / 'scripts'))
from release_version import release_version

manifest = json.loads((root / 'build-manifest.json').read_text())
version = release_version(manifest)
helper = (root / 'scripts/download-install.sh').read_text()
assert re.search(r'^release_version=(.+)$', helper, re.M)[1] == version
assert '/releases/latest' not in helper
for name in ['README.md', 'docs/BETA.md', '.github/ISSUE_TEMPLATE/beta-bug.yml']:
    assert version in (root / name).read_text(), name
assert subprocess.check_output([sys.argv[1], '--version'], text=True).strip() == 'Wallpaper Lab ' + version
for invalid in [dict(version='1.0.0', channel='stable'), dict(version='0.25.0', channel='beta'),
                dict(version='0.25.0-beta.0', channel='beta'), dict(version='0.25.0-beta.1-beta.1', channel='beta')]:
    try:
        release_version(invalid)
    except ValueError:
        pass
    else:
        raise AssertionError('Invalid release identity accepted: ' + repr(invalid))
print('PASS: canonical beta version matches executable, helper and documentation')

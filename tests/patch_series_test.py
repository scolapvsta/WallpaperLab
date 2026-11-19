#!/usr/bin/env python3
"""Check overlapping patch prefixes and preservation of unrelated local edits."""
import difflib
from pathlib import Path
import subprocess
import sys
import tempfile

helper = Path(__file__).resolve().parents[1]/'scripts/apply-patch-series.py'
with tempfile.TemporaryDirectory(prefix='wallpaper patches ') as tmp:
    root = Path(tmp)
    source = root/'tree'
    source.mkdir()
    original = ''.join(f'line {n}\n' for n in range(30))
    one = original.replace('line 1\n', 'first change\n')
    two = one.replace('line 2\n', 'overlapping change\n')
    patches = []
    for n, (before, after) in enumerate(((original, one), (one, two))):
        patch = root/f'{n}.patch'
        patch.write_text(''.join(difflib.unified_diff(before.splitlines(True), after.splitlines(True), fromfile='a/file', tofile='b/file')))
        patches.append(str(patch))
    path = source/'file'
    for initial in (original, one, two):
        # The local edit lies beyond the overlapping hunks and must survive.
        path.write_text(initial.replace('line 29\n', 'local edit\n'))
        command = [sys.executable, str(helper), str(source), *patches]
        subprocess.run(command[:2]+['--check']+command[2:], check=True, capture_output=True)
        assert path.read_text() == initial.replace('line 29\n', 'local edit\n')
        subprocess.run(command, check=True, capture_output=True)
        assert path.read_text() == two.replace('line 29\n', 'local edit\n')
    path.write_text('conflicting source\n')
    result = subprocess.run(command, capture_output=True)
    assert result.returncode != 0 and path.read_text() == 'conflicting source\n'
print('Fresh, overlapping prefix, complete series and conflict preservation passed')

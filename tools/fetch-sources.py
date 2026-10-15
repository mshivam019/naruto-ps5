#!/usr/bin/env python3
"""Fetch pinned public source into ignored worktrees; preserve existing edits."""
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
PINS = json.loads((ROOT / 'sources.json').read_text())
for directory, url, revision in (
    ('naruto-rise-recomp', 'https://github.com/jorjbarbosa/naruto-rise-recomp', PINS['naruto']),
):
    path = ROOT / 'upstream' / directory
    if not path.exists():
        subprocess.run(['git', 'clone', url, str(path)], check=True)
        subprocess.run(['git', '-C', str(path), 'checkout', '--detach', revision], check=True)
    actual = subprocess.check_output(['git', '-C', str(path), 'rev-parse', 'HEAD'], text=True).strip()
    if actual != revision:
        raise SystemExit(f'{directory}: expected {revision}, found {actual}; existing checkout preserved')
subprocess.run(['git', '-C', str(ROOT / 'upstream/naruto-rise-recomp'),
                'submodule', 'update', '--init', '--recursive'], check=True)

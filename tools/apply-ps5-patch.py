#!/usr/bin/env python3
"""Apply the PS5 runtime patch after the six pinned Naruto SDK patches."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
SDK = ROOT / 'upstream/naruto-rise-recomp/sdk'
patch = ROOT / 'ps5/patches/rexglue-ps5.patch'
if not patch.is_file():
    raise SystemExit('PS5 runtime patch has not been exported yet')
reverse = subprocess.run(['git', '-C', str(SDK), 'apply', '--reverse', '--check', str(patch)],
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
if reverse.returncode == 0:
    print('PS5 runtime patch already applied')
else:
    subprocess.run(['git', '-C', str(SDK), 'apply', '--check', str(patch)], check=True)
    subprocess.run(['git', '-C', str(SDK), 'apply', str(patch)], check=True)
    print('Applied PS5 runtime patch')

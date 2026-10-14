#!/usr/bin/env python3
"""Stage a PRIVATE native test title; includes generated executable and disc data."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[2]
TITLE = 'PPSA99680'

def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, default=ROOT / 'dist' / TITLE)
args = parser.parse_args()
if args.output.name != TITLE or args.output.exists():
    parser.error('Choose a new staging folder ending in PPSA99680')
exe = ROOT / 'build/native-title/eboot.bin'
if not exe.is_file() or exe.stat().st_size < 1024:
    parser.error('A signed native eboot.bin must be built first')
files = {'eboot.bin': exe,
         'sce_module/libc.prx': ROOT.parent / 'PS5_Vulkan/runtime/libc.prx',
         'sce_sys/param.json': ROOT / 'ps5/sce_sys/param.json',
         'user/narutorise.toml': ROOT / 'ps5/config/narutorise.toml'}
for name in ('icon0.png', 'pic0.png', 'pic1.png'):
    source = ROOT / 'ps5/sce_sys' / name
    if source.is_file():
        files['sce_sys/' + name] = source
for source in (ROOT / 'private/game_root').rglob('*'):
    if source.is_file() and '$SystemUpdate' not in source.parts:
        files['game/' + source.relative_to(ROOT / 'private/game_root').as_posix()] = source
if not any(name.casefold() == 'game/ai2c.dll' for name in files) or 'game/default.xex' not in files:
    parser.error('Missing private base-game files')
expected = (ROOT.parent / 'PS5_Vulkan/runtime/libc.prx.sha256').read_text().split()[0]
if digest(files['sce_module/libc.prx']) != expected:
    parser.error('libc.prx does not match its recorded digest')
receipt = {'title_id': TITLE, 'private_game_content': True, 'publishable': False, 'files': []}
for name, source in sorted(files.items()):
    target = args.output / name
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, target)
    sha = digest(source)
    if digest(target) != sha:
        raise RuntimeError(f'Copy mismatch: {name}')
    receipt['files'].append({'path': name, 'bytes': target.stat().st_size, 'sha256': sha})
(ROOT / 'evidence/local-package.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(f'Private test title: {args.output}; {len(files)} verified files. Never upload this title to GitHub.')

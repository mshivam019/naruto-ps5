#!/usr/bin/env python3
"""Create an isolated base-game manifest without changing upstream DLC mappings."""
from pathlib import Path
import subprocess
import sys
import tomllib

root = Path(__file__).resolve().parent.parent
source = root / 'upstream/naruto-rise-recomp/app'
game = root / 'private/game_root'
target = root / 'private/codegen'
subprocess.run([sys.executable, str(root / 'tools/validate-game.py'), str(game),
                '--receipt', str(root / 'evidence/game-identity.json')], check=True)
target.mkdir(exist_ok=True)
manifest = (source / 'narutorise_manifest.toml').read_text().split('# Character DLCs')[0]
for name in ('default.xex', 'ai2c.dll'):
    actual = next(p for p in game.iterdir() if p.name.casefold() == name.casefold())
    manifest = manifest.replace(f'../game_root/{name}', actual.as_posix())
manifest = manifest.replace('game_root = "../game_root"', f'game_root = "{game.as_posix()}"')
parsed = tomllib.loads(manifest)
assert len(parsed['modules']) == 1 and parsed['modules'][0]['guest_path'] == 'ai2c.dll'
for name in ('analysis-seeds.toml', 'ai2c-seeds.toml'):
    (target / name).write_bytes((source / name).read_bytes())
(target / 'narutorise_manifest.toml').write_text(manifest)
print(target / 'narutorise_manifest.toml')

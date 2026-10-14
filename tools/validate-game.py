#!/usr/bin/env python3
"""Inspect private base-game XEX headers; title identity is not revision proof."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('game_root', type=Path)
parser.add_argument('--receipt', type=Path, required=True)
args = parser.parse_args()
result = {'revision_compatibility': 'unverified', 'files': {}}
for name in ('default.xex', 'ai2c.dll'):
    matches = [p for p in args.game_root.iterdir() if p.name.casefold() == name.casefold()]
    if len(matches) != 1:
        raise SystemExit(f"{name}: expected exactly one case-insensitive match")
    path = matches[0]
    data = path.read_bytes()
    if len(data) < 24 or data[:4] != b'XEX2':
        raise SystemExit(f'{name}: not a complete XEX2 header')
    count = struct.unpack_from('>I', data, 20)[0]
    if count > (len(data) - 24) // 8:
        raise SystemExit(f'{name}: invalid optional header count')
    entry = {'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
    for index in range(count):
        key, offset = struct.unpack_from('>II', data, 24 + index * 8)
        if key == 0x00040006:
            if offset + 24 > len(data):
                raise SystemExit(f'{name}: truncated execution ID')
            media, version, base, title = struct.unpack_from('>IIII', data, offset)
            entry.update(title_id=f'{title:08X}', media_id=f'{media:08X}',
                         version=f'{version:08X}', base_version=f'{base:08X}')
    if name == 'default.xex' and entry.get('title_id') != '555307E5':
        raise SystemExit(f'Unexpected title identity: {entry}')
    result['files'][name] = entry
args.receipt.write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result, indent=2))

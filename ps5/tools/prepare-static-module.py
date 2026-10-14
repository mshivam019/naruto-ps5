#!/usr/bin/env python3
"""Namespace guest helper functions that collide when two DLLs become static."""
import argparse
import json
from pathlib import Path
import re

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--generated', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
def functions(module):
    result = set()
    for source in (args.generated / module).glob('narutorise_recomp.*.cpp'):
        result.update(re.findall(r'DEFINE_REX_FUNC\((\w+)\)', source.read_text()))
    return result
shared = functions('default') & functions('ai2c')
if not shared:
    raise SystemExit('Expected shared guest helper names; inspect codegen changes')
names = {}
for name in shared:
    names[name] = 'naruto_ai2c_' + name
    names['__imp__' + name] = '__imp__naruto_ai2c_' + name
pattern = re.compile(r'\b(?:' + '|'.join(re.escape(n) for n in sorted(names, key=len, reverse=True)) + r')\b')
args.output.mkdir(parents=True, exist_ok=True)
for module in ('default', 'ai2c'):
    destination = args.output / module
    destination.mkdir(exist_ok=True)
    for path in (args.generated / module).iterdir():
        if path.suffix not in {'.h', '.cpp', '.cmake'}:
            continue
        text = path.read_text()
        if module == 'ai2c':
            text = pattern.sub(lambda match: names[match[0]], text)
        text = text.replace(
            '#if REX_PLATFORM_WIN32 || (REX_PLATFORM_MAC && REX_ARCH_ARM64)',
            '#if REX_PLATFORM_PS5 || REX_PLATFORM_WIN32 || (REX_PLATFORM_MAC && REX_ARCH_ARM64)')
        target = destination / path.name
        if not target.exists() or target.read_text() != text:
            target.write_text(text)
(args.output / 'static-name-map.json').write_text(json.dumps(names, indent=2) + '\n')
print(f'Namespaced {len(shared)} shared guest functions for static AI2C linking')

#!/usr/bin/env python3
"""Export SDK edits into ps5/patches/rexglue-ps5.patch: the difference between
the working SDK and the pinned SDK plus the six upstream patches. Checks that
the result applies to a fresh checkout before replacing the patch."""
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / 'upstream/naruto-rise-recomp'
SDK = SOURCE / 'sdk'
PATCH = ROOT / 'ps5/patches/rexglue-ps5.patch'
PIN = json.loads((ROOT / 'sources.json').read_text())['sdk']


def git(*args, cwd=SDK, **kwargs):
    return subprocess.run(['git', '-C', str(cwd), *map(str, args)], check=True, **kwargs)


def base_worktree(directory):
    git('worktree', 'add', '--detach', directory, PIN, capture_output=True)
    for patch in sorted((SOURCE / 'patches/sdk').glob('*.patch')):
        git('apply', '--whitespace=nowarn', patch, cwd=directory)


def main():
    changed = subprocess.check_output(['git', '-C', str(SDK), 'diff', '--name-only', PIN], text=True).split()
    changed += subprocess.check_output(['git', '-C', str(SDK), 'ls-files', '--others', '--exclude-standard'],
                                       text=True).split()
    with tempfile.TemporaryDirectory() as temporary:
        base = Path(temporary) / 'base'
        check = Path(temporary) / 'check'
        try:
            base_worktree(base)
            chunks = []
            for name in sorted(set(changed)):
                old, new = base / name, SDK / name
                if old.is_file() and new.is_file() and old.read_bytes() == new.read_bytes():
                    continue
                result = subprocess.run(['git', 'diff', '--no-index', '--no-color', '--binary',
                                         str(old) if old.exists() else '/dev/null',
                                         str(new) if new.exists() else '/dev/null'],
                                        capture_output=True, text=True)
                text = result.stdout.replace(f'a{base}/', 'a/').replace(f'b{SDK}/', 'b/')
                text = text.replace(f'a{SDK}/', 'a/').replace(f'b{base}/', 'b/')
                text = text.replace(str(base) + '/', '').replace(str(SDK) + '/', '')
                chunks.append(text)
            patch = ''.join(chunks)
            base_worktree(check)
            subprocess.run(['git', '-C', str(check), 'apply', '--check', '-'], input=patch, text=True, check=True)
        finally:
            for directory in (base, check):
                if directory.exists():
                    git('worktree', 'remove', '--force', directory, capture_output=True)
            git('worktree', 'prune')
    PATCH.write_text(patch)
    print(f'{PATCH}: {patch.count("diff --git ")} files')


if __name__ == '__main__':
    main()

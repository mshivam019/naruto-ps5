#!/usr/bin/env python3
"""Compile code generated from your own disc into the private game module
(naruto.module) that the PS5 app loads. Never publish its output."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import importlib.util
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
EXPORTS = ['naruto_module_abi', 'PPCImageConfig', 'NarutoAI2C_Register', 'NarutoAI2C_GetImageInfo']
AI2C_DEFINES = ['-DReXModule_Register=NarutoAI2C_Register',
                '-DReXModule_GetImageInfo=NarutoAI2C_GetImageInfo',
                '-DPPCImageConfig=NarutoAI2C_ImageConfig',
                '-DPPCFuncMappings=NarutoAI2C_FuncMappings']


def load_support():
    spec = importlib.util.spec_from_file_location('module_support', ROOT / 'ps5/tools/module-support.py')
    support = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(support)
    return support


def sha(data):
    return hashlib.sha256(data).hexdigest()


def run(command, log):
    with log.open('a', encoding='utf-8') as stream:
        subprocess.run([str(c) for c in command], check=True, stdout=stream, stderr=subprocess.STDOUT)


def symbol_names(nm, path, *flags):
    text = subprocess.check_output([str(nm), *flags, str(path)], text=True)
    return {line.split()[-1] for line in text.splitlines() if line.strip()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--generated', required=True, type=Path, help='rexglue codegen output (default/, ai2c/)')
    parser.add_argument('--toolchain', required=True, type=Path, help='LLVM bin directory (clang++, ld.lld, llvm-nm)')
    parser.add_argument('--sysroot', required=True, type=Path, help='PS5 payload SDK')
    parser.add_argument('--sdk', required=True, type=Path, help='Patched ReXGlue SDK source')
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--jobs', type=int, default=4)
    parser.add_argument('--write-imports', action='store_true',
                        help='Developer mode: record the module imports as the app contract')
    args = parser.parse_args()
    support = load_support()
    tools = {name: args.toolchain / name for name in ('clang++', 'ld.lld', 'llvm-nm')}
    for name, path in tools.items():
        if not path.is_file():
            parser.error(f'Missing {name} in {args.toolchain}')
    args.work.mkdir(parents=True, exist_ok=True)
    log = args.work / 'build.log'

    replacements = {'{SYSROOT}': str(args.sysroot.resolve()), '{SDK}': str(args.sdk.resolve())}
    flags = []
    for line in support.FLAGS.read_text().splitlines():
        if not line or line.startswith('#'):
            continue
        for key, value in replacements.items():
            line = line.replace(key, value)
        flags.append(line)

    generated = args.work / 'generated'
    run([sys.executable, ROOT / 'ps5/tools/prepare-static-module.py', '--generated', args.generated,
         '--output', generated], log)

    objects_dir = args.work / 'objects'
    objects_dir.mkdir(exist_ok=True)
    clang = tools['clang++']
    identity_base = sha('\n'.join(flags).encode() + clang.read_bytes()[:4096] +
                        (args.sdk / 'include/rex/ppc.h').read_bytes())

    jobs = []  # (source, object, extra flags, pch)
    for module in ('default', 'ai2c'):
        directory = generated / module
        extra = [f'-I{directory}'] + (AI2C_DEFINES if module == 'ai2c' else [])
        header = directory / 'narutorise_pch.h'
        pch = objects_dir / f'{module}.pch'
        pch_identity = sha((identity_base + str(extra)).encode() + header.read_bytes())
        stamp = pch.with_suffix('.sha256')
        if not (pch.is_file() and stamp.is_file() and stamp.read_text() == pch_identity):
            print(f'Preparing {module} headers...', flush=True)
            run([clang, *flags, *extra, '-x', 'c++-header', header, '-o', pch], log)
            stamp.write_text(pch_identity)
        for source in sorted(directory.glob('*.cpp')):
            jobs.append((source, objects_dir / f'{module}-{source.stem}.o', extra, pch))
    hooks = ROOT / 'ps5/app/loader_thread.cpp'
    jobs.append((hooks, objects_dir / 'hook-loader_thread.o', [f'-I{generated / "default"}'], None))
    abi_source = args.work / 'module_abi.cpp'

    def compile_one(job):
        source, obj, extra, pch = job
        identity = sha((identity_base + str(extra)).encode() + source.read_bytes() +
                       (pch.with_suffix('.sha256').read_bytes() if pch else b''))
        stamp = obj.with_suffix('.sha256')
        if obj.is_file() and stamp.is_file() and stamp.read_text() == identity:
            return obj
        command = [clang, *flags, *extra]
        if pch:
            command += ['-include-pch', pch]
        run([*command, '-c', source, '-o', obj], log)
        stamp.write_text(identity)
        return obj

    print(f'Compiling {len(jobs)} game code files with {args.jobs} jobs (this takes a while)...', flush=True)
    with ThreadPoolExecutor(max_workers=args.jobs) as executor:
        objects = list(executor.map(compile_one, jobs))

    version_script = args.work / 'exports.map'
    version_script.write_text('{ global: ' + ' '.join(n + ';' for n in EXPORTS) + ' local: *; };\n')

    def link():
        abi_source.write_text('extern "C" __attribute__((visibility("default"))) const char '
                              f'naruto_module_abi[] = "{support.abi()}";\n')
        abi_object = args.work / 'module_abi.o'
        run([clang, *flags, '-c', abi_source, '-o', abi_object], log)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        run([tools['ld.lld'], '-shared', '-z', 'max-page-size=16384', '-z', 'common-page-size=16384',
             '--hash-style=sysv', f'--version-script={version_script}', *objects, abi_object,
             '-o', args.output], log)

    link()
    undefined = symbol_names(tools['llvm-nm'], args.output, '-D', '--undefined-only')
    if args.write_imports:
        support.IMPORTS.write_text('# Runtime symbols the game module imports from the app.\n'
                                   '# Generated by ps5/tools/build-game-module.py --write-imports.\n' +
                                   ''.join(name + '\n' for name in sorted(undefined)))
        link()  # the ABI identity covers the import list
    else:
        missing = undefined - set(support.imports())
        if missing:
            raise ValueError('Game module needs runtime symbols this app does not provide: ' +
                             ', '.join(sorted(missing)[:20]))
    exported = symbol_names(tools['llvm-nm'], args.output, '-D', '--defined-only')
    if set(EXPORTS) - exported:
        raise ValueError('Missing game module exports: ' + ', '.join(sorted(set(EXPORTS) - exported)))
    print(f'Game module: {args.output} ({args.output.stat().st_size} bytes, {len(undefined)} runtime imports)')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(f'Module build failed: {error}', file=sys.stderr)
        sys.exit(1)

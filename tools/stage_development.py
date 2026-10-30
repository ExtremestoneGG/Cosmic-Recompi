"""Stage only the current local build, never fallback to downloaded release DLLs."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def stage(native, frontend, output):
    native, frontend, output = [Path(p).resolve() for p in (native, frontend, output)]
    if output in (native, frontend) or output.is_relative_to(native) or output.is_relative_to(frontend):
        raise ValueError('Output must be separate from build inputs')
    sources = {p.name: p for p in native.glob('*.dll')}
    sources['ben10.exe'] = native / 'ben10.exe'
    required = {'ben10.exe', 'rexruntime.dll', 'rexgpu-xenos.dll'}
    if not required <= sources.keys() or not all(p.is_file() for p in sources.values()):
        raise ValueError('Incomplete native build output; do not borrow release binaries')
    for name in ('CosmicRecomp.exe', 'CosmicBridge.exe'):
        if not (frontend / name).is_file():
            raise ValueError('Missing current frontend/bridge: ' + name)
    runtime = output / 'runtime'
    existing = {p.name for p in runtime.glob('*') if p.is_file()}
    if existing - sources.keys() - {'pc-menu-api.txt', 'SHA256SUMS.txt'}:
        raise ValueError('Untracked runtime files in output; select a new staging directory')
    runtime.mkdir(parents=True, exist_ok=True)
    for name, source in sources.items():
        shutil.copyfile(source, runtime / name)
    for name in ('CosmicRecomp.exe', 'CosmicBridge.exe'):
        shutil.copyfile(frontend / name, output / name)
    (runtime / 'pc-menu-api.txt').write_text('7\n', encoding='ascii')
    names = sorted([*sources, 'pc-menu-api.txt'])
    hashes = {name: hashlib.sha256((runtime / name).read_bytes()).hexdigest() for name in names}
    (runtime / 'SHA256SUMS.txt').write_text(''.join(f'{hashes[name]}  {name}\n' for name in names), encoding='ascii')
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    dirty = bool(subprocess.check_output(['git', 'status', '--porcelain'], cwd=ROOT, text=True).strip())
    metadata = dict(consumer_commit=revision, working_tree_dirty=dirty,
                    profile='hero-time', native_menu_api=7,
                    features=['All Aliens', 'Hero Time', 'native Video and Extras menus'],
                    runtime_sha256=hashes,
                    validation='Local staging only; not proof of gameplay or ABI correctness')
    (output / 'DEVELOPMENT_BUILD.json').write_text(json.dumps(metadata, indent=2) + '\n', encoding='utf-8')
    return metadata

if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    for flag in ('native', 'frontend', 'output'):
        parser.add_argument('--' + flag, type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(stage(args.native, args.frontend, args.output), indent=2))

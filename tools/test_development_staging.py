"""Reject incomplete or mixed development stages; synthetic files, no game data."""
from pathlib import Path
import tempfile
from stage_development import ROOT, stage

(ROOT / 'build').mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
    base = Path(temporary)
    native, frontend, output = [base / name for name in ('native', 'frontend', 'app')]
    native.mkdir(); frontend.mkdir()
    for name in ('ben10.exe', 'rexgpu-xenos.dll', 'rexruntime.dll'):
        (native / name).write_bytes(name.encode())
    for name in ('CosmicRecomp.exe', 'CosmicBridge.exe'):
        (frontend / name).write_bytes(name.encode())
    report = stage(native, frontend, output)
    assert report['profile'] == 'hero-time' and len(report['runtime_sha256']) == 4
    stage(native, frontend, output)
    (output / 'runtime/stale.dll').write_bytes(b'stale')
    try:
        stage(native, frontend, output)
        raise RuntimeError('Stale DLL accepted')
    except ValueError:
        pass
    (native / 'rexgpu-xenos.dll').unlink()
    try:
        stage(native, frontend, base / 'incomplete')
        raise RuntimeError('Missing renderer accepted')
    except ValueError:
        pass
print('Development manifest, repeated staging, stale DLL and missing renderer checks passed.')

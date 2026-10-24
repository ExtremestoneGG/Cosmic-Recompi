"""Wrap an explicitly staged, asset-free release as one portable Windows EXE.

Does not install, publish, collect an ISO, or copy the working game's user data.
The staging directory must already contain a verified matching runtime manifest.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import zipfile

parser=argparse.ArgumentParser()
parser.add_argument('--stage',type=Path,required=True)
parser.add_argument('--output',type=Path,required=True)
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
stage=args.stage.resolve();output=args.output.resolve()
assert stage.is_dir() and not output.exists(),'Use a complete stage and a new output filename'
assert output.suffix.lower()=='.exe'
manifest=stage/'runtime/SHA256SUMS.txt'
assert manifest.is_file(),'Runtime manifest required'
required={'ben10.exe','rexruntime.dll','rexgpu-xenos.dll','pc-menu-api.txt'}
runtime=set()
for line in manifest.read_text().splitlines():
    digest,name=line.split('  ',1)
    assert Path(name).name==name and ':' not in name and '/' not in name and '\\' not in name
    p=stage/'runtime'/name
    assert hashlib.sha256(p.read_bytes()).hexdigest()==digest,name
    runtime.add(name)
assert required<=runtime
assert (stage/'runtime/pc-menu-api.txt').read_text().strip()=='3','All Aliens/native-menu runtime required'
allowed_root={'CosmicRecomp.exe','CosmicBridge.exe','PROVENANCE.json','README.txt',
              'README.md','README.pt-BR.md','SHA256SUMS.txt','LICENSE','LICENSE.txt',
              'ARCHITECTURE.md','BUILD.md','CONTRIBUTING.md','RELEASE_NOTES.md','HOTFIX.md',
              'ROADMAP.md','SECURITY.md','SOURCES.md','TESTING.md','THIRD_PARTY.md',
              'TROUBLESHOOTING.md','USER_GUIDE.md'}
crt={'msvcp140.dll','vcruntime140.dll','vcruntime140_1.dll','concrt140.dll',
     'msvcp140_1.dll','msvcp140_2.dll','msvcp140_atomic_wait.dll',
     'msvcp140_codecvt_ids.dll','vccorlib140.dll','vcruntime140_threads.dll'}
files=[]
for p in sorted(stage.rglob('*')):
    if p.is_dir():continue
    relative=p.relative_to(stage)
    assert not p.is_symlink(),'No staged links'
    if len(relative.parts)==1:assert p.name in allowed_root|crt,relative
    elif relative.parts[0]=='runtime':assert len(relative.parts)==2 and (p.name in runtime or p.name=='SHA256SUMS.txt'),relative
    elif relative.parts[0]=='licenses':
        assert p.suffix.lower() in ('.txt','.md','') or p.name.upper().startswith(('LICENSE','LICENCE','COPYING','NOTICE','AUTHORS')),relative
    else:raise AssertionError('Unexpected package directory: '+str(relative))
    assert p.suffix.lower() not in ('.iso','.xex','.pk4','.ds','.pdb','.xbx','.dat')
    data=p.read_bytes()
    for needle in filter(None,(os.environ.get('USERPROFILE'),'.codex/attachments')):
        assert needle.lower().encode() not in data.lower(),relative
        assert needle.encode('utf-16-le').lower() not in data.lower(),relative
    files.append((relative,p))
assert {'CosmicRecomp.exe','CosmicBridge.exe','PROVENANCE.json'}<={str(p) for p,_ in files}
output.parent.mkdir(parents=True,exist_ok=True)
payload=output.with_suffix('.payload.zip')
assert not payload.exists()
with zipfile.ZipFile(payload,'x',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as archive:
    for relative,p in files:archive.write(p,relative.as_posix())
with zipfile.ZipFile(payload) as archive:assert archive.testzip() is None
os.environ.update(TEMP=str(root/'data/temp'),TMP=str(root/'data/temp'))
compiler=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
subprocess.run([str(compiler),'/nologo','/optimize+','/platform:x64','/target:winexe',
 '/win32manifest:'+str(root/'launcher/windows.manifest'),
 '/reference:System.Windows.Forms.dll','/reference:System.IO.Compression.dll',
 '/reference:System.IO.Compression.FileSystem.dll','/out:'+str(output),
 '/resource:'+str(payload)+',CosmicPayload.zip',str(root/'launcher/PortableBootstrap.cs')],check=True)
report=dict(executable=output.name,bytes=output.stat().st_size,sha256=hashlib.sha256(output.read_bytes()).hexdigest(),
 payload_sha256=hashlib.sha256(payload.read_bytes()).hexdigest(),files=len(files),original_game_assets=False,
 personal_saves=False,installed=False,published=False,gameplay_validation='Separate release gate required')
output.with_suffix('.validation.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))

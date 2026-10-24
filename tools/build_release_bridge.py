"""Build the public test bridge with Rath excluded. No game files are read."""
import os,subprocess
from pathlib import Path
from prepare_all_aliens_candidate import ROOT,authored,encode

out=ROOT/'build/rath-disabled-release';out.mkdir(parents=True,exist_ok=True)
program=out/'AllAliensProgram.bin';program.write_bytes(encode(authored(disable_rath=True)))
os.environ.update(TEMP=str(ROOT/'data/temp'),TMP=str(ROOT/'data/temp'))
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
sources=[str(ROOT/'launcher'/n) for n in ['NativeBridge.cs','InstalledGame.cs','IsoImage.cs','GameMenuPatch.cs','AllAliensPatch.cs']]
subprocess.run([str(csc),'/nologo','/optimize+','/target:winexe','/platform:x64',
 '/define:COSMIC_DISABLE_RATH','/win32manifest:'+str(ROOT/'launcher/windows.manifest'),
 '/reference:System.Windows.Forms.dll','/resource:'+str(program)+',CosmicAllAliens.bin',
 '/out:'+str(out/'CosmicBridge.exe'),*sources],check=True)
print('Public test bridge built: Rath locked and excluded from All Aliens.')

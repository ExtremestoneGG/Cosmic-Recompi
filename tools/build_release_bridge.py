"""Build the corrected settings/animation bridge, or the legacy exclusion profile."""
import argparse,os,subprocess
from pathlib import Path
from prepare_all_aliens_candidate import ROOT,authored,encode

parser=argparse.ArgumentParser();parser.add_argument('--rath-disabled',action='store_true');args=parser.parse_args()
out=ROOT/('build/rath-disabled-release' if args.rath_disabled else 'build/settings-release');out.mkdir(parents=True,exist_ok=True)
program=out/'AllAliensProgram.bin';program.write_bytes(encode(authored(disable_rath=args.rath_disabled)))
os.environ.update(TEMP=str(ROOT/'data/temp'),TMP=str(ROOT/'data/temp'))
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
sources=[str(ROOT/'launcher'/n) for n in ['NativeBridge.cs','InstalledGame.cs','IsoImage.cs','GameMenuPatch.cs','AllAliensPatch.cs','SaveReset.cs']]
subprocess.run([str(csc),'/nologo','/optimize+','/target:winexe','/platform:x64',
 '/define:'+('COSMIC_DISABLE_RATH' if args.rath_disabled else 'COSMIC_NATIVE_SETTINGS;COSMIC_ALIEN_STREAMS'),'/win32manifest:'+str(ROOT/'launcher/windows.manifest'),
 '/reference:System.Windows.Forms.dll','/resource:'+str(program)+',CosmicAllAliens.bin',
 '/out:'+str(out/'CosmicBridge.exe'),*sources],check=True)
print('Legacy exclusion bridge built.' if args.rath_disabled else 'Corrected animation/settings bridge built; matching API 7 runtime required.')

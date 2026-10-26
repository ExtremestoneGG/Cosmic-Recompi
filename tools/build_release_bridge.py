"""Build the corrected settings/animation bridge, or the legacy exclusion profile."""
import argparse,os,subprocess
from pathlib import Path
from prepare_all_aliens_candidate import ROOT,authored,encode

parser=argparse.ArgumentParser();parser.add_argument('--rath-disabled',action='store_true');parser.add_argument('--hero-time',action='store_true');args=parser.parse_args()
assert not (args.rath_disabled and args.hero_time)
out=ROOT/('build/rath-disabled-release' if args.rath_disabled else 'build/hero-time-release' if args.hero_time else 'build/settings-release');out.mkdir(parents=True,exist_ok=True)
program=out/'AllAliensProgram.bin';program.write_bytes(encode(authored(disable_rath=args.rath_disabled,preload_resources=args.hero_time)))
hero_arguments=[]
if args.hero_time:
    from prepare_hero_time_candidate import authored as hero_authored
    hero=out/'HeroTimeProgram.bin';hero.write_bytes(encode(hero_authored()))
    hero_arguments=['/resource:'+str(hero)+',CosmicHeroTime.bin']
os.environ.update(TEMP=str(ROOT/'data/temp'),TMP=str(ROOT/'data/temp'))
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
sources=[str(ROOT/'launcher'/n) for n in ['NativeBridge.cs','InstalledGame.cs','IsoImage.cs','GameMenuPatch.cs','AllAliensPatch.cs','SaveReset.cs']]
subprocess.run([str(csc),'/nologo','/optimize+','/target:winexe','/platform:x64',
 '/define:'+('COSMIC_DISABLE_RATH' if args.rath_disabled else 'COSMIC_NATIVE_SETTINGS;COSMIC_ALIEN_STREAMS'+(';COSMIC_HERO_TIME;COSMIC_PRELOAD_ALIENS' if args.hero_time else '')),'/win32manifest:'+str(ROOT/'launcher/windows.manifest'),
 '/reference:System.Windows.Forms.dll','/resource:'+str(program)+',CosmicAllAliens.bin',
 *hero_arguments,'/out:'+str(out/'CosmicBridge.exe'),*sources],check=True)
print('Legacy exclusion bridge built.' if args.rath_disabled else 'Corrected animation/settings bridge built; matching API 7 runtime required.')

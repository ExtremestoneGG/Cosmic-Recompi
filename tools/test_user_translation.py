"""Validate a user-supplied text patch locally; do not distribute its files."""
import argparse,os,subprocess,uuid
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('patch',type=Path);args=parser.parse_args()
assert args.patch.is_file()
out=ROOT/'build/diagnostics/user-translation'/uuid.uuid4().hex;out.mkdir(parents=True)
os.environ.update(TEMP=str(ROOT/'data/temp'),TMP=str(ROOT/'data/temp'))
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
exe=out/'test.exe'
subprocess.run([str(csc),'/nologo','/target:exe','/define:COSMIC_NATIVE_SETTINGS','/out:'+str(exe),
 str(ROOT/'launcher/GameMenuPatch.cs'),str(ROOT/'launcher/AllAliensPatch.cs'),str(ROOT/'tests/UserTranslationTests.cs')],check=True)
subprocess.run([str(exe),str(ROOT/'originals/global/scripts'),str(args.patch),str(out/'fixture')],check=True)
print('Private fixture:',out)

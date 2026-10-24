"""Compile and execute shipping importer/path/menu unit suites on private fixtures."""
import json,os,subprocess,uuid
from pathlib import Path
root=Path(__file__).resolve().parents[1]
output=root/'build/diagnostics/shipping-bridge-final';output.mkdir(parents=True,exist_ok=True)
os.environ.update(TEMP=str(root/'data/temp'),TMP=str(root/'data/temp'))
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
reports={}
fixtures=output/('fixtures-'+uuid.uuid4().hex)
for suite,sources,args in [
 ('IsoReaderTests',['IsoImage.cs'],[str(fixtures/'iso')]),
 ('InstallerPathTests',['InstalledGame.cs','IsoImage.cs'],[str(fixtures/'paths')]),
 ('GameMenuPatchTests',['GameMenuPatch.cs','AllAliensPatch.cs'],[str(root/'originals/global/scripts'),str(fixtures/'menus')]),
]:
    executable=output/(suite+'.exe')
    subprocess.run([str(csc),'/nologo','/target:exe','/platform:x64','/out:'+str(executable),
        *[str(root/'launcher'/name) for name in sources],str(root/'tests'/(suite+'.cs'))],check=True)
    result=subprocess.run([str(executable),*args],check=True,capture_output=True,text=True)
    reports[suite]=result.stdout.strip()
(root/'data/analysis/shipping-bridge-final-tests.json').write_text(json.dumps(reports,indent=2)+'\n',encoding='utf-8')
print(json.dumps(reports,indent=2))

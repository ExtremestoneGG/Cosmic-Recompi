"""Verify the public exclusion policy using authored bytecode and C# transforms."""
import json,os,subprocess,uuid
from pathlib import Path
from prepare_all_aliens_candidate import ROOT,authored,crc,decode,records
from test_all_aliens_script import VM

out=ROOT/'build/diagnostics/rath-disabled-release'/uuid.uuid4().hex;out.mkdir(parents=True)
cases=[]
for rath in (1,8,11):
 preset=[n for n in range(1,12) if n!=rath][:4]
 vm=VM(preset);vm.functions={h:(a,c) for h,a,c in authored(True)}
 vm.globals[0xe1003dc4]=lambda :{crc('Rath'):rath}
 vm.page=0;vm.run('CosmicAlienUpdate');assert len(vm.pending)==12
 vm.complete();seen=set(preset)
 for page in (1,2):
  vm.page=page;vm.run('CosmicAlienUpdate');seen.update(x for x in vm.slots() if x is not None)
 assert rath not in seen and len(seen)==10 and vm.slots()[-2:]==[None,None]
 vm.run('CosmicAlienUnload');assert vm.slots()==preset
 cases.append({'rath_id':rath,'selectable_forms':10})
os.environ.update(TEMP=str(ROOT/'data/temp'),TMP=str(ROOT/'data/temp'))
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
exe=out/'PolicyTests.exe'
subprocess.run([str(csc),'/nologo','/target:exe','/define:COSMIC_DISABLE_RATH',
 '/resource:'+str(ROOT/'build/rath-disabled-release/AllAliensProgram.bin')+',CosmicAllAliens.bin',
 '/out:'+str(exe),str(ROOT/'launcher/GameMenuPatch.cs'),str(ROOT/'launcher/AllAliensPatch.cs'),
 str(ROOT/'tests/AllAliensPatchTests.cs')],check=True)
subprocess.run([str(exe),str(ROOT/'originals/global/scripts'),str(out/'game')],check=True)
scripts=out/'game/global/scripts'
for h,a,c in records((scripts/'gld_xbx.ds').read_bytes()):
 if h==0x66960347:
  words,tokens=decode(c)
  assert sum(1 for p,k in tokens if k==0x17 and words[p+1]==crc('Rath'))==4
for h,a,c in records((scripts/'frontend.ds').read_bytes()):
 if h==crc('MainMenuText_Enter'):
  words,tokens=decode(c);assert words[8]==0x00000402,'Release must not force-unlock Rath'
report={'passed':True,'roster_cases':cases,'all_four_native_selection_paths_guarded':True,
 'main_menu_rath_locked':True,'real_gameplay_validated':False}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n')
print('Rath exclusion policy verified:',out)

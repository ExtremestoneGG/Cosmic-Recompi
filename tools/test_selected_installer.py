import hashlib,json,os,subprocess,uuid
from pathlib import Path
r=Path(__file__).resolve().parents[1];out=r/'build/diagnostics/selected-installer'/uuid.uuid4().hex;out.mkdir(parents=True)
payload=r/'build/releases/CosmicRecomp-v0.4.0-alpha.2-installer.1-win64.payload.zip'
os.environ.update(TEMP=str(out),TMP=str(out))
driver=out/'Driver.cs';driver.write_text('static class Driver { static int Main(string[] args) { return PortableBootstrap.RunAt(args[0],false,false); } }')
exe=out/'Test.exe';csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
subprocess.run([str(csc),'/nologo','/target:exe','/main:Driver','/out:'+str(exe),'/reference:System.Windows.Forms.dll','/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll','/resource:'+str(payload)+',CosmicPayload.zip',str(r/'launcher/PortableBootstrap.cs'),str(driver)],check=True)
home=out/'Selected folder — vídeo';save=home/'game-data/user/synthetic-save.dat';save.parent.mkdir(parents=True);save.write_bytes(b'synthetic save fixture')
for i in range(2):subprocess.run([str(exe),str(home)],cwd=out,check=True)
assert not (out/'CosmicRecomp').exists()
assert save.read_bytes()==b'synthetic save fixture'
app=home/'app'/hashlib.sha256(payload.read_bytes()).hexdigest()
assert Path((app/'installation-path.txt').read_text(encoding='utf8'))==home/'game-data'
for line in (app/'runtime/SHA256SUMS.txt').read_text().splitlines():
 digest,name=line.split('  ',1);assert hashlib.sha256((app/'runtime'/name).read_bytes()).hexdigest()==digest
assert not (home/'game-data/game').exists()
report=dict(passed=True,selected_unicode_path=True,no_sibling_game_folder=True,repeated_install_preserves_save=True,runtime_hashes=True,game_launched=False,dialog_interaction_tested=False)
(r/'build/releases/installer-improvement-tests.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))

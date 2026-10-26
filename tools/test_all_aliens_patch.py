"""Compile the shipping transformer, verify its private outputs independently."""
import argparse,hashlib,json,os,subprocess,uuid
from pathlib import Path
from prepare_all_aliens_candidate import ROOT,records,decode,transform_globals,crc,encode,authored

parser=argparse.ArgumentParser();parser.add_argument('--preload-resources',action='store_true');parser.add_argument('--native-settings',action='store_true');args=parser.parse_args()
out=ROOT/'build/diagnostics/all-aliens-install-20261023'/uuid.uuid4().hex
out.mkdir(parents=True)
os.environ.update(TEMP=str(ROOT/'data/temp'),TMP=str(ROOT/'data/temp'))
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
exe=out/'test.exe'
program=ROOT/'launcher/AllAliensProgram.bin'
if args.preload_resources:
    program=out/'private-program.bin';program.write_bytes(encode(authored(preload_resources=True)))
subprocess.run([str(csc),'/nologo','/target:exe','/out:'+str(exe),
 *(['/define:COSMIC_PRELOAD_ALIENS'] if args.preload_resources else []),
 *(['/define:COSMIC_NATIVE_SETTINGS'] if args.native_settings else []),
 '/resource:'+str(program)+',CosmicAllAliens.bin',
 str(ROOT/'launcher/GameMenuPatch.cs'),str(ROOT/'launcher/AllAliensPatch.cs'),
 str(ROOT/'tests/AllAliensPatchTests.cs')],check=True)
subprocess.run([str(exe),str(ROOT/'originals/global/scripts'),str(out/'game')],check=True)
scripts=out/'game/global/scripts'
assert program.read_bytes()==encode(authored())
assert (scripts/'globals.ds').read_bytes()==transform_globals((ROOT/'originals/global/scripts/globals.ds').read_bytes())
report={}
for filename in ['frontend.ds','globals.ds','gld_xbx.ds','circdisp.ds']:
    blob=(scripts/filename).read_bytes();rows=records(blob)
    for h,a,c in rows:
        words,tokens=decode(c);boundaries={p for p,k in tokens}|{len(words)}
        for p,k in tokens:
            if k in (0x1d,0x1e,0x1f):assert words[p]>>(8 if k==0x1d else 16) in boundaries,(filename,hex(h),p)
    report[filename]={'sha256':hashlib.sha256(blob).hexdigest(),'functions':len(rows),'all_branch_targets_valid':True}
for filename,expected_changed in [('gld_xbx.ds',{0x66960347}),('circdisp.ds',{0x8f436ad3})]:
    old={h:(a,c) for h,a,c in records((ROOT/'originals/global/scripts'/filename).read_bytes())}
    new={h:(a,c) for h,a,c in records((scripts/filename).read_bytes())}
    assert {h for h in old if old[h]!=new[h]}==expected_changed
    if filename=='circdisp.ds':
        # Both injected lookups clobber temporary register 31. The original
        # GetIcon result must be copied to register 24 before either guard.
        words,tokens=decode(new[0x8f436ad3][1])
        sites=[p for p,k in tokens if k==7 and words[p+1]==crc('gFormHotKey')
               and words[p+2:p+5]==[0x1f1d2d18,0x00201f17,crc('formID')]]
        assert len(sites)==2
        assert all(words[p-1]==0x00181f06 for p in sites),'Guard clobbers GetIcon result'
        empty=new[crc('CosmicEmptyAlienIcon')][1]
        assert list(__import__('struct').unpack('>8I',empty[-32:]))==[
            0x00301806,0x00003103,0,0x00002d07,0x41871bc9,0x02302d20,31,0x21], 'Empty slot must be hidden, not display Ben'
report['real_gameplay_validated']=False
if args.native_settings:
    old={h:(a,c) for h,a,c in records((ROOT/'originals/global/scripts/frontend.ds').read_bytes())}
    new={h:(a,c) for h,a,c in records((scripts/'frontend.ds').read_bytes())}
    assert old[crc('MainMenuText_Enter')]==new[crc('MainMenuText_Enter')], 'Normal Rath progression must remain game-owned'
    for name in ('CosmicOverlayOptions','cosmic_overlay_on_row','cosmic_overlay_off_row'):assert crc(name) in new
    # The authored guard bypasses the original Gameplay A interceptor only
    # for the new fourth row; B and the three existing checkbox rows delegate.
    guard,_=decode(new[0x3b77a27e][1]);assert guard[:11]==[
        0x001e0017,0x74a09b28,0x00001f03,0x40800000,0x201f1e10,
        0x000b201f,0x00220117,0xe585980f,0x000b221f,0x00002102,0x00002122]
    report['native_settings']={'original_rath_progression_unchanged':True,'overlay_native_submenu_present':True}
(out/'independent-verification.json').write_text(json.dumps(report,indent=2))
print('Independent C#/Python transform and original branch checks passed:',out)

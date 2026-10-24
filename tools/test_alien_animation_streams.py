"""Independent C#/Python checks using local, user-owned animation records."""
import hashlib,json,os,struct,subprocess,uuid
from pathlib import Path
from merge_alien_streams import entries,merge_streams,channel_records
ROOT=Path(__file__).resolve().parents[1]
out=ROOT/'build/diagnostics/alien-animation-streams'/uuid.uuid4().hex;out.mkdir(parents=True)
os.environ.update(TEMP=str(ROOT/'data/temp'),TMP=str(ROOT/'data/temp'))
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
exe=out/'test.exe'
subprocess.run([str(csc),'/nologo','/target:exe','/out:'+str(exe),
 str(ROOT/'launcher/AllAliensPatch.cs'),str(ROOT/'tests/AlienAnimationStreamTests.cs')],check=True)
forms=('ad','af','bc','ec','hm','nr','ra','sf','sm','ts','wh')
paths=[ROOT/'originals/global'/('g_'+form+'.xbx') for form in forms]
archives=[(p.name,p.read_bytes()) for p in paths];proof=[]
for source in sorted((ROOT/'originals').glob('*/a_*_s.xbx')):
    original=source.read_bytes();expected,report=merge_streams(original,archives)
    destination=out/source.name
    subprocess.run([str(exe),str(source),str(destination),*[str(p) for p in paths]],check=True)
    actual=destination.read_bytes();assert actual==expected
    decoded=entries(actual);old=entries(original,True)
    assert all(decoded[k]==v for k,v in old.items())
    # Verify the game's modulo-bucket lookup, not just sequential index parsing.
    buckets,n,bat,rat,stride=struct.unpack_from('>5I',actual,32)
    for key in decoded:
        bucket=key%buckets;first=struct.unpack_from('>I',actual,32+bat+bucket*4)[0]
        end=struct.unpack_from('>I',actual,32+bat+(bucket+1)*4)[0] if bucket+1<buckets else n
        assert any(struct.unpack_from('>I',actual,32+rat+i*stride)[0]==key for i in range(first,end))
    assert all(k in decoded for k in (0x82e8f18b,0xa955f0d1,0x475f44cc))
    report.update(file=source.name,sha256=hashlib.sha256(actual).hexdigest(),csharp_python_identical=True,
                  native_hash_lookup_verified=True)
    proof.append(report)
(out/'proof.json').write_text(json.dumps(proof,indent=2))
fixture=out/'install/game';fixture.mkdir(parents=True)
for source in [*paths,*sorted((ROOT/'originals').glob('*/a_*_s.xbx'))]:
    target=fixture/source.relative_to(ROOT/'originals');target.parent.mkdir(parents=True,exist_ok=True)
    os.link(source,target)
subprocess.run([str(exe),'--ensure',str(fixture)],check=True)
before={p.relative_to(fixture).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in fixture.rglob('*') if p.is_file()}
subprocess.run([str(exe),'--ensure',str(fixture)],check=True)
after={p.relative_to(fixture).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in fixture.rglob('*') if p.is_file()}
assert before==after
for source in (ROOT/'originals').glob('*/a_*_s.xbx'):
    backup=fixture/source.relative_to(ROOT/'originals')
    assert Path(str(backup)+'.cosmic-animation-original').read_bytes()==source.read_bytes()
(out/'install-proof.json').write_text(json.dumps(dict(atomic_hardlink_replacement=True,idempotent=True,
    original_backups_identical=True,original_assets_modified=False,gameplay_verified=False),indent=2))
print('Eight native animation banks: C#/Python parity, old records and native hash lookups passed.',out)

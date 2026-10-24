"""Exercise reset/archive behavior only on a disposable synthetic installation."""
import os,subprocess,uuid
from pathlib import Path
root=Path(__file__).resolve().parents[1]
out=root/'build/diagnostics/save-reset'/uuid.uuid4().hex;out.mkdir(parents=True)
os.environ.update(TEMP=str(out),TMP=str(out))
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
exe=out/'SaveResetTests.exe'
subprocess.run([str(csc),'/nologo','/target:exe','/out:'+str(exe),
    str(root/'launcher/SaveReset.cs'),str(root/'tests/SaveResetTests.cs')],check=True)
subprocess.run([str(exe),str(out/'fixture')],check=True)

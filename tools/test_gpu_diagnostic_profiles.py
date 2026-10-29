"""Exercise the experimental C# profile selector without starting a game."""
import json, os, subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
out=root/'build/diagnostics/gpu-profiles';out.mkdir(parents=True,exist_ok=True)
os.environ.update(TEMP=str(out),TMP=str(out))
driver=out/'Driver.cs'
driver.write_text(r'''
using System; using System.IO; using System.Diagnostics; using CosmicRecompi;
class Driver {
 static int Main(string[] args) {
  string root=args[0], log=Path.Combine(root,"test.log"), file=Path.Combine(root,"graphics-diagnostic-profile.txt");
  string[] profiles={"standard","gpu-bounds","viewport","stencil"};
  string[] keys={"BEN10_EDRAM_BOUNDS","BEN10_INTEL_VIEWPORT","BEN10_INTEL_STENCIL"};
  for(int i=0;i<4;i++) {
   File.WriteAllText(file,profiles[i]); var start=new ProcessStartInfo();
   foreach(string key in keys)start.EnvironmentVariables.Remove(key);
   InstalledGame.ApplyGraphicsDiagnosticProfile(root,start,log);
   for(int k=0;k<3;k++)if(start.EnvironmentVariables[keys[k]]!=(i==k+1?"false":null))return 10+i;
  }
  File.WriteAllText(file,"unknown");
  try{InstalledGame.ApplyGraphicsDiagnosticProfile(root,new ProcessStartInfo(),log);return 20;}catch(IOException){}
  File.Delete(file); var defaults=new ProcessStartInfo();
  foreach(string key in keys)defaults.EnvironmentVariables.Remove(key);
  InstalledGame.ApplyGraphicsDiagnosticProfile(root,defaults,log);
  foreach(string key in keys)if(defaults.EnvironmentVariables[key]!=null)return 21;
  if(String.IsNullOrEmpty(defaults.EnvironmentVariables["BEN10_D3D12_PROFILE"]))return 24;
  foreach(string profile in new string[]{"measure-standard","measure-gpu-bounds"}) {
   File.WriteAllText(file,profile);var measured=new ProcessStartInfo();
   measured.EnvironmentVariables.Remove("BEN10_EDRAM_BOUNDS");
   InstalledGame.ApplyGraphicsDiagnosticProfile(root,measured,log);
   if(measured.EnvironmentVariables["BEN10_EDRAM_BOUNDS"]!=(profile=="measure-gpu-bounds"?"false":null))return 22;
   string output=measured.EnvironmentVariables["BEN10_D3D12_PROFILE"];
   if(String.IsNullOrEmpty(output) || Path.GetDirectoryName(output)!=root || Path.GetExtension(output)!=".jsonl")return 23;
  }
  Console.WriteLine("Profile isolation, experimental default and invalid-profile rejection passed."); return 0;
 }
}
''')
csc=Path(os.environ['WINDIR'])/'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
exe=out/'check.exe'
sources=[root/'launcher'/name for name in ('NativeBridge.cs','InstalledGame.cs','IsoImage.cs','GameMenuPatch.cs','AllAliensPatch.cs','SaveReset.cs')]
subprocess.run([str(csc),'/nologo','/target:exe','/main:Driver','/define:COSMIC_GPU_DIAGNOSTICS;COSMIC_NATIVE_SETTINGS;COSMIC_ALIEN_STREAMS;COSMIC_HERO_TIME;COSMIC_PRELOAD_ALIENS','/reference:System.Windows.Forms.dll','/out:'+str(exe),str(driver),*map(str,sources)],check=True)
subprocess.run([str(exe),str(out)],check=True)
(out/'report.json').write_text(json.dumps(dict(passed=True,profile_isolation=True,missing_file_experimental_default=True,invalid_rejected=True,game_started=False,rtx4060_tested=False),indent=2)+'\n')

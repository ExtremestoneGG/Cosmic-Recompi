using System;
using System.IO;
using System.Linq;
using CosmicRecompi;
static class AllAliensPatchTests {
    static void Check(bool value,string message){if(!value)throw new Exception(message);}
    static void Main(string[] args) {
        string source=args[0],game=args[1],scripts=Path.Combine(game,"global","scripts");
        Directory.CreateDirectory(scripts);
        string[] files={"frontend.ds","xbx_us.txt","globals.ds","gld_xbx.ds","circdisp.ds"};
        foreach(string file in files)File.Copy(Path.Combine(source,file),Path.Combine(scripts,file),true);
        string save=Path.Combine(game,"..","user","save-proof.txt");Directory.CreateDirectory(Path.GetDirectoryName(save));File.WriteAllText(save,"preserve");
        GameMenuPatch.Ensure(game);AllAliensPatch.Ensure(game);
        var expected=files.ToDictionary(f=>f,f=>File.ReadAllBytes(Path.Combine(scripts,f)));
        GameMenuPatch.Ensure(game);AllAliensPatch.Ensure(game);
        foreach(string file in files) {
            Check(expected[file].SequenceEqual(File.ReadAllBytes(Path.Combine(scripts,file))),"Repeated install changed "+file);
            Check(File.ReadAllBytes(Path.Combine(source,file)).SequenceEqual(File.ReadAllBytes(Path.Combine(scripts,file+".cosmic-original"))),"Backup changed "+file);
        }
        byte[] corrupt=(byte[])expected["globals.ds"].Clone();corrupt[corrupt.Length-1]^=1;
        File.WriteAllBytes(Path.Combine(scripts,"globals.ds"),corrupt);
        bool rejected=false;try{AllAliensPatch.Ensure(game);}catch(IOException){rejected=true;}
        Check(rejected,"Unknown extension accepted");
        Check(File.ReadAllBytes(Path.Combine(scripts,"globals.ds")).SequenceEqual(corrupt),"Unknown file overwritten");
        File.WriteAllBytes(Path.Combine(scripts,"globals.ds"),expected["globals.ds"]);
        Check(File.ReadAllText(save)=="preserve","Save changed");
        Console.WriteLine("All Aliens install, backups, idempotency, unknown-edit rejection and save preservation passed.");
    }
}

using System;
using System.IO;
using CosmicRecompi;
class UserTranslationTests {
    static void Main(string[] args) {
        string game=Path.Combine(args[2],"game"),scripts=Path.Combine(game,"global","scripts");Directory.CreateDirectory(scripts);
        foreach(string name in new[]{"frontend.ds","xbx_us.txt"})File.Copy(Path.Combine(args[0],name),Path.Combine(scripts,name));
        string save=Path.Combine(args[2],"user/save.dat");Directory.CreateDirectory(Path.GetDirectoryName(save));File.WriteAllText(save,"preserved");
        byte[] patch=File.ReadAllBytes(args[1]);string stored=Path.Combine(args[2],"patches/translation/xbx_us.txt");
        GameMenuPatch.InstallTranslation(game,stored,patch);GameMenuPatch.Ensure(game,File.ReadAllBytes(stored));
        byte[] merged=File.ReadAllBytes(Path.Combine(scripts,"xbx_us.txt"));
        if(merged.Length<=patch.Length)throw new Exception("Native menu strings missing.");
        for(int i=0;i<patch.Length;i++)if(merged[i]!=patch[i])throw new Exception("Translation content changed.");
        if(File.ReadAllText(save)!="preserved")throw new Exception("Save changed.");
        byte[] bad=(byte[])patch.Clone();bad[bad.Length-1]=1;bool rejected=false;
        try{GameMenuPatch.InstallTranslation(game,stored,bad);}catch(InvalidDataException){rejected=true;}
        if(!rejected||Convert.ToBase64String(File.ReadAllBytes(stored))!=Convert.ToBase64String(patch))throw new Exception("Invalid patch was committed.");
        Console.WriteLine("User PT-BR text patch: install, native menu strings, repeated launch, save preservation and invalid-patch rejection passed.");
    }
}

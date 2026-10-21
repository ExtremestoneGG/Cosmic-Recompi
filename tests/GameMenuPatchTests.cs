using System;
using System.IO;
using CosmicRecompi;

static class GameMenuPatchTests {
    static int Main(string[] args) {
        if(args.Length!=2)throw new ArgumentException("Usage: tests <own-game/scripts> <output-directory>");
        byte[] original=File.ReadAllBytes(Path.Combine(args[0],"frontend.ds"));
        byte[] english=File.ReadAllBytes(Path.Combine(args[0],"xbx_us.txt"));
        byte[] before=(byte[])original.Clone();
        byte[] patched=GameMenuPatch.Frontend(original);
        if(!System.Linq.Enumerable.SequenceEqual(original,before))throw new Exception("Input was mutated.");
        byte[] bad=(byte[])original.Clone();bad[4]^=1;
        bool rejected=false;
        try{GameMenuPatch.Frontend(bad);}catch(InvalidDataException){rejected=true;}
        if(!rejected)throw new Exception("Changed revision was accepted.");
        rejected=false;
        try{GameMenuPatch.Frontend(patched);}catch(InvalidDataException){rejected=true;}
        if(!rejected)throw new Exception("Double patch was accepted.");
        byte[] text=GameMenuPatch.English(english);
        for(int i=0;i<english.Length;i++)if(english[i]!=text[i])throw new Exception("Original localization changed.");
        Directory.CreateDirectory(args[1]);
        File.WriteAllBytes(Path.Combine(args[1],"frontend.ds"),patched);
        File.WriteAllBytes(Path.Combine(args[1],"xbx_us.txt"),text);
        string game=Path.Combine(args[1],"installation","game"),scripts=Path.Combine(game,"global","scripts");
        Directory.CreateDirectory(scripts);
        File.WriteAllBytes(Path.Combine(scripts,"frontend.ds"),original);
        File.WriteAllBytes(Path.Combine(scripts,"xbx_us.txt"),english);
        string save=Path.Combine(args[1],"installation","user","save.dat");
        Directory.CreateDirectory(Path.GetDirectoryName(save));File.WriteAllText(save,"preserve-test-save");
        GameMenuPatch.Ensure(game);GameMenuPatch.Ensure(game);
        if(!System.Linq.Enumerable.SequenceEqual(File.ReadAllBytes(Path.Combine(scripts,"frontend.ds")),patched))throw new Exception("Installed patch differs.");
        if(!System.Linq.Enumerable.SequenceEqual(File.ReadAllBytes(Path.Combine(scripts,"frontend.ds.cosmic-original")),original))throw new Exception("Original backup differs.");
        if(File.ReadAllText(save)!="preserve-test-save")throw new Exception("Save changed.");
        File.WriteAllText(Path.Combine(scripts,"frontend.ds"),"modified-other-version");
        rejected=false;
        try{GameMenuPatch.Ensure(game);}catch(IOException){rejected=true;}
        if(!rejected || File.ReadAllText(Path.Combine(scripts,"frontend.ds"))!="modified-other-version")throw new Exception("Modified file overwritten.");
        Console.WriteLine("8 menu patch checks passed; compare outputs with the independent reader.");
        return 0;
    }
}

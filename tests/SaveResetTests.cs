using System;
using System.IO;
using CosmicRecompi;
static class SaveResetTests {
    static int Main(string[] args) {
        string root=args[0];Directory.CreateDirectory(root);
        string user=Path.Combine(root,"user");Directory.CreateDirectory(user);
        File.WriteAllText(Path.Combine(user,"pc-video.ini"),"resolution=1080p");
        Directory.CreateDirectory(Path.Combine(user,"achievements"));
        File.WriteAllText(Path.Combine(user,"achievements","keep.txt"),"keep");
        foreach(string profile in new string[]{"0000000000000001","0000000000000002"}) {
            foreach(string title in new string[]{"445007F7","12345678"}) {
                string folder=Path.Combine(user,profile,title);Directory.CreateDirectory(folder);
                File.WriteAllText(Path.Combine(folder,"save.txt"),title);
            }
        }
        using(var busy=new FileStream(Path.Combine(root,"running.lock"),FileMode.OpenOrCreate,FileAccess.ReadWrite,FileShare.None)) {
            bool rejected=false;try{SaveReset.ArchiveProgress(root);}catch(IOException){rejected=true;}
            if(!rejected)throw new Exception("Active game save reset accepted");
        }
        string backup=SaveReset.ArchiveProgress(root);
        if(String.IsNullOrEmpty(backup))throw new Exception("Backup missing");
        foreach(string profile in new string[]{"0000000000000001","0000000000000002"}) {
            if(Directory.Exists(Path.Combine(user,profile,"445007F7")))throw new Exception("Progress was not reset");
            if(File.ReadAllText(Path.Combine(backup,profile,"445007F7","save.txt"))!="445007F7")throw new Exception("Backup changed");
            if(File.ReadAllText(Path.Combine(user,profile,"12345678","save.txt"))!="12345678")throw new Exception("Other title modified");
        }
        if(!File.Exists(Path.Combine(user,"pc-video.ini")) || !File.Exists(Path.Combine(user,"achievements","keep.txt")))throw new Exception("Unrelated settings changed");
        if(SaveReset.ArchiveProgress(root)!=null)throw new Exception("Empty reset was not idempotent");
        Console.WriteLine("Save reset: running-game rejection, two profiles, recoverable backup and unrelated-data preservation passed.");return 0;
    }
}

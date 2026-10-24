using System;
using System.IO;
using System.Collections.Generic;

namespace CosmicRecompi {
    // Called only after in-game confirmation and full game-process shutdown.
    // Keep native video settings, achievements and unrelated title data intact.
    internal static class SaveReset {
        static void NoLinks(string path) {
            for(string p=Path.GetFullPath(path);!String.IsNullOrEmpty(p);p=Path.GetDirectoryName(p))
                if((Directory.Exists(p)||File.Exists(p)) && (File.GetAttributes(p)&FileAttributes.ReparsePoint)!=0)
                    throw new IOException("Save folders cannot traverse links or junctions.");
        }
        internal static string ArchiveProgress(string installation) {
            string root=Path.GetFullPath(installation);NoLinks(root);
            using(var runLock=new FileStream(Path.Combine(root,"running.lock"),FileMode.OpenOrCreate,FileAccess.ReadWrite,FileShare.None)) {
                string user=Path.Combine(root,"user");NoLinks(user);
                string backup=Path.Combine(root,"save-backups",DateTime.UtcNow.ToString("yyyyMMdd-HHmmss")+"-"+Guid.NewGuid().ToString("N"));
                var moves=new List<string[]>();
                if(!Directory.Exists(user))return null;
                // Profiles are represented by 16-digit hexadecimal XUID folders.
                foreach(string profile in Directory.GetDirectories(user)) {
                    string name=Path.GetFileName(profile);ulong id;
                    if(name.Length!=16 || !UInt64.TryParse(name,System.Globalization.NumberStyles.HexNumber,System.Globalization.CultureInfo.InvariantCulture,out id))continue;
                    string title=Path.Combine(profile,"445007F7");
                    if(!Directory.Exists(title))continue;
                    NoLinks(title);
                    var pending=new Stack<string>();pending.Push(title);
                    while(pending.Count!=0) {
                        string folder=pending.Pop();NoLinks(folder);
                        foreach(string child in Directory.GetFileSystemEntries(folder)) {
                            NoLinks(child);if(Directory.Exists(child))pending.Push(child);
                        }
                    }
                    string target=Path.Combine(backup,name,"445007F7");NoLinks(target);
                    moves.Add(new string[]{title,target});
                }
                int completed=0;
                try {
                    foreach(string[] move in moves) {
                        Directory.CreateDirectory(Path.GetDirectoryName(move[1]));
                        Directory.Move(move[0],move[1]);completed++;
                    }
                } catch {
                    for(int i=completed-1;i>=0;i--)Directory.Move(moves[i][1],moves[i][0]);
                    throw;
                }
                return completed==0?null:backup;
            }
        }
    }
}

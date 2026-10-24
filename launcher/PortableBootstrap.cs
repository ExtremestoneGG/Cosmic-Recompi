using System;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Security.Cryptography;
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;
using System.Threading;

// Asset-free installer. The package contains only the host binaries
// and notices; original game data is supplied later through the ISO importer.
internal static class PortableBootstrap {
    static void NoLinks(string path) {
        for(string p=Path.GetFullPath(path);!String.IsNullOrEmpty(p);p=Path.GetDirectoryName(p))
            if((Directory.Exists(p) || File.Exists(p)) && (File.GetAttributes(p)&FileAttributes.ReparsePoint)!=0)
                throw new IOException("Package folders cannot traverse links or junctions.");
    }
    [STAThread] static int Main() {
        Application.EnableVisualStyles();
        using(var dialog=new FolderBrowserDialog()) {
            dialog.Description="Choose where to install Cosmic Recomp. Game files, saves and logs will be stored in this folder. Select your existing CosmicRecomp folder to update.";
            dialog.ShowNewFolderButton=true;
            dialog.SelectedPath=AppDomain.CurrentDomain.BaseDirectory;
            if(dialog.ShowDialog()!=DialogResult.OK)return 0;
            return RunAt(dialog.SelectedPath,true,true);
        }
    }
    // Noninteractive extraction entry point retained for package verification.
    internal static int Run(bool showErrors,bool launch) {
        return RunAt(Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"CosmicRecomp"),showErrors,launch);
    }
    internal static int RunAt(string selectedFolder,bool showErrors,bool launch) {
        string home=Path.GetFullPath(selectedFolder);
        string logs=Path.Combine(home,"logs");
        Mutex setup=null;bool acquired=false;
        try {
            NoLinks(home);Directory.CreateDirectory(logs);
            string temp=Path.Combine(home,"temp");Directory.CreateDirectory(temp);
            Environment.SetEnvironmentVariable("TEMP",temp);Environment.SetEnvironmentVariable("TMP",temp);
            byte[] payload;
            using(Stream resource=Assembly.GetExecutingAssembly().GetManifestResourceStream("CosmicPayload.zip")) {
                if(resource==null)throw new IOException("The embedded package is missing.");
                using(var buffer=new MemoryStream()){resource.CopyTo(buffer);payload=buffer.ToArray();}
            }
            string hash;using(var sha=SHA256.Create())hash=BitConverter.ToString(sha.ComputeHash(payload)).Replace("-","").ToLowerInvariant();
            setup=new Mutex(false,"Local\\CosmicRecompSetup-"+hash);
            try {acquired=setup.WaitOne(0);}catch(AbandonedMutexException){acquired=true;}
            if(!acquired)throw new IOException("This package is already being prepared. Wait for the other launcher.");
            string app=Path.Combine(home,"app",hash),marker=Path.Combine(app,"package-ready.txt");
            NoLinks(app);Directory.CreateDirectory(app);
            using(var bytes=new MemoryStream(payload))using(var archive=new ZipArchive(bytes,ZipArchiveMode.Read)) {
                string prefix=Path.GetFullPath(app)+Path.DirectorySeparatorChar;
                foreach(var entry in archive.Entries) {
                    string name=entry.FullName.Replace('/',Path.DirectorySeparatorChar);
                    if(name.IndexOf(':')>=0 || Path.IsPathRooted(name))throw new IOException("Invalid package entry.");
                    string target=Path.GetFullPath(Path.Combine(app,name));
                    if(!target.StartsWith(prefix,StringComparison.OrdinalIgnoreCase))throw new IOException("Entry outside package folder.");
                    NoLinks(target);
                    if(String.IsNullOrEmpty(entry.Name)){Directory.CreateDirectory(target);continue;}
                    Directory.CreateDirectory(Path.GetDirectoryName(target));
                    if(File.Exists(marker)) {
                        // Recheck installed bytes before every launch. Existing
                        // versions are immutable and never silently overwritten.
                        using(var expected=entry.Open())using(var actual=File.OpenRead(target))using(var sha=SHA256.Create()) {
                            string a=Convert.ToBase64String(sha.ComputeHash(expected));
                            string b=Convert.ToBase64String(sha.ComputeHash(actual));
                            if(a!=b)throw new IOException("Installed package was modified: "+entry.FullName);
                        }
                    } else {
                        using(var source=entry.Open())using(var output=new FileStream(target,FileMode.Create,FileAccess.Write,FileShare.None))source.CopyTo(output);
                    }
                }
            }
            string data=Path.Combine(home,"game-data");NoLinks(data);Directory.CreateDirectory(data);
            File.WriteAllText(Path.Combine(app,"installation-path.txt"),data,new UTF8Encoding(false));
            File.WriteAllText(marker,hash,new UTF8Encoding(false));
            File.AppendAllText(Path.Combine(logs,"setup.log"),DateTime.UtcNow.ToString("o")+" Package verified: "+hash+Environment.NewLine);
            if(launch)Process.Start(new ProcessStartInfo(Path.Combine(app,"CosmicRecomp.exe"),"--launcher") {UseShellExecute=false,WorkingDirectory=app});
            return 0;
        } catch(Exception error) {
            try{NoLinks(logs);Directory.CreateDirectory(logs);File.AppendAllText(Path.Combine(logs,"setup.log"),error+Environment.NewLine);}catch{}
            if(showErrors)MessageBox.Show(error.Message+Environment.NewLine+"Diagnostics: "+Path.Combine(logs,"setup.log"),"Cosmic Recomp",MessageBoxButtons.OK,MessageBoxIcon.Error);
            return 1;
        } finally {
            if(acquired)setup.ReleaseMutex();if(setup!=null)setup.Dispose();
        }
    }
}

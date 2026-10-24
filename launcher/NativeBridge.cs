using System;
using System.IO;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace CosmicRecompi {
    // Narrow process boundary: the native UI never parses or extracts an ISO.
    internal static class NativeBridge {
        static readonly string Root = AppDomain.CurrentDomain.BaseDirectory;
        static void Diagnostic(string value) {
            File.AppendAllText(InstalledGame.LauncherLog(Root),DateTime.UtcNow.ToString("o")+" "+value+Environment.NewLine,new UTF8Encoding(false));
        }
        internal static void CreateDesktopShortcut() {
            string desktop=Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
            if(String.IsNullOrEmpty(desktop))throw new IOException("Windows did not provide a desktop folder.");
            object shell=null,shortcut=null;
            try {
                Type shellType=Type.GetTypeFromProgID("WScript.Shell",true);
                shell=Activator.CreateInstance(shellType);
                shortcut=shellType.InvokeMember("CreateShortcut",System.Reflection.BindingFlags.InvokeMethod,null,shell,new object[]{Path.Combine(desktop,"Cosmic Recomp.lnk")});
                Type type=shortcut.GetType();
                type.InvokeMember("TargetPath",System.Reflection.BindingFlags.SetProperty,null,shortcut,new object[]{Path.Combine(Root,"CosmicRecomp.exe")});
                type.InvokeMember("Arguments",System.Reflection.BindingFlags.SetProperty,null,shortcut,new object[]{"--launcher"});
                type.InvokeMember("WorkingDirectory",System.Reflection.BindingFlags.SetProperty,null,shortcut,new object[]{Root});
                type.InvokeMember("Description",System.Reflection.BindingFlags.SetProperty,null,shortcut,new object[]{"Cosmic Recomp"});
                type.InvokeMember("Save",System.Reflection.BindingFlags.InvokeMethod,null,shortcut,null);
            } finally {
                if(shortcut!=null)System.Runtime.InteropServices.Marshal.FinalReleaseComObject(shortcut);
                if(shell!=null)System.Runtime.InteropServices.Marshal.FinalReleaseComObject(shell);
            }
        }
        static void Status(string value) {
            string path = Path.Combine(Root, "launcher-status.txt");
            File.WriteAllText(path + ".new", value, new UTF8Encoding(false));
            if (File.Exists(path)) File.Replace(path + ".new", path, null);
            else File.Move(path + ".new", path);
        }
        internal static void WaitForParent(string[] args) {
            if(args.Length==1)return;
            int pid;
            if(args.Length!=3 || args[0]!="play" || args[1]!="--parent" || !Int32.TryParse(args[2],out pid) || pid<=0)
                throw new ArgumentException("Invalid launcher handoff.");
            System.Diagnostics.Process parent;
            try {parent=System.Diagnostics.Process.GetProcessById(pid);}
            catch(ArgumentException){return;} // Frontend already exited.
            using(parent) {
                if(parent.HasExited)return;
                string expected=Path.GetFullPath(Path.Combine(Root,"CosmicRecomp.exe"));
                string actual;
                try{actual=parent.MainModule.FileName;}catch(InvalidOperationException){return;}
                if(!String.Equals(expected,actual,StringComparison.OrdinalIgnoreCase))throw new IOException("Unexpected launcher parent process.");
                if(!parent.WaitForExit(30000))throw new IOException("The launcher did not finish closing. The game was not started.");
            }
        }
        internal static void Play(Action<System.Diagnostics.ProcessStartInfo> configureProcess = null, Action<System.Diagnostics.Process> onLauncherOpened = null) {
                        Status("Verifying game files…");
                        string menuApi=Path.Combine(Root,"runtime","pc-menu-api.txt");
                        if(!File.Exists(menuApi) || File.ReadAllText(menuApi).Trim()!="3")
                            throw new IOException("This launcher requires the matching Cosmic Recomp game runtime. Extract the complete new release into a separate folder.");
                        string returnEventName = "Local\\CosmicRecompReturn-" + Guid.NewGuid().ToString("N");
                        bool returnRequested;
                        using (var returnEvent = new EventWaitHandle(false, EventResetMode.ManualReset, returnEventName)) {
                        using (var game = InstalledGame.Start(Root, false, returnEventName, delegate {
                            string installation=InstalledGame.Installation(Root);
                            string patch=Path.Combine(installation,"patches","translation","xbx_us.txt");
                            GameMenuPatch.Ensure(Path.Combine(installation,"game"),File.Exists(patch)?File.ReadAllBytes(patch):null);
                            AllAliensPatch.Ensure(Path.Combine(installation,"game"));
                        }, configureProcess)) {
                            Status("Game is running."); game.WaitForExit();
                            if (game.ExitCode != 0 && !returnEvent.WaitOne(0)) throw new IOException("The game exited with an error. Check the installation logs.");
                        }
                        returnRequested = returnEvent.WaitOne(0);
                        }
                        Status("Ready to play.");
                        if (returnRequested) {
                            var next = new System.Diagnostics.ProcessStartInfo(Path.Combine(Root, "CosmicRecomp.exe"), "--launcher");
                            next.WorkingDirectory = Root; next.UseShellExecute = false;
                            using (var launcher = System.Diagnostics.Process.Start(next)) { if (onLauncherOpened != null) onLauncherOpened(launcher); }
                        }
        }
        [STAThread] static int Main(string[] args) {
            try {
                string temp = Path.Combine(Root, "temp"); Directory.CreateDirectory(temp);
                Environment.SetEnvironmentVariable("TEMP", temp);
                Environment.SetEnvironmentVariable("TMP", temp);
                Diagnostic("Bridge started. OS="+Environment.OSVersion+"; 64-bit OS="+Environment.Is64BitOperatingSystem+"; action="+(args.Length==0?"missing":args[0]));
                if (args.Length == 0) throw new ArgumentException("Missing action.");
                WaitForParent(args);
                Application.EnableVisualStyles();
                switch (args[0]) {
                    case "play": Play(); break;
                    case "folder":
                        using (var dialog = new FolderBrowserDialog { Description = "Cosmic Recomp installation folder", SelectedPath = InstalledGame.Installation(Root) })
                            if (dialog.ShowDialog() == DialogResult.OK) InstalledGame.ChooseInstallation(Root, dialog.SelectedPath);
                        Status("Saves: " + Path.Combine(InstalledGame.Installation(Root), "user")); break;
                    case "saves":
                        string saves = Path.Combine(InstalledGame.Installation(Root), "user");
                        if (!Directory.Exists(saves)) throw new IOException("Import your ISO first.");
                        System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(saves) { UseShellExecute = true });
                        Status("Saves: " + saves); break;
                    case "patch":
                        if(!InstalledGame.Ready(Root))throw new IOException("Prepare your own ISO before applying a patch.");
                        using(var dialog=new OpenFileDialog {Filter="Translation table (xbx_us.txt)|xbx_us.txt",CheckFileExists=true,RestoreDirectory=true}) {
                            if(dialog.ShowDialog()!=DialogResult.OK){Status("Patch selection cancelled.");break;}
                            if(new FileInfo(dialog.FileName).Length>16*1024*1024)throw new IOException("The translation table is too large.");
                            string installation=InstalledGame.Installation(Root);
                            using(var runLock=new FileStream(Path.Combine(installation,"running.lock"),FileMode.OpenOrCreate,FileAccess.ReadWrite,FileShare.None)) {
                                InstalledGame.VerifyImportedExecutable(Root);
                                GameMenuPatch.InstallTranslation(Path.Combine(installation,"game"),Path.Combine(installation,"patches","translation","xbx_us.txt"),File.ReadAllBytes(dialog.FileName));
                            }
                            Status("Translation patch applied. Native PC menu entries preserved.");
                        }
                        break;
                    case "select-iso":
                        using (var dialog = new OpenFileDialog { Filter = "Xbox 360 ISO (*.iso)|*.iso", CheckFileExists = true, RestoreDirectory = true }) {
                            if (dialog.ShowDialog() != DialogResult.OK) { Status("ISO selection cancelled."); break; }
                            Status("Verifying ISO structure and supported edition…");
                            using (var image = new IsoImage(dialog.FileName)) image.VerifySupportedGame();
                            File.WriteAllText(Path.Combine(Root, "selected-iso.txt"), dialog.FileName, new UTF8Encoding(false));
                            Status("ISO accepted. Select Prepare game to continue.");
                        }
                        break;
                    case "import":
                        {
                            string selected = Path.Combine(Root, "selected-iso.txt");
                            if (!File.Exists(selected)) throw new IOException("Select and validate your ISO first.");
                            string iso = File.ReadAllText(selected, Encoding.UTF8);
                            string installation = InstalledGame.Installation(Root);
                            string destination = Path.Combine(installation, "game");
                            if (Directory.Exists(destination)) throw new IOException("This package already contains an imported game. Your existing game and saves were preserved.");
                            string staging = Path.Combine(installation, "import-" + Guid.NewGuid().ToString("N"));
                            InstalledGame.VerifyRuntime(Root);
                            Status("Verifying ISO structure and supported edition…");
                            DateTime last = DateTime.MinValue;
                            using (var image = new IsoImage(iso)) image.ExtractFresh(staging, CancellationToken.None, delegate(long done, long total, string file) {
                                if ((DateTime.UtcNow - last).TotalMilliseconds < 250 && done != total) return;
                                last = DateTime.UtcNow;
                                Status("Preparing: " + (total == 0 ? 0 : done * 99 / total) + "% — " + file);
                            });
                            Directory.Move(staging, destination); InstalledGame.CompleteImport(Root);
                            try {CreateDesktopShortcut();Status("Preparing: 100% — Ready to play. Desktop shortcut created.");}
                            catch(Exception shortcutError) {Diagnostic("Shortcut creation failed: "+shortcutError);Status("Preparing: 100% — Ready to play. Desktop shortcut could not be created.");}
                        }
                        break;
                    default: throw new ArgumentException("Unknown action.");
                }
                return 0;
            } catch (Exception error) {
                try { Diagnostic(error.ToString()); } catch { }
                try { Status("Error: " + error.Message); } catch { }
                MessageBox.Show(error.Message+Environment.NewLine+Environment.NewLine+"Startup diagnostics: "+InstalledGame.LauncherLog(Root), "Cosmic Recomp", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return 1;
            }
        }
    }
}

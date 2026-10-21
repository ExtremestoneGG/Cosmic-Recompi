using System;
using System.IO;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace CosmicRecompi {
    // Narrow process boundary: the native UI never parses or extracts an ISO.
    internal static class NativeBridge {
        static readonly string Root = AppDomain.CurrentDomain.BaseDirectory;
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
                        if(!File.Exists(menuApi) || File.ReadAllText(menuApi).Trim()!="1")
                            throw new IOException("This launcher requires the matching Cosmic Recomp game runtime. Extract the complete new release into a separate folder.");
                        string returnEventName = "Local\\CosmicRecompReturn-" + Guid.NewGuid().ToString("N");
                        bool returnRequested;
                        using (var returnEvent = new EventWaitHandle(false, EventResetMode.ManualReset, returnEventName)) {
                        using (var game = InstalledGame.Start(Root, false, returnEventName, delegate {
                            GameMenuPatch.Ensure(Path.Combine(InstalledGame.Installation(Root),"game"));
                        }, configureProcess)) {
                            Status("Game is running."); game.WaitForExit();
                            if (game.ExitCode != 0) throw new IOException("The game exited with an error. Check the installation logs.");
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
                    case "import":
                        using (var dialog = new OpenFileDialog { Filter = "Xbox 360 ISO (*.iso)|*.iso", CheckFileExists = true, RestoreDirectory = true }) {
                            if (dialog.ShowDialog() != DialogResult.OK) { Status("Import cancelled."); break; }
                            string installation = InstalledGame.Installation(Root);
                            string destination = Path.Combine(installation, "game");
                            if (Directory.Exists(destination)) throw new IOException("The existing import was preserved. Choose another folder for a new installation.");
                            string staging = Path.Combine(installation, "import-" + Guid.NewGuid().ToString("N"));
                            InstalledGame.VerifyRuntime(Root);
                            Status("Verifying ISO structure and supported edition…");
                            DateTime last = DateTime.MinValue;
                            using (var image = new IsoImage(dialog.FileName)) image.ExtractFresh(staging, CancellationToken.None, delegate(long done, long total, string file) {
                                if ((DateTime.UtcNow - last).TotalMilliseconds < 250 && done != total) return;
                                last = DateTime.UtcNow;
                                Status("Importing: " + (total == 0 ? 0 : done * 100 / total) + "% — " + file);
                            });
                            Directory.Move(staging, destination); InstalledGame.CompleteImport(Root);
                            Status("ISO imported. Ready to play.");
                        }
                        break;
                    default: throw new ArgumentException("Unknown action.");
                }
                return 0;
            } catch (Exception error) {
                try { Status("Error: " + error.Message); } catch { }
                MessageBox.Show(error.Message, "Cosmic Recomp", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return 1;
            }
        }
    }
}

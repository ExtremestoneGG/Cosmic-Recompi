using System;
using System.Collections;
using System.Diagnostics;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace CosmicRecompi {
    internal static class InstalledGame {
        internal static string LauncherLog(string root) {
            try {
                string logs=Path.Combine(Installation(root),"logs");Directory.CreateDirectory(logs);
                return Path.Combine(logs,"launcher.log");
            }catch{return Path.Combine(root,"launcher.log");}
        }
        public static string Installation(string root) {
            string preference = Path.Combine(root, "installation-path.txt");
            if (File.Exists(preference)) {
                string chosen = File.ReadAllText(preference, Encoding.UTF8).Trim();
                if (!String.IsNullOrEmpty(chosen) && Path.IsPathRooted(chosen)) return Path.GetFullPath(chosen);
                throw new IOException("The saved installation path is invalid.");
            }
            return Path.Combine(root, "installation");
        }
        public static void ChooseInstallation(string root, string path) {
            string full = Path.GetFullPath(path);
            if (!Path.IsPathRooted(path)) throw new IOException("Select an absolute folder path.");
            File.WriteAllText(Path.Combine(root, "installation-path.txt"), full, new UTF8Encoding(false));
        }
        public static bool Ready(string root) {
            return File.Exists(Path.Combine(Installation(root), "installed.txt")) &&
                File.Exists(Path.Combine(Installation(root), "game", "default.xex")) &&
                File.Exists(Path.Combine(root, "runtime", "ben10.exe"));
        }
        public static void VerifyRuntime(string root) {
            string folder = Path.Combine(root, "runtime");
            string manifest = Path.Combine(folder, "SHA256SUMS.txt");
            if (!File.Exists(manifest)) throw new IOException("Incomplete package: runtime manifest missing. Extract the complete ZIP first.");
            bool game = false, gpu = false, kernel = false;
            foreach (string line in File.ReadAllLines(manifest)) {
                if (String.IsNullOrWhiteSpace(line)) continue;
                int split = line.IndexOf("  ", StringComparison.Ordinal);
                if (split != 64) throw new InvalidDataException("Invalid runtime manifest.");
                string name = line.Substring(split + 2);
                if (Path.GetFileName(name) != name || name.Contains(":")) throw new InvalidDataException("Invalid path in the manifest.");
                string path = Path.Combine(folder, name);
                using (SHA256 sha = SHA256.Create())
                using (FileStream file = File.OpenRead(path)) {
                    string hash = BitConverter.ToString(sha.ComputeHash(file)).Replace("-", "");
                    if (!hash.Equals(line.Substring(0, 64), StringComparison.OrdinalIgnoreCase))
                        throw new IOException("Incomplete or modified file: " + name);
                }
                game |= name == "ben10.exe"; gpu |= name == "rexgpu-xenos.dll"; kernel |= name == "rexruntime.dll";
            }
            if (!game || !gpu || !kernel) throw new IOException("The manifest does not include the required components.");
        }
        public static void VerifyImportedExecutable(string root) {
            using (SHA256 sha = SHA256.Create())
            using (FileStream file = File.OpenRead(Path.Combine(Installation(root), "game", "default.xex"))) {
                string hash = BitConverter.ToString(sha.ComputeHash(file)).Replace("-", "").ToLowerInvariant();
                if (hash != IsoImage.SupportedXexSha256) throw new IOException("The imported Xbox executable does not match the supported edition.");
            }
        }
        public static void CompleteImport(string root) {
            string folder = Installation(root);
            Directory.CreateDirectory(Path.Combine(folder, "user"));
            Directory.CreateDirectory(Path.Combine(folder, "cache"));
            Directory.CreateDirectory(Path.Combine(folder, "metadata"));
            Directory.CreateDirectory(Path.Combine(folder, "logs"));
            File.WriteAllText(Path.Combine(folder, "installed.txt"), "Cosmic Recomp test-0.1\r\n" + IsoImage.SupportedXexSha256 + "\r\n", new UTF8Encoding(false));
        }
        public static Process Start(string root, bool uncapped, string returnToLauncherEvent = null, Action prepareGameMenu = null, Action<ProcessStartInfo> configureProcess = null) {
            string diagnostics=LauncherLog(root);
            try {
                File.AppendAllText(diagnostics,DateTime.UtcNow.ToString("o")+" Starting game verification."+Environment.NewLine);
                return StartCore(root,uncapped,returnToLauncherEvent,prepareGameMenu,configureProcess,diagnostics);
            } catch(Exception error) {
                try {File.AppendAllText(diagnostics,DateTime.UtcNow.ToString("o")+" Launch failed: "+error+Environment.NewLine);} catch {}
                throw new IOException("The game could not start. Startup diagnostics: "+diagnostics,error);
            }
        }
        static Process StartCore(string root, bool uncapped, string returnToLauncherEvent, Action prepareGameMenu, Action<ProcessStartInfo> configureProcess,string diagnostics) {
            if (!Ready(root)) throw new IOException("Import your ISO before starting the game.");
            VerifyRuntime(root);
            VerifyImportedExecutable(root);
            string folder = Installation(root);
            FileStream runLock;
            try { runLock = new FileStream(Path.Combine(folder, "running.lock"), FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None); }
            catch (IOException) { throw new IOException("This installation is already in use. Close the other game or launcher session first."); }
            try {
            if (prepareGameMenu != null) prepareGameMenu();
            string log = Path.Combine(folder, "logs", "runtime.log").Replace('\\', '/');
            string resolution = "720p", frameLimit = "30",fullscreen="false";
            string video = Path.Combine(folder, "user", "pc-video.ini");
            if (File.Exists(video)) foreach (string setting in File.ReadAllLines(video)) {
                if (setting == "resolution=720p") resolution = "720p";
                if (setting == "resolution=1080p") resolution = "1080p";
                if (setting == "resolution=1440p") resolution = "1440p";
                if (setting == "resolution=4k") resolution = "4k";
                if (setting == "frame_limit=30") frameLimit = "30";
                if (setting == "frame_limit=60") frameLimit = "60";
                if (setting == "frame_limit=unlimited") frameLimit = "unlimited";
                if (setting == "display_mode=borderless") fullscreen="true";
                if (setting == "display_mode=windowed") fullscreen="false";
            }
            // These are launcher-owned settings. User saves live in a different directory.
            string config = "resolution = \"" + resolution + "\"\nfullscreen = "+fullscreen+"\nmnk_mode = true\ninput_backend = \"sdl\"\ngpu_allow_invalid_fetch_constants = true\nlog_level = \"info\"\nlog_file = \"" + log.Replace("\"", "\\\"") + "\"\n";
            File.WriteAllText(Path.Combine(folder, "runtime.toml"), config, new UTF8Encoding(false));
            ProcessStartInfo start = new ProcessStartInfo(Path.Combine(root, "runtime", "ben10.exe"));
            // Keep relative diagnostic output inside the selected installation.
            // Command-line settings also avoid relying on the SDK TOML reader's
            // narrow filename handling for Windows paths containing accents.
            start.UseShellExecute = false; start.WorkingDirectory = folder;
            start.Arguments = "--gpu_plugin xenos --game_data_root \"" + Path.Combine(folder, "game") + "\" --user_data_root \"" + Path.Combine(folder, "user") + "\" --cache_root \"" + Path.Combine(folder, "cache") + "\"";
            start.Arguments += " --resolution " + resolution + " --fullscreen "+fullscreen+" --mnk_mode true --input_backend sdl --gpu_allow_invalid_fetch_constants true --log_level info --log_file logs/runtime.log";
            foreach (DictionaryEntry variable in Environment.GetEnvironmentVariables())
                if (((string)variable.Key).StartsWith("BEN10_", StringComparison.OrdinalIgnoreCase)) start.EnvironmentVariables.Remove((string)variable.Key);
            start.EnvironmentVariables["BEN10_PORTABLE_ROOT"] = folder;
            if (returnToLauncherEvent != null)
                start.EnvironmentVariables["BEN10_RETURN_TO_LAUNCHER_EVENT"] = returnToLauncherEvent;
            start.EnvironmentVariables["BEN10_GRAPHICS_BACKEND"] = "d3d12";
            start.EnvironmentVariables["BEN10_RENDER_TARGET_PATH"] = "rtv";
            start.EnvironmentVariables["BEN10_RESOLUTION"] = resolution;
            start.EnvironmentVariables["BEN10_WINDOW_MODE"] = fullscreen=="true"?"borderless":"windowed";
#if COSMIC_NATIVE_SETTINGS
            start.EnvironmentVariables["BEN10_PRESENTATION_API"] = "1";
#endif
            start.EnvironmentVariables["BEN10_INTEL_NATIVE_STENCIL"] = "true";
            start.EnvironmentVariables["BEN10_INTEL_VIEWPORT"] = "true";
            start.EnvironmentVariables["BEN10_INTEL_STENCIL"] = "true";
            foreach (string flag in new string[] { "RTV_INITIAL_DEPTH_ZERO", "RTV_UNSIGNED_TEXTURES", "RTV_ZERO_LOD", "REUSE_UPLOADS", "DIRECT_RESOLVE", "ESTIMATE_DRAW_EXTENT", "IDLE_NOTIFICATION_PACING" })
                start.EnvironmentVariables["BEN10_" + flag] = "1";
            if (!uncapped && frameLimit != "unlimited") start.EnvironmentVariables["BEN10_FRAME_PACING_HZ"] = frameLimit;
            string temp = Path.Combine(folder, "temp"); Directory.CreateDirectory(temp);
            start.EnvironmentVariables["TEMP"] = temp; start.EnvironmentVariables["TMP"] = temp;
            if (configureProcess != null) configureProcess(start);
            start.RedirectStandardOutput=true;start.RedirectStandardError=true;
            string output=Path.Combine(folder,"logs","startup-output.log");
            File.WriteAllText(output,DateTime.UtcNow.ToString("o")+" Launching game. Resolution="+resolution+"; frame limit="+frameLimit+Environment.NewLine);
            File.AppendAllText(diagnostics,DateTime.UtcNow.ToString("o")+" Process.Start: "+start.FileName+"; early output: "+output+Environment.NewLine);
            Process process = Process.Start(start);
            object logLock=new object();
            DataReceivedEventHandler capture=delegate(object sender,DataReceivedEventArgs args) {
                if(args.Data==null)return;
                lock(logLock) {try {File.AppendAllText(output,args.Data+Environment.NewLine);}catch {}}
            };
            process.OutputDataReceived+=capture;process.ErrorDataReceived+=capture;
            process.BeginOutputReadLine();process.BeginErrorReadLine();
            process.Exited += delegate {
                try {lock(logLock)File.AppendAllText(diagnostics,DateTime.UtcNow.ToString("o")+" Game exit code: 0x"+process.ExitCode.ToString("X8")+Environment.NewLine);}catch {}
                runLock.Dispose();
            };
            process.EnableRaisingEvents = true;
            return process;
            } catch { runLock.Dispose(); throw; }
        }
    }
}

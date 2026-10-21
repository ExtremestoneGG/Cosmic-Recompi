using System;
using System.Collections;
using System.Diagnostics;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace CosmicRecompi {
    internal static class InstalledGame {
        public static string Installation(string root) { return Path.Combine(root, "installation"); }
        public static bool Ready(string root) {
            return File.Exists(Path.Combine(Installation(root), "installed.txt")) &&
                File.Exists(Path.Combine(Installation(root), "game", "default.xex")) &&
                File.Exists(Path.Combine(root, "runtime", "ben10.exe"));
        }
        public static void VerifyRuntime(string root) {
            string folder = Path.Combine(root, "runtime");
            string manifest = Path.Combine(folder, "SHA256SUMS.txt");
            if (!File.Exists(manifest)) throw new IOException("Pacote incompleto: manifesto do runtime ausente. Extraia o ZIP inteiro antes de abrir o programa.");
            bool game = false, gpu = false, kernel = false;
            foreach (string line in File.ReadAllLines(manifest)) {
                if (String.IsNullOrWhiteSpace(line)) continue;
                int split = line.IndexOf("  ", StringComparison.Ordinal);
                if (split != 64) throw new InvalidDataException("Manifesto do runtime inválido.");
                string name = line.Substring(split + 2);
                if (Path.GetFileName(name) != name || name.Contains(":")) throw new InvalidDataException("Caminho inválido no manifesto.");
                string path = Path.Combine(folder, name);
                using (SHA256 sha = SHA256.Create())
                using (FileStream file = File.OpenRead(path)) {
                    string hash = BitConverter.ToString(sha.ComputeHash(file)).Replace("-", "");
                    if (!hash.Equals(line.Substring(0, 64), StringComparison.OrdinalIgnoreCase))
                        throw new IOException("Arquivo incompleto ou alterado: " + name);
                }
                game |= name == "ben10.exe"; gpu |= name == "rexgpu-xenos.dll"; kernel |= name == "rexruntime.dll";
            }
            if (!game || !gpu || !kernel) throw new IOException("O manifesto não contém os componentes obrigatórios.");
        }
        public static void VerifyImportedExecutable(string root) {
            using (SHA256 sha = SHA256.Create())
            using (FileStream file = File.OpenRead(Path.Combine(Installation(root), "game", "default.xex"))) {
                string hash = BitConverter.ToString(sha.ComputeHash(file)).Replace("-", "").ToLowerInvariant();
                if (hash != IsoImage.SupportedXexSha256) throw new IOException("O executável Xbox importado não corresponde à edição suportada.");
            }
        }
        public static void CompleteImport(string root) {
            string folder = Installation(root);
            Directory.CreateDirectory(Path.Combine(folder, "user"));
            Directory.CreateDirectory(Path.Combine(folder, "cache"));
            Directory.CreateDirectory(Path.Combine(folder, "metadata"));
            Directory.CreateDirectory(Path.Combine(folder, "logs"));
            File.WriteAllText(Path.Combine(folder, "installed.txt"), "Cosmic Recompi test-0.1\r\n" + IsoImage.SupportedXexSha256 + "\r\n", new UTF8Encoding(false));
        }
        public static Process Start(string root, bool uncapped) {
            if (!Path.GetPathRoot(Path.GetFullPath(root)).Equals("D:\\", StringComparison.OrdinalIgnoreCase))
                throw new IOException("Esta distribuição deve permanecer no disco D:.");
            if (!Ready(root)) throw new IOException("Prepare sua ISO antes de iniciar o jogo.");
            VerifyRuntime(root);
            VerifyImportedExecutable(root);
            string folder = Installation(root);
            string log = Path.Combine(folder, "logs", "runtime.log").Replace('\\', '/');
            // These are launcher-owned settings. User saves live in a different directory.
            string config = "resolution = \"720p\"\nfullscreen = false\nmnk_mode = true\ninput_backend = \"sdl\"\ngpu_allow_invalid_fetch_constants = true\nlog_level = \"info\"\nlog_file = \"" + log.Replace("\"", "\\\"") + "\"\n";
            File.WriteAllText(Path.Combine(folder, "runtime.toml"), config, new UTF8Encoding(false));
            ProcessStartInfo start = new ProcessStartInfo(Path.Combine(root, "runtime", "ben10.exe"));
            start.UseShellExecute = false; start.WorkingDirectory = Path.Combine(root, "runtime");
            start.Arguments = "--gpu_plugin xenos --game_data_root \"" + Path.Combine(folder, "game") + "\" --user_data_root \"" + Path.Combine(folder, "user") + "\" --cache_root \"" + Path.Combine(folder, "cache") + "\"";
            foreach (DictionaryEntry variable in Environment.GetEnvironmentVariables())
                if (((string)variable.Key).StartsWith("BEN10_", StringComparison.OrdinalIgnoreCase)) start.EnvironmentVariables.Remove((string)variable.Key);
            start.EnvironmentVariables["BEN10_PORTABLE_ROOT"] = folder;
            start.EnvironmentVariables["BEN10_GRAPHICS_BACKEND"] = "d3d12";
            start.EnvironmentVariables["BEN10_RENDER_TARGET_PATH"] = "rtv";
            start.EnvironmentVariables["BEN10_RESOLUTION"] = "720p";
            start.EnvironmentVariables["BEN10_INTEL_NATIVE_STENCIL"] = "true";
            start.EnvironmentVariables["BEN10_INTEL_VIEWPORT"] = "true";
            start.EnvironmentVariables["BEN10_INTEL_STENCIL"] = "true";
            foreach (string flag in new string[] { "RTV_INITIAL_DEPTH_ZERO", "RTV_UNSIGNED_TEXTURES", "RTV_ZERO_LOD", "REUSE_UPLOADS", "DIRECT_RESOLVE", "ESTIMATE_DRAW_EXTENT", "IDLE_NOTIFICATION_PACING" })
                start.EnvironmentVariables["BEN10_" + flag] = "1";
            if (!uncapped) start.EnvironmentVariables["BEN10_FRAME_PACING_HZ"] = "30";
            string temp = Path.Combine(folder, "temp"); Directory.CreateDirectory(temp);
            start.EnvironmentVariables["TEMP"] = temp; start.EnvironmentVariables["TMP"] = temp;
            return Process.Start(start);
        }
    }
}

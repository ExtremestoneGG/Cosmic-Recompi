using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Threading;

namespace CosmicRecompi {
    // Original files are read only from the user's image. No game data is embedded.
    public sealed class IsoImage : IDisposable {
        public const string SupportedXexSha256 = "4e8c3b121081ce26fd3ec61f28e935951a755d7015ae42e23a2c7899b730dd89";
        public sealed class Entry {
            public string Name;
            public long Offset, Size;
        }
        readonly FileStream stream;
        readonly List<Entry> files = new List<Entry>();
        readonly HashSet<long> directories = new HashSet<long>();
        readonly HashSet<string> names = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        long partition;
        public IList<Entry> Files { get { return files.AsReadOnly(); } }
        public long TotalBytes { get; private set; }
        public IsoImage(string path) {
            stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
            try {
                byte[] descriptor = null;
                foreach (long offset in new long[] { 0, 0x2080000, 0xFD90000, 0x2000000 }) {
                    if (offset + 65536 + 2048 > stream.Length) continue;
                    byte[] candidate = Read(offset + 65536, 2048);
                    if (Encoding.ASCII.GetString(candidate, 0, 20) != "MICROSOFT*XBOX*MEDIA") continue;
                    if (Encoding.ASCII.GetString(candidate, 2028, 20) != "MICROSOFT*XBOX*MEDIA")
                        throw new InvalidDataException("Invalid closing XDVDFS signature.");
                    partition = offset; descriptor = candidate; break;
                }
                if (descriptor == null) throw new InvalidDataException("The ISO does not contain a recognized Xbox partition.");
                Directory(BitConverter.ToUInt32(descriptor, 20), BitConverter.ToUInt32(descriptor, 24), "", 0);
                if (!names.Contains("default.xex")) throw new InvalidDataException("default.xex was not found at the ISO root.");
            } catch { stream.Dispose(); throw; }
        }
        byte[] Read(long offset, int count) {
            if (offset < 0 || count < 0 || offset > stream.Length - count)
                throw new InvalidDataException("The ISO is truncated or contains invalid offsets.");
            stream.Position = offset;
            byte[] buffer = new byte[count]; int used = 0;
            while (used < count) {
                int n = stream.Read(buffer, used, count - used);
                if (n == 0) throw new EndOfStreamException("Incomplete ISO read.");
                used += n;
            }
            return buffer;
        }
        static void ValidateName(string name) {
            if (name.Length == 0 || name == "." || name == ".." ||
                name.IndexOfAny(Path.GetInvalidFileNameChars()) >= 0 ||
                name.EndsWith(".") || name.EndsWith(" "))
                throw new InvalidDataException("Invalid filename in the ISO.");
            string stem = name.Split('.')[0].ToUpperInvariant();
            if (stem == "CON" || stem == "PRN" || stem == "AUX" || stem == "NUL" ||
                stem == "CONIN$" || stem == "CONOUT$" ||
                (stem.Length == 4 && (stem.StartsWith("COM") || stem.StartsWith("LPT")) &&
                 stem[3] >= '0' && stem[3] <= '9'))
                throw new InvalidDataException("Windows device names are not allowed in the ISO.");
        }
        void Directory(uint sector, uint size, string prefix, int depth) {
            if (depth > 64 || size > 16 * 1024 * 1024 || !directories.Add(sector))
                throw new InvalidDataException("Cyclic or excessive directory structure in the ISO.");
            if (size == 0) return;
            byte[] data = Read(checked(partition + (long)sector * 2048), (int)size);
            Stack<int> pending = new Stack<int>(); pending.Push(0);
            HashSet<int> visited = new HashSet<int>();
            while (pending.Count != 0) {
                int p = pending.Pop();
                if (!visited.Add(p) || p < 0 || p > data.Length - 14)
                    throw new InvalidDataException("Invalid ISO file tree.");
                int len = data[p + 13];
                if (len == 0 || p + 14 + len > data.Length) throw new InvalidDataException("Truncated filename in the ISO.");
                for (int n = 0; n < len; ++n)
                    if (data[p + 14 + n] < 32 || data[p + 14 + n] > 126)
                        throw new InvalidDataException("Unsupported filename encoding in this ISO.");
                string name = Encoding.ASCII.GetString(data, p + 14, len); ValidateName(name);
                string full = prefix.Length == 0 ? name : prefix + "/" + name;
                if (!names.Add(full) || names.Count > 200000) throw new InvalidDataException("Duplicate files or excessive file count in the ISO.");
                uint childSector = BitConverter.ToUInt32(data, p + 4), bytes = BitConverter.ToUInt32(data, p + 8);
                long offset = checked(partition + (long)childSector * 2048);
                if (offset > stream.Length - bytes) throw new InvalidDataException("File outside ISO bounds.");
                if ((data[p + 12] & 16) != 0) Directory(childSector, bytes, full, depth + 1);
                else { files.Add(new Entry { Name = full, Offset = offset, Size = bytes }); TotalBytes = checked(TotalBytes + bytes); }
                int left = BitConverter.ToUInt16(data, p) * 4, right = BitConverter.ToUInt16(data, p + 2) * 4;
                if (right != 0) pending.Push(right); if (left != 0) pending.Push(left);
            }
        }
        public void VerifySupportedGame() {
            Entry xex = files.Find(delegate(Entry e) { return e.Name.Equals("default.xex", StringComparison.OrdinalIgnoreCase); });
            if (xex == null || xex.Size < 24 || xex.Size > 256 * 1024 * 1024)
                throw new InvalidDataException("Missing or invalid Xbox executable.");
            byte[] bytes = Read(xex.Offset, (int)xex.Size);
            string hash;
            using (SHA256 sha = SHA256.Create()) hash = BitConverter.ToString(sha.ComputeHash(bytes)).Replace("-", "").ToLowerInvariant();
            if (hash != SupportedXexSha256)
                throw new InvalidDataException("This game edition is not supported by this build. default.xex SHA-256: " + hash);
        }
        public void ExtractFresh(string destination, CancellationToken cancellation, Action<long, long, string> progress) {
            VerifySupportedGame(); cancellation.ThrowIfCancellationRequested();
            string root = Path.GetFullPath(destination).TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar;
            if (System.IO.Directory.Exists(root) || File.Exists(destination))
                throw new IOException("The temporary installation folder already exists; choose a new folder.");
            // Reject junction ancestors before writing any original game files.
            string ancestor = Path.GetDirectoryName(root.TrimEnd(Path.DirectorySeparatorChar));
            while (!String.IsNullOrEmpty(ancestor)) {
                if (System.IO.Directory.Exists(ancestor) && (File.GetAttributes(ancestor) & FileAttributes.ReparsePoint) != 0)
                    throw new IOException("The installation folder cannot traverse links or junctions.");
                ancestor = Path.GetDirectoryName(ancestor);
            }
            System.IO.Directory.CreateDirectory(root);
            byte[] buffer = new byte[1024 * 1024]; long completed = 0;
            foreach (Entry entry in files) {
                cancellation.ThrowIfCancellationRequested();
                string target = Path.GetFullPath(Path.Combine(root, entry.Name.Replace('/', Path.DirectorySeparatorChar)));
                if (!target.StartsWith(root, StringComparison.OrdinalIgnoreCase)) throw new InvalidDataException("Path outside the installation.");
                System.IO.Directory.CreateDirectory(Path.GetDirectoryName(target)); stream.Position = entry.Offset;
                using (FileStream output = new FileStream(target, FileMode.CreateNew, FileAccess.Write, FileShare.None)) {
                    long left = entry.Size;
                    while (left != 0) {
                        cancellation.ThrowIfCancellationRequested();
                        int n = stream.Read(buffer, 0, (int)Math.Min(left, buffer.Length));
                        if (n == 0) throw new EndOfStreamException("ISO read interrupted during extraction.");
                        output.Write(buffer, 0, n); left -= n; completed += n;
                        if (progress != null) progress(completed, TotalBytes, entry.Name);
                    }
                    output.Flush(true);
                }
            }
        }
        public void Dispose() { stream.Dispose(); }
    }
}

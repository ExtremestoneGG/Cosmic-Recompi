using System;
using System.IO;
using System.Text;
using CosmicRecompi;
internal static class IsoReaderTests {
    static byte[] Image(string name) {
        byte[] b = new byte[131072];
        byte[] magic = Encoding.ASCII.GetBytes("MICROSOFT*XBOX*MEDIA");
        Array.Copy(magic, 0, b, 65536, 20); Array.Copy(magic, 0, b, 65536 + 2028, 20);
        Array.Copy(BitConverter.GetBytes((uint)40), 0, b, 65536 + 20, 4);
        Array.Copy(BitConverter.GetBytes((uint)2048), 0, b, 65536 + 24, 4);
        int p = 40 * 2048; byte[] n = Encoding.ASCII.GetBytes("default.xex");
        Array.Copy(BitConverter.GetBytes((uint)45), 0, b, p + 4, 4);
        Array.Copy(BitConverter.GetBytes((uint)24), 0, b, p + 8, 4);
        b[p + 13] = (byte)n.Length; Array.Copy(n, 0, b, p + 14, n.Length);
        if (name != "default.xex") {
            b[p] = 32; // second node at offset 128, preserving the valid root XEX
            p += 128; n = Encoding.ASCII.GetBytes(name);
            Array.Copy(BitConverter.GetBytes((uint)45), 0, b, p + 4, 4);
            Array.Copy(BitConverter.GetBytes((uint)24), 0, b, p + 8, 4);
            b[p + 13] = (byte)n.Length; Array.Copy(n, 0, b, p + 14, n.Length);
        }
        return b;
    }
    static int Main(string[] args) {
        string root = args[0]; Directory.CreateDirectory(root); int count = 0;
        Action<string, byte[], bool> test = delegate(string name, byte[] bytes, bool valid) {
            string path = Path.Combine(root, name + ".img"); File.WriteAllBytes(path, bytes);
            bool accepted = false;
            try { using (IsoImage image = new IsoImage(path)) { accepted = true; } }
            catch (InvalidDataException) { }
            if (accepted != valid) throw new Exception("Unexpected parser result: " + name);
            ++count;
        };
        test("valid-catalog", Image("default.xex"), true);
        test("truncated", new byte[15], false);
        test("path-traversal", Image("../default.xex"), false);
        test("reserved-device", Image("CON"), false);
        test("trailing-dot", Image("default.xex."), false);
        byte[] bounds = Image("default.xex"); Array.Copy(BitConverter.GetBytes(uint.MaxValue), 0, bounds, 40 * 2048 + 4, 4); test("outside-image", bounds, false);
        byte[] cycle = Image("other.bin"); cycle[40 * 2048 + 128] = 32; test("directory-node-cycle", cycle, false);
        byte[] signature = Image("default.xex"); signature[65536 + 2028] = 0; test("bad-final-signature", signature, false);
        using (IsoImage image = new IsoImage(Path.Combine(root, "valid-catalog.img"))) {
            bool rejected = false; try { image.VerifySupportedGame(); } catch (InvalidDataException) { rejected = true; }
            if (!rejected) throw new Exception("Unsupported game accepted"); ++count;
        }
        Console.WriteLine("Passed " + count + " ISO parser and identity tests."); return 0;
    }
}

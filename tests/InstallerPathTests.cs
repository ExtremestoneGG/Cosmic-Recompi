using System;
using System.IO;
using CosmicRecompi;
internal static class InstallerPathTests {
    static int Main(string[] args) {
        string root = Path.GetFullPath(args[0]); Directory.CreateDirectory(root); int tests = 0;
        string original = InstalledGame.Installation(root);
        if (original != Path.Combine(root, "installation")) throw new Exception("Default path"); ++tests;
        Directory.CreateDirectory(Path.Combine(original, "user"));
        string sentinel = Path.Combine(original, "user", "synthetic-save.dat"); File.WriteAllText(sentinel, "synthetic save - not game data");
        string selected = Path.Combine(root, "Pasta escolhida — vídeo");
        InstalledGame.ChooseInstallation(root, selected);
        if (InstalledGame.Installation(root) != selected) throw new Exception("Unicode/space path round trip"); ++tests;
        if (File.ReadAllText(sentinel) != "synthetic save - not game data") throw new Exception("Previous saves altered"); ++tests;
        // Only a preference string is written on the test drive. No C: file is touched.
        InstalledGame.ChooseInstallation(root, @"C:\Games\CosmicRecompi");
        if (InstalledGame.Installation(root) != @"C:\Games\CosmicRecompi") throw new Exception("Other drive rejected"); ++tests;
        bool rejected = false; try { InstalledGame.ChooseInstallation(root, "relative-folder"); } catch (IOException) { rejected = true; }
        if (!rejected) throw new Exception("Relative destination accepted"); ++tests;
        InstalledGame.ChooseInstallation(root, selected);
        if (InstalledGame.Ready(root)) throw new Exception("Incomplete installation marked ready"); ++tests;
        Console.WriteLine("Passed " + tests + " installation-path and save-preservation tests."); return 0;
    }
}

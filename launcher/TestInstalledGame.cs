using System;
using CosmicRecompi;
internal static class TestInstalledGame {
    static int Main(string[] args) {
        try {
            InstalledGame.VerifyRuntime(args[0]);
            InstalledGame.CompleteImport(args[0]);
            if (!InstalledGame.Ready(args[0])) throw new Exception("Installation not ready");
            if (args.Length == 2 && args[1] == "--play") {
                var process = InstalledGame.Start(args[0], false);
                Console.WriteLine("Manual save test PID=" + process.Id);
            } else Console.WriteLine("Runtime hashes and installation readiness verified.");
            return 0;
        } catch (Exception error) { Console.Error.WriteLine(error); return 1; }
    }
}

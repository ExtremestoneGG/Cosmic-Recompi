using System;
using CosmicRecompi;
internal static class InspectIso {
    static int Main(string[] args) {
        try {
            using (IsoImage image = new IsoImage(args[0])) {
                image.VerifySupportedGame();
                Console.WriteLine("Supported ISO: {0} files, {1} bytes", image.Files.Count, image.TotalBytes);
                if (args.Length == 3 && args[1] == "--extract") {
                    DateTime last = DateTime.MinValue;
                    image.ExtractFresh(args[2], System.Threading.CancellationToken.None, delegate(long done, long total, string file) {
                        if ((DateTime.UtcNow - last).TotalSeconds >= 5 || done == total) {
                            last = DateTime.UtcNow; Console.WriteLine("{0}/{1} {2}", done, total, file);
                        }
                    });
                    Console.WriteLine("Extraction completed.");
                }
            }
            return 0;
        } catch (Exception error) { Console.Error.WriteLine(error.Message); return 1; }
    }
}

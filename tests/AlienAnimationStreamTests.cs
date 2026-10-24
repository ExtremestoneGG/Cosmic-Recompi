using System;
using System.IO;
using System.Collections.Generic;
namespace CosmicRecompi {
    class AlienAnimationStreamTests {
        static int Main(string[] args) {
            if(args.Length==2&&args[0]=="--ensure"){AlienAnimationStreams.Ensure(args[1]);return 0;}
            var archives=new List<byte[]>();
            for(int i=2;i<args.Length;i++)archives.Add(File.ReadAllBytes(args[i]));
            File.WriteAllBytes(args[1],AlienAnimationStreams.Merge(File.ReadAllBytes(args[0]),archives));
            return 0;
        }
    }
}

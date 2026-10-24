using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;

namespace CosmicRecompi {
    // Source for the authored program: tools/prepare_all_aliens_candidate.py.
    // Original instructions are read only from hash-verified user files.
    internal static class AllAliensPatch {
        static uint Read(byte[] b,int p) {
            if(p<0 || p>b.Length-4)throw new InvalidDataException("Truncated All Aliens script.");
            return ((uint)b[p]<<24)|((uint)b[p+1]<<16)|((uint)b[p+2]<<8)|b[p+3];
        }
        static uint Crc(string name) {
            uint c=0xffffffff;
            foreach(byte b in Encoding.ASCII.GetBytes(name.ToLowerInvariant())) {
                c^=b;for(int i=0;i<8;i++)c=(c>>1)^((c&1)!=0?0xedb88320U:0);
            }
            return c^0xffffffff;
        }
        static string Hash(byte[] b) {
            using(var s=SHA256.Create())return BitConverter.ToString(s.ComputeHash(b)).Replace("-","").ToLowerInvariant();
        }
        static void Word(Stream s,uint v){s.WriteByte((byte)(v>>24));s.WriteByte((byte)(v>>16));s.WriteByte((byte)(v>>8));s.WriteByte((byte)v);}
        static byte[] Bytes(IEnumerable<uint> words){using(var s=new MemoryStream()){foreach(uint w in words)Word(s,w);return s.ToArray();}}
        sealed class Record {public uint Id,Args;public byte[] Code;}
        static List<Record> Parse(byte[] b) {
            if(Read(b,0)!=0x1badbabe)throw new InvalidDataException("Invalid script signature.");
            var rows=new List<Record>();int p=8;uint count=Read(b,4);
            if(count>4096)throw new InvalidDataException("Unexpected script record count.");
            var ids=new HashSet<uint>();
            for(int i=0;i<count;i++) {
                var row=new Record {Id=Read(b,p),Args=Read(b,p+4)};
                int n=checked((int)Read(b,p+8)*4);p+=12;
                if(n>b.Length-p || !ids.Add(row.Id))throw new InvalidDataException("Invalid script record.");
                row.Code=new byte[n];Buffer.BlockCopy(b,p,row.Code,0,n);p+=n;rows.Add(row);
            }
            if(p!=b.Length)throw new InvalidDataException("Trailing script bytes.");return rows;
        }
        static byte[] Pack(List<Record> rows) {
            using(var s=new MemoryStream()) {
                Word(s,0x1badbabe);Word(s,(uint)rows.Count);
                foreach(var r in rows){Word(s,r.Id);Word(s,r.Args);Word(s,(uint)(r.Code.Length/4));s.Write(r.Code,0,r.Code.Length);}
                return s.ToArray();
            }
        }
        static List<int> Tokens(byte[] b) {
            var positions=new List<int>();int p=0;
            while(p<b.Length) {
                positions.Add(p/4);uint kind=Read(b,p)&255;int size=4;
                if(kind==3 || kind==5 || kind==7 || kind==8 || kind==0x17 || kind==0x19 || kind==0x20)size=8;
                else if(kind==4) {
                    int end=Array.IndexOf(b,(byte)0,p+4);
                    if(end<0)throw new InvalidDataException("Unterminated script literal.");
                    size=((end+1-p+3)/4)*4;
                }else if(kind<1 || kind>0x22)throw new InvalidDataException("Unexpected script opcode.");
                p=checked(p+size);if(p>b.Length)throw new InvalidDataException("Truncated script token.");
            }
            return positions;
        }
        sealed class Fixup {public int Position,Target,Shift;public bool Local;}
        sealed class Injection {
            public List<uint> Words=new List<uint>();public List<Fixup> Fixups=new List<Fixup>();
            public void Add(params uint[] words){Words.AddRange(words);}
            public void Branch(int reg,int target,bool local=false) {
                Fixups.Add(new Fixup{Position=Words.Count,Target=target,Shift=16,Local=local});Words.Add((uint)(reg<<8)|0x1f);
            }
            public void Jump(int target,bool local=false) {
                Fixups.Add(new Fixup{Position=Words.Count,Target=target,Shift=8,Local=local});Words.Add(0x1d);
            }
        }
        static Injection Hook(string name) {
            var i=new Injection();i.Add(0x00001b07,Crc(name),0x00281b20,3);return i;
        }
        static byte[] Relocate(byte[] code,Dictionary<int,Injection> insertions) {
            var positions=Tokens(code);var boundaries=new HashSet<int>(positions);boundaries.Add(code.Length/4);
            foreach(int p in insertions.Keys)if(!boundaries.Contains(p))throw new InvalidDataException("Hook inside script token.");
            Func<int,int> map=delegate(int p){int q=p;foreach(var i in insertions)if(i.Key<p)q+=i.Value.Words.Count;return q;};
            var output=new List<uint>();
            for(int i=0;i<positions.Count;i++) {
                int p=positions[i],next=i+1<positions.Count?positions[i+1]:code.Length/4;
                Injection added;
                if(insertions.TryGetValue(p,out added)) {
                    uint[] words=added.Words.ToArray();int start=output.Count;
                    foreach(var f in added.Fixups)words[f.Position]|=(uint)(f.Local?start+f.Target:map(f.Target))<<f.Shift;
                    output.AddRange(words);
                }
                uint value=Read(code,p*4),kind=value&255;
                if(kind==0x1d || kind==0x1e || kind==0x1f) {
                    int shift=kind==0x1d?8:16,target=(int)(value>>shift);
                    if(!boundaries.Contains(target))throw new InvalidDataException("Invalid original branch.");
                    value=(value&((1U<<shift)-1))|((uint)map(target)<<shift);
                }
                output.Add(value);for(int k=p+1;k<next;k++)output.Add(Read(code,k*4));
            }
            byte[] result=Bytes(output);var targets=new HashSet<int>(Tokens(result));targets.Add(result.Length/4);
            foreach(int p in Tokens(result)) {
                uint value=Read(result,p*4),kind=value&255;
                if((kind==0x1d || kind==0x1e || kind==0x1f) && !targets.Contains((int)(value>>(kind==0x1d?8:16))))
                    throw new InvalidDataException("Invalid relocated branch.");
            }
            return result;
        }
        static Injection EmptySlotGuard(int nextIteration) {
            var i=new Injection();
            i.Add(0x00002d07,Crc("gFormHotKey"),0x1f1d2d18,0x00201f17,Crc("formID"));
            // Present form -> original HUD path. Empty form -> original blank
            // portrait routine, then skip all gFormData[nil] reads.
            i.Branch(32,7,true);i.Jump(15,true);
            i.Add(0x00281806,0x00291d06,0x00001b07,Crc("CosmicEmptyAlienIcon"),0x02281b20,31);
            i.Jump(nextIteration);
            i.Fixups[1].Target=i.Words.Count;
            return i;
        }
        public static byte[] Frontend(byte[] original) {
            var rows=Parse(original);int patched=0;
            foreach(var r in rows)if(r.Id==Crc("GameplayMenu_Enter")) {
                if(r.Code.Length!=241*4)throw new InvalidDataException("Unexpected Gameplay menu.");
                var i=new Injection();i.Add(
                    0x00000807,0x0fc54df4,0x00000905,Crc("cosmic_all_aliens_label"),
                    0x00001d07,Crc("overlay"),0x00281d06,0x001e1d17,Crc("cosmic_all_aliens_enabled"),0x01281e20,1,
                    0x00000203,0x3f800000,0x0a020110,
                    0x00000b07,Crc("CosmicToggleAllAliens"),
                    0x00280806,0x00290906,0x002a0a06,0x002b0b06,
                    0x00001e07,0xa6c36cbb,0x04281e20,3);
                r.Code=Relocate(r.Code,new Dictionary<int,Injection>{{96,i}});patched++;
            }
            if(patched!=1)throw new InvalidDataException("Gameplay menu missing.");
            foreach(var r in rows)if(r.Id==Crc("pausemenu_enter"))
                r.Code=Relocate(r.Code,new Dictionary<int,Injection>{{0,Hook("CosmicAlienRestore")}});
            rows.Add(new Record{Id=Crc("CosmicToggleAllAliens"),Args=1,Code=Bytes(new uint[]{
                0x00000507,Crc("overlay"),0x00000207,Crc("overlay"),0x00000217,Crc("cosmic_toggle_all_aliens"),0x01050020,1,0x21})});
            return Pack(rows);
        }
        public static byte[] English(byte[] original) {
            using(var s=new MemoryStream()) {
                s.Write(original,0,original.Length);byte[] label=Encoding.ASCII.GetBytes("ALL ALIENS\0");
                Word(s,Crc("cosmic_all_aliens_label"));Word(s,(uint)label.Length);s.Write(label,0,label.Length);return s.ToArray();
            }
        }
        public static byte[] Transform(string file,byte[] original) {
            var rows=Parse(original);int patched=0;
            if(file=="globals.ds") {
                foreach(var r in rows) {
                    if(r.Id==Crc("switch_form_update")){r.Code=Relocate(r.Code,new Dictionary<int,Injection>{{34,Hook("CosmicAlienUpdate")}});patched++;}
                    if(r.Id==Crc("destroy_form_switching")){r.Code=Relocate(r.Code,new Dictionary<int,Injection>{{0,Hook("CosmicAlienUnload")}});patched++;}
                    // Mission preload snapshots the four active hotkeys into
                    // another game-owned table. Keep temporary pages out of it.
                    if(r.Id==0xa67003d2){r.Code=Relocate(r.Code,new Dictionary<int,Injection>{{0,Hook("CosmicAlienRestore")}});patched++;}
                }
                using(var stream=Assembly.GetExecutingAssembly().GetManifestResourceStream("CosmicAllAliens.bin")) {
                    if(stream==null)throw new IOException("The launcher is missing its All Aliens program.");
                    using(var memory=new MemoryStream()){stream.CopyTo(memory);rows.AddRange(Parse(memory.ToArray()));}
                }
                if(patched!=3)throw new InvalidDataException("Original form lifecycle missing.");
            }else if(file=="gld_xbx.ds") {
                foreach(var r in rows)if(r.Id==0x66960347) {
                    if(r.Code.Length!=230*4)throw new InvalidDataException("Unexpected form input handler.");
                    var hooks=new Dictionary<int,Injection>();
                    foreach(int p in new int[]{55,98,141,184}){
                        var i=new Injection();i.Branch(4,207);
#if COSMIC_DISABLE_RATH
                        // Also guard old profiles/cheats: Rath must not reach
                        // the native transform even if already in a saved slot.
                        i.Add(0x00000807,0xe1003dc4,0x000b0820,8,
                            0x00090817,Crc("Rath"),0x05090411);
                        i.Branch(5,207);
#endif
                        hooks.Add(p,i);
                    }
                    r.Code=Relocate(r.Code,hooks);patched++;
                }
                if(patched!=1)throw new InvalidDataException("Original form input missing.");
            }else if(file=="circdisp.ds") {
                byte[] blank=null;
                foreach(var r in rows)if(r.Id==0x8f436ad3) {
                    if(r.Code.Length!=769*4)throw new InvalidDataException("Unexpected form HUD.");
                    if(Read(r.Code,49*4)!=0x00181f06 || Read(r.Code,635*4)!=0x00181f06)
                        throw new InvalidDataException("Original HUD icon capture missing.");
                    using(var s=new MemoryStream()) {
                        Word(s,0x00180006);Word(s,0x001d0106);
                        // Original blank portrait creation/position/animation.
                        s.Write(r.Code,227*4,(340-227)*4);
                        // The original placeholder texture is Ben, not an
                        // empty portrait. Hide the whole unused item through
                        // the original SetAlpha routine; the normal HUD path
                        // restores alpha when this slot has a form again.
                        foreach(uint w in new uint[]{0x00301806,0x00003103,0,
                            0x00002d07,0x41871bc9,0x02302d20,31,0x21})Word(s,w);
                        blank=s.ToArray();
                    }
                    // Both guards reuse register 31. Capture the returned icon
                    // into register 24 first; otherwise the guard's hotkey
                    // lookup would become the icon argument to a95e2870.
                    r.Code=Relocate(r.Code,new Dictionary<int,Injection>{{50,EmptySlotGuard(340)},{636,EmptySlotGuard(758)}});patched++;
                }
                if(patched!=1)throw new InvalidDataException("Original form HUD missing.");
                rows.Add(new Record{Id=Crc("CosmicEmptyAlienIcon"),Args=2,Code=blank});
            }else throw new ArgumentException("Unsupported script file.");
            return Pack(rows);
        }
        public static void Ensure(string game) {
            string scripts=Path.Combine(game,"global","scripts");
            string[] names={"globals.ds","gld_xbx.ds","circdisp.ds"};
            string[] hashes={"5bf03a8d2ce2cd8c26176139cd57aea63a15af5e6405a6449e265375ea7103c2",
                "4c090a5b10488ddccd742adf25bc606d2a3cf3c94d5f5ae14f6df60948f09256",
                "2748e4bc8810f9af390f92999acdedbc7ee3b4d4dcbe833606f41c61b36bb576"};
            var replacements=new Dictionary<string,byte[]>();var originals=new Dictionary<string,byte[]>();
            for(int i=0;i<names.Length;i++) {
                string path=Path.Combine(scripts,names[i]),backup=path+".cosmic-original";
                byte[] original=File.ReadAllBytes(File.Exists(backup)?backup:path);
                if(Hash(original)!=hashes[i])throw new InvalidDataException("Unsupported original form script: "+names[i]);
                byte[] updated=Transform(names[i],original),current=File.ReadAllBytes(path);
                if(Hash(current)==Hash(updated))continue;
                if(Hash(current)!=hashes[i])throw new IOException("Another version modified "+names[i]+". Import into a fresh folder.");
                replacements[path]=updated;originals[path]=original;
            }
            foreach(var item in replacements) {
                string path=item.Key,backup=path+".cosmic-original",temporary=path+".cosmic-new";
                if(!File.Exists(backup))File.WriteAllBytes(backup,originals[path]);
                File.WriteAllBytes(temporary,item.Value);
                // Replace the directory entry: never modify a hard-linked ISO
                // extraction file in place. Interrupted installs are resumable.
                File.Replace(temporary,path,null);
            }
        }
    }
}

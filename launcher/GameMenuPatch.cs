using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace CosmicRecompi {
    // Transform only the user's verified scripts. No original bytecode is bundled.
    internal static class GameMenuPatch {
        const string FrontendHash = "dfe41cb728d90d93fb685a5d19e1664bb08bbebc9056ef23e25e997a00eafabb";
        const string EnglishHash = "fd1571d2b528a1710a73f5af4b0a8e916b6e0623db1916f615975bdc10e08365";
        static uint Read(byte[] bytes, int offset) {
            if (offset < 0 || offset > bytes.Length - 4) throw new InvalidDataException("Truncated game script.");
            return ((uint)bytes[offset] << 24) | ((uint)bytes[offset+1] << 16) | ((uint)bytes[offset+2] << 8) | bytes[offset+3];
        }
        static void Write(byte[] bytes, int offset, uint value) {
            bytes[offset]=(byte)(value>>24); bytes[offset+1]=(byte)(value>>16); bytes[offset+2]=(byte)(value>>8); bytes[offset+3]=(byte)value;
        }
        static void Append(Stream stream, uint value) {
            byte[] bytes=new byte[4]; Write(bytes,0,value); stream.Write(bytes,0,4);
        }
        static string Hash(byte[] bytes) {
            using (SHA256 sha=SHA256.Create()) return BitConverter.ToString(sha.ComputeHash(bytes)).Replace("-", "").ToLowerInvariant();
        }
        static uint Crc(string text) {
            uint crc=0xffffffff;
            foreach (byte value in Encoding.ASCII.GetBytes(text.ToLowerInvariant())) {
                crc^=value;
                for(int bit=0;bit<8;bit++) crc=(crc>>1)^((crc&1)!=0 ? 0xedb88320U : 0);
            }
            return crc^0xffffffff;
        }
        static List<int> Tokens(byte[] code) {
            List<int> positions=new List<int>(); int cursor=0;
            while(cursor<code.Length) {
                positions.Add(cursor); int kind=(int)(Read(code,cursor)&255),size=4;
                if(kind==3 || kind==5 || kind==7 || kind==8 || kind==0x17 || kind==0x20) size=8;
                else if(kind==4) {
                    int end=Array.IndexOf(code,(byte)0,cursor+4);
                    if(end<0) throw new InvalidDataException("Unterminated script string.");
                    size=((end+1-cursor+3)/4)*4;
                } else if(kind!=1 && kind!=2 && kind!=0x0f && kind!=0x10 && kind!=0x11 && kind!=0x13 && kind!=0x14 && kind!=0x15 && kind!=0x1f && kind!=0x21)
                    throw new InvalidDataException("Unsupported menu token.");
                cursor=checked(cursor+size);
                if(cursor>code.Length) throw new InvalidDataException("Invalid menu token size.");
            }
            return positions;
        }
        static byte[] InsertRow(byte[] code,string label,string callback,int expectedCount) {
            List<int> positions=Tokens(code),calls=new List<int>();
            foreach(int p in positions)
                if((Read(code,p)&255)==7 && Read(code,p+4)==Crc("Menu_AddOption"))calls.Add(p);
            if(calls.Count!=expectedCount)throw new InvalidDataException("Unexpected original menu structure.");
            int begin=calls[calls.Count-1]-32,end=begin+48;
            byte[] row=new byte[48];Buffer.BlockCopy(code,begin,row,0,48);
            Write(row,12,Crc(label));Write(row,24,Crc(callback));
            byte[] moved=(byte[])code.Clone();
            foreach(int p in positions) {
                uint word=Read(code,p);
                if((word&255)==0x1f && (word>>16)>end/4)Write(moved,p,word+(12U<<16));
            }
            byte[] result=new byte[code.Length+48];
            Buffer.BlockCopy(moved,0,result,0,end);Buffer.BlockCopy(row,0,result,end,48);
            Buffer.BlockCopy(moved,end,result,end+48,code.Length-end);
            HashSet<int> boundaries=new HashSet<int>(Tokens(result));boundaries.Add(result.Length);
            foreach(int p in Tokens(result)) {
                uint word=Read(result,p);
                if((word&255)==0x1f && !boundaries.Contains(checked((int)(word>>16)*4)))throw new InvalidDataException("Invalid relocated menu branch.");
            }
            return result;
        }
        static void Wrapper(Stream stream,string name,string native) {
            Append(stream,Crc(name));Append(stream,1);Append(stream,native=="cosmic_video_options" ? 19U : 9U);
            foreach(uint word in new uint[] {0x00000507,Crc("overlay"),0x00000207,Crc("overlay"),0x00000217,Crc(native),0x01050020,1})Append(stream,word);
            if(native=="cosmic_video_options")foreach(uint word in new uint[] {0x00000507,Crc("overlay"),0x00000607,Crc("CosmicRestoreOptions"),0x00000207,Crc("overlay"),0x00000217,Crc("pushfunction"),0x02050020,1})Append(stream,word);
            Append(stream,0x21);
        }
        public static byte[] Frontend(byte[] original) {
            if(Hash(original)!=FrontendHash)throw new InvalidDataException("Unsupported original frontend script.");
            if(Read(original,0)!=0x1badbabe || Read(original,4)!=103)throw new InvalidDataException("Unsupported frontend header.");
            using(MemoryStream output=new MemoryStream()) {
                Append(output,0x1badbabe);Append(output,107);int cursor=8,patched=0;byte[] quit=null;uint quitArguments=0;
                for(int i=0;i<103;i++) {
                    uint identity=Read(original,cursor),arguments=Read(original,cursor+4);
                    int size=checked((int)Read(original,cursor+8)*4);
                    byte[] code=new byte[size];Buffer.BlockCopy(original,cursor+12,code,0,size);
                    if(identity==Crc("QuitGame")){quit=(byte[])code.Clone();quitArguments=arguments;}
                    if(identity==Crc("pausemenu_enter")){int anchors=0;foreach(int p in Tokens(code))if((Read(code,p)&255)==3 && Read(code,p+4)==0x3f066666){Write(code,p+4,0x3f166666);anchors++;}if(anchors!=1)throw new InvalidDataException("Unexpected pause list anchor.");code=InsertRow(code,"cosmic_quit_to_launcher_label","CosmicQuitToLauncher",5);patched++;}
                    if(identity==0xb4db5294){code=InsertRow(code,"cosmic_video_label","CosmicVideoOptions",3);patched++;}
                    Append(output,identity);Append(output,arguments);Append(output,(uint)(code.Length/4));output.Write(code,0,code.Length);
                    cursor=checked(cursor+12+size);
                }
                if(cursor!=original.Length || patched!=2)throw new InvalidDataException("Invalid original menu boundaries.");
                if(quit==null)throw new InvalidDataException("Original confirmation missing.");
                int replacements=0;
                foreach(int p in Tokens(quit))if((Read(quit,p)&255)==7 && Read(quit,p+4)==0xd4abb75d){Write(quit,p+4,Crc("CosmicExitToLauncher"));replacements++;}
                if(replacements!=1)throw new InvalidDataException("Unexpected confirmation layout.");
                Append(output,Crc("CosmicQuitToLauncher"));Append(output,quitArguments);Append(output,(uint)(quit.Length/4));output.Write(quit,0,quit.Length);
                Wrapper(output,"CosmicExitToLauncher","cosmic_quit_to_launcher");
                Wrapper(output,"CosmicVideoOptions","cosmic_video_options");
                uint[] restore={0x00000507,Crc("overlay"),0x00000207,Crc("overlay"),0x00000217,Crc("popfunction"),0x01050020,1,0x00000207,0x8610f32e,0x00000502,0x00050219,0x84fe6ee3,0x00000503,0,0x00050219,0xa393d2fb,0x00010502,0x00000207,Crc("MenuStack_ShowCurrentMenu"),0x01050220,1,0x21};
                Append(output,Crc("CosmicRestoreOptions"));Append(output,1);Append(output,(uint)restore.Length);foreach(uint word in restore)Append(output,word);
                return output.ToArray();
            }
        }
        public static byte[] English(byte[] original) {
            if(Hash(original)!=EnglishHash)throw new InvalidDataException("Unsupported original English text table.");
            using(MemoryStream output=new MemoryStream()) {
                output.Write(original,0,original.Length);
                foreach(string[] row in new string[][] {new string[] {"cosmic_quit_to_launcher_label","QUIT TO LAUNCHER"},new string[] {"cosmic_video_label","VIDEO"}}) {
                    byte[] text=Encoding.ASCII.GetBytes(row[1]+"\0");Append(output,Crc(row[0]));Append(output,(uint)text.Length);output.Write(text,0,text.Length);
                }
                return output.ToArray();
            }
        }
        public static void Ensure(string gameFolder) {
            string scripts=Path.Combine(gameFolder,"global","scripts");
            string frontend=Path.Combine(scripts,"frontend.ds"),english=Path.Combine(scripts,"xbx_us.txt");
            string frontendBackup=frontend+".cosmic-original",englishBackup=english+".cosmic-original";
            byte[] original=File.ReadAllBytes(File.Exists(frontendBackup)?frontendBackup:frontend);
            byte[] originalEnglish=File.ReadAllBytes(File.Exists(englishBackup)?englishBackup:english);
            byte[] updated=Frontend(original),updatedEnglish=English(originalEnglish);
            byte[] current=File.ReadAllBytes(frontend),currentEnglish=File.ReadAllBytes(english);
            string expected=Hash(updated),expectedEnglish=Hash(updatedEnglish);
            if(Hash(current)==expected && Hash(currentEnglish)==expectedEnglish)return;
            if(Hash(current)!=FrontendHash || Hash(currentEnglish)!=EnglishHash)
                throw new IOException("The game's menu files were modified by another version. Import your ISO into a new folder; your saves are preserved separately.");
            if(!File.Exists(frontendBackup))File.WriteAllBytes(frontendBackup,original);
            if(!File.Exists(englishBackup))File.WriteAllBytes(englishBackup,originalEnglish);
            string first=frontend+".cosmic-new",second=english+".cosmic-new";
            File.WriteAllBytes(first,updated);File.WriteAllBytes(second,updatedEnglish);
            try {
                File.Replace(first,frontend,null);File.Replace(second,english,null);
            } catch {
                // Restore only these launcher-owned changes, never saves or other assets.
                File.WriteAllBytes(frontend,original);File.WriteAllBytes(english,originalEnglish);
                throw;
            }
        }
    }
}

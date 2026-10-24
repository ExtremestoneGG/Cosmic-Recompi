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
                if(kind==3 || kind==5 || kind==7 || kind==8 || kind==0x17 || kind==0x19 || kind==0x20) size=8;
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
        static byte[] LegacyFrontend(byte[] original) {
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
        static readonly string[][] VideoChoices = {
            new string[]{"cosmic_resolution_720", "1280 X 720 (NATIVE)", "cosmic_set_720"},
            new string[]{"cosmic_resolution_1080", "1920 X 1080", "cosmic_set_1080"},
            new string[]{"cosmic_resolution_1440", "2560 X 1440", "cosmic_set_1440"},
            new string[]{"cosmic_resolution_4k", "3840 X 2160", "cosmic_set_4k"},
            new string[]{"cosmic_fps_30", "30 FPS", "cosmic_set_30"},
            new string[]{"cosmic_fps_60", "60 FPS", "cosmic_set_60"},
            new string[]{"cosmic_fps_unlimited", "UNLIMITED", "cosmic_set_unlimited"}
        };
        static void Function(Stream output,string name,uint arguments,byte[] code) {
            Append(output,Crc(name));Append(output,arguments);Append(output,(uint)(code.Length/4));output.Write(code,0,code.Length);
        }
        static byte[] NativeMenu(byte[] template,string menu,string title,string[][] rows) {
            // Preserve the original Options menu's VM registers, repeat-input,
            // intro animation and cancel callback. Only authored rows differ.
            using(MemoryStream code=new MemoryStream()) {
                byte[] header=new byte[37*4];Buffer.BlockCopy(template,0,header,0,header.Length);
                Write(header,12*4,Crc(title));
                foreach(int p in Tokens(header))if((Read(header,p)&255)==7 || (Read(header,p)&255)==8)
                    if(Read(header,p+4)==0x8610f32e)Write(header,p+4,Crc(menu));
                code.Write(header,0,header.Length);
                foreach(string[] row in rows)foreach(uint word in new uint[]{
                    0x00000607,Crc(menu),0x00000705,Crc(row[0]),0x00000801,
                    0x00000907,Crc(row[2]),0x00000a01,0x00000307,Crc("Menu_AddOption"),0x05060320,1})Append(code,word);
                foreach(uint word in new uint[]{0x00000607,Crc(menu),0x00000307,Crc("MenuStack_Push"),0x01060320,1,0x21})Append(code,word);
                return code.ToArray();
            }
        }
        static byte[] NativeMenuFrontend(byte[] original) {
            byte[] legacy=LegacyFrontend(original),template=null;
            int cursor=8;
            for(int i=0;i<103;i++) {
                int bytes=checked((int)Read(original,cursor+8)*4);
                if(Read(original,cursor)==0xb4db5294){template=new byte[bytes];Buffer.BlockCopy(original,cursor+12,template,0,bytes);}
                cursor+=12+bytes;
            }
            if(template==null)throw new InvalidDataException("Original Options menu missing.");
            using(MemoryStream output=new MemoryStream()) {
                Append(output,0x1badbabe);Append(output,Read(legacy,4)+9);cursor=8;
                for(int i=0;i<Read(legacy,4);i++) {
                    uint identity=Read(legacy,cursor);int bytes=checked((int)Read(legacy,cursor+8)*4);
                    if(identity==Crc("CosmicVideoOptions"))Function(output,"CosmicVideoOptions",1,NativeMenu(template,"cosmic_video_menu","cosmic_video_label",new string[][] {
                        new string[]{"cosmic_resolution_label","RESOLUTION (RESTART REQUIRED)","CosmicResolutionOptions"},
                        new string[]{"cosmic_frame_limit_label","FRAME LIMIT","CosmicFrameOptions"}}));
                    else output.Write(legacy,cursor,12+bytes);
                    cursor+=12+bytes;
                }
                Function(output,"CosmicResolutionOptions",1,NativeMenu(template,"cosmic_resolution_menu","cosmic_resolution_label",new string[][] {VideoChoices[0],VideoChoices[1],VideoChoices[2],VideoChoices[3]}));
                Function(output,"CosmicFrameOptions",1,NativeMenu(template,"cosmic_frame_menu","cosmic_frame_limit_label",new string[][] {VideoChoices[4],VideoChoices[5],VideoChoices[6]}));
                foreach(string[] choice in VideoChoices) {
                    using(MemoryStream callback=new MemoryStream()) {
                        foreach(uint word in new uint[]{0x00000507,Crc("overlay"),0x00000207,Crc("overlay"),0x00000217,Crc(choice[2]),0x01050020,1,0x21})Append(callback,word);
                        Function(output,choice[2]+"_row",1,callback.ToArray());
                    }
                }
                // Menu callbacks point at script wrappers; the native names are
                // invoked through the overlay's registered native methods.
                byte[] result=output.ToArray();
                cursor=8;
                for(int i=0;i<Read(result,4);i++) {
                    int bytes=checked((int)Read(result,cursor+8)*4);byte[] code=new byte[bytes];Buffer.BlockCopy(result,cursor+12,code,0,bytes);
                    uint identity=Read(result,cursor);
                    if(identity==Crc("CosmicResolutionOptions") || identity==Crc("CosmicFrameOptions"))
                        foreach(int p in Tokens(code))if((Read(code,p)&255)==7)
                            foreach(string[] choice in VideoChoices)if(Read(code,p+4)==Crc(choice[2]))Write(result,cursor+12+p+4,Crc(choice[2]+"_row"));
                    cursor+=12+bytes;
                }
                return result;
            }
        }
        public static byte[] Frontend(byte[] original) {
            return AllAliensPatch.Frontend(RathFrontend(original));
        }
        static byte[] RathFrontend(byte[] original) {
            byte[] menus=NativeMenuFrontend(original);
            using(MemoryStream output=new MemoryStream()) {
                Append(output,Read(menus,0));Append(output,Read(menus,4));int cursor=8,patched=0;
                for(int i=0;i<Read(menus,4);i++) {
                    uint identity=Read(menus,cursor),arguments=Read(menus,cursor+4);
                    int bytes=checked((int)Read(menus,cursor+8)*4);
                    byte[] code=new byte[bytes];Buffer.BlockCopy(menus,cursor+12,code,0,bytes);
                    if(identity==Crc("MainMenuText_Enter")) {
                        // The original Xbox "primus" branch sets
                        // postMission.unlocked[9] = true. Apply just that flag
                        // after loading a profile, before roster selection.
                        // Nil guards retain the original flow on invalid data.
                        uint[] prefix={0x00000107,Crc("postMission"),0x000a011f,
                            0x00020117,Crc("unlocked"),0x000a021f,
                            0x00000303,0x41100000,0x00010402,0x0403021a};
#if COSMIC_DISABLE_RATH
                        // Public test policy: keep Rath locked while his
                        // transformation resource failure is investigated.
                        prefix[8]=0x00000402;
#endif
                        var decoded=Tokens(code);var boundaries=new HashSet<int>(decoded);boundaries.Add(code.Length);
                        foreach(int p in decoded) {
                            uint word=Read(code,p);
                            if((word&255)==0x1f) {
                                if(!boundaries.Contains(checked((int)(word>>16)*4)))throw new InvalidDataException("Invalid main-menu branch.");
                                Write(code,p,word+((uint)prefix.Length<<16));
                            }
                        }
                        using(MemoryStream extended=new MemoryStream()) {
                            foreach(uint word in prefix)Append(extended,word);
                            extended.Write(code,0,code.Length);code=extended.ToArray();
                        }
                        patched++;
                    }
                    Append(output,identity);Append(output,arguments);Append(output,(uint)(code.Length/4));output.Write(code,0,code.Length);cursor+=12+bytes;
                }
                if(patched!=1 || cursor!=menus.Length)throw new InvalidDataException("Expected main-menu unlock point missing.");
                return output.ToArray();
            }
        }
        static byte[] LegacyEnglish(byte[] original) {
            if(Hash(original)!=EnglishHash)throw new InvalidDataException("Unsupported original English text table.");
            using(MemoryStream output=new MemoryStream()) {
                output.Write(original,0,original.Length);
                foreach(string[] row in new string[][] {new string[] {"cosmic_quit_to_launcher_label","QUIT TO LAUNCHER"},new string[] {"cosmic_video_label","VIDEO"}}) {
                    byte[] text=Encoding.ASCII.GetBytes(row[1]+"\0");Append(output,Crc(row[0]));Append(output,(uint)text.Length);output.Write(text,0,text.Length);
                }
                return output.ToArray();
            }
        }
        public static byte[] English(byte[] original) {
            return AllAliensPatch.English(NativeEnglish(original));
        }
        static byte[] NativeEnglish(byte[] original) {
            using(MemoryStream output=new MemoryStream()) {
                byte[] legacy=LegacyEnglish(original);output.Write(legacy,0,legacy.Length);
                var rows=new List<string[]>(VideoChoices);
                rows.Add(new string[]{"cosmic_resolution_label","RESOLUTION (RESTART REQUIRED)"});
                rows.Add(new string[]{"cosmic_frame_limit_label","FRAME LIMIT"});
                foreach(string[] row in rows) {
                    byte[] text=Encoding.ASCII.GetBytes(row[1]+"\0");Append(output,Crc(row[0]));Append(output,(uint)text.Length);output.Write(text,0,text.Length);
                }
                return output.ToArray();
            }
        }
        static HashSet<uint> TextKeys(byte[] bytes) {
            if(bytes.Length<4 || bytes.Length>16*1024*1024 || Read(bytes,0)!=0x1badbabe)throw new InvalidDataException("Invalid translation table header or size.");
            var keys=new HashSet<uint>();int cursor=4;
            while(cursor<bytes.Length) {
                uint key=Read(bytes,cursor),length=Read(bytes,cursor+4);
                if(length==0 || length>bytes.Length-cursor-8 || !keys.Add(key))throw new InvalidDataException("Truncated or duplicate translation entry.");
                int count=checked((int)length);
                if(bytes[cursor+8+count-1]!=0)throw new InvalidDataException("Translation text is not terminated.");
                cursor=checked(cursor+8+count);
            }
            if(cursor!=bytes.Length)throw new InvalidDataException("Invalid translation table boundary.");
            return keys;
        }
        public static byte[] MergeTranslation(byte[] original,byte[] translation) {
            byte[] authored=English(original); // Also verifies the supported original hash.
            if(!TextKeys(original).SetEquals(TextKeys(translation)))throw new InvalidDataException("The translation must contain exactly the supported game's text IDs.");
            using(var result=new MemoryStream()) {
                result.Write(translation,0,translation.Length);
                result.Write(authored,original.Length,authored.Length-original.Length);
                byte[] merged=result.ToArray();TextKeys(merged);return merged;
            }
        }
        public static void InstallTranslation(string gameFolder,string storedPatch,byte[] translation) {
            byte[] previous=File.Exists(storedPatch)?File.ReadAllBytes(storedPatch):null;
            Ensure(gameFolder,previous);
            string english=Path.Combine(gameFolder,"global","scripts","xbx_us.txt");
            byte[] original=File.ReadAllBytes(english+".cosmic-original");
            MergeTranslation(original,translation); // Validate before committing the source patch.
            Directory.CreateDirectory(Path.GetDirectoryName(storedPatch));
            string patchTemp=storedPatch+".new";
            File.WriteAllBytes(patchTemp,translation);
            // One canonical atomic commit. Generated text is rebuilt from this
            // source at every launch, including after an interrupted apply.
            if(previous==null)File.Move(patchTemp,storedPatch);else File.Replace(patchTemp,storedPatch,null);
            Ensure(gameFolder,translation);
        }
        static bool IsAuthoredTranslation(byte[] original,byte[] current) {
            try {
                foreach(byte[] authored in new byte[][]{English(original),NativeEnglish(original)}) {
                    int suffix=authored.Length-original.Length;
                    if(current.Length<suffix+4 || !TextKeys(authored).SetEquals(TextKeys(current)))continue;
                    bool matches=true;
                    for(int i=0;i<suffix;i++)if(authored[original.Length+i]!=current[current.Length-suffix+i]){matches=false;break;}
                    if(matches)return true;
                }
                return false;
            }catch(InvalidDataException){return false;}
        }
        public static void Ensure(string gameFolder,byte[] translation=null) {
            string scripts=Path.Combine(gameFolder,"global","scripts");
            string frontend=Path.Combine(scripts,"frontend.ds"),english=Path.Combine(scripts,"xbx_us.txt");
            string frontendBackup=frontend+".cosmic-original",englishBackup=english+".cosmic-original";
            byte[] original=File.ReadAllBytes(File.Exists(frontendBackup)?frontendBackup:frontend);
            byte[] originalEnglish=File.ReadAllBytes(File.Exists(englishBackup)?englishBackup:english);
            byte[] updated=Frontend(original),updatedEnglish=translation==null?English(originalEnglish):MergeTranslation(originalEnglish,translation);
            byte[] current=File.ReadAllBytes(frontend),currentEnglish=File.ReadAllBytes(english);
            string expected=Hash(updated),expectedEnglish=Hash(updatedEnglish);
            if(Hash(current)==expected && Hash(currentEnglish)==expectedEnglish)return;
            if((Hash(current)!=expected && Hash(current)!=FrontendHash && Hash(current)!=Hash(LegacyFrontend(original)) && Hash(current)!=Hash(NativeMenuFrontend(original)) && Hash(current)!=Hash(RathFrontend(original))) ||
               (Hash(currentEnglish)!=EnglishHash && Hash(currentEnglish)!=Hash(LegacyEnglish(originalEnglish)) && !IsAuthoredTranslation(originalEnglish,currentEnglish)))
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

"""Prepare a private menu candidate; never modify an installation or publish game data.

Only supported original hashes are accepted. The native callback still needs
runtime registration/validation before this candidate may be used in a release.
"""
import hashlib
import json
from pathlib import Path
import struct
import zlib

def crc(name):
    return zlib.crc32(name.lower().encode('ascii'))

def records(blob):
    magic, count = struct.unpack_from('>II', blob)
    if magic != 0x1badbabe:
        raise ValueError('Unsupported script signature')
    result = []
    cursor = 8
    for _ in range(count):
        identity, arguments, words = struct.unpack_from('>III', blob, cursor)
        end = cursor + 12 + words * 4
        if end > len(blob):
            raise ValueError('Truncated script record')
        result.append((identity, arguments, blob[cursor+12:end]))
        cursor = end
    if cursor != len(blob):
        raise ValueError('Unexpected trailing script data')
    return result

def tokens(code):
    """Decode only the token kinds actually encountered in the pause function."""
    offset = 0
    result = []
    while offset < len(code):
        value = struct.unpack_from('>I', code, offset)[0]
        kind = value & 255
        size = 4
        if kind in (3, 5, 7, 8, 0x17, 0x20):
            size = 8
        elif kind == 4:
            end = code.index(b'\0', offset+4)+1
            size = ((end-offset+3)//4)*4
        elif kind not in (1, 2, 0x0f, 0x10, 0x11, 0x13, 0x14, 0x15, 0x1f, 0x21):
            raise ValueError(f'Unreviewed token kind {kind:02x} at {offset:x}')
        if offset+size > len(code):
            raise ValueError('Token exceeds function')
        result.append((offset, kind, value, size))
        offset += size
    return result

def insert_menu_row(code, label, callback, expected_count):
    decoded=tokens(code)
    boundaries={p//4 for p,_,_,_ in decoded}|{len(code)//4}
    if any(kind==0x1f and (value>>16) not in boundaries for _,kind,value,_ in decoded):
        raise ValueError('Unexpected branch target')
    calls=[p for p,kind,_,_ in decoded if kind==7 and struct.unpack_from('>I',code,p+4)[0]==crc('Menu_AddOption')]
    if len(calls)!=expected_count: raise ValueError('Unexpected original option count')
    row_start,row_end=calls[-1]-32,calls[-1]+16
    row=bytearray(code[row_start:row_end])
    if len(row)!=48: raise ValueError('Unexpected menu row size')
    struct.pack_into('>I',row,12,crc(label))
    struct.pack_into('>I',row,24,crc(callback))
    updated=bytearray(code)
    for position,kind,value,_ in decoded:
        # A branch to the old insertion boundary must execute the added row.
        # Only destinations strictly beyond it move forward.
        if kind==0x1f and value>>16>row_end//4:
            struct.pack_into('>I',updated,position,value+(len(row)//4<<16))
    updated[row_end:row_end]=row
    boundaries={p//4 for p,_,_,_ in tokens(updated)}|{len(updated)//4}
    if any(kind==0x1f and (value>>16) not in boundaries for _,kind,value,_ in tokens(updated)):
        raise ValueError('Invalid relocated branch')
    return bytes(updated)

def extend_script(blob):
    functions=records(blob)
    if len(functions)!=103: raise ValueError('Unexpected original function count')
    changed=[]; patched=set()
    for identity,arguments,code in functions:
        if identity==crc('pausemenu_enter'):
            shifted=bytearray(code)
            anchors=[p for p,kind,_,_ in tokens(code) if kind==3 and struct.unpack_from('>I',code,p+4)[0]==0x3f066666]
            if len(anchors)!=1:raise ValueError('Unexpected pause list anchor')
            struct.pack_into('>I',shifted,anchors[0]+4,0x3f166666)
            code=bytes(shifted)
            code=insert_menu_row(code,'cosmic_quit_to_launcher_label','CosmicQuitToLauncher',5)
            patched.add(identity)
        if identity==0xb4db5294:
            code=insert_menu_row(code,'cosmic_video_label','CosmicVideoOptions',3)
            patched.add(identity)
        changed.append((identity,arguments,code))
    if len(patched)!=2: raise ValueError('Original menu entry functions absent')
    original_quit=next((a,c) for h,a,c in functions if h==crc('QuitGame'))
    confirm=bytearray(original_quit[1]);replaced=0
    for position,kind,_,_ in tokens(confirm):
        if kind==7 and struct.unpack_from('>I',confirm,position+4)[0]==0xd4abb75d:
            struct.pack_into('>I',confirm,position+4,crc('CosmicExitToLauncher'));replaced+=1
    if replaced!=1:raise ValueError('Unexpected original confirmation layout')
    changed.append((crc('CosmicQuitToLauncher'),original_quit[0],bytes(confirm)))
    for callback,native in [('CosmicExitToLauncher','cosmic_quit_to_launcher'),('CosmicVideoOptions','cosmic_video_options')]:
        words=[0x00000507,crc('overlay'),0x00000207,crc('overlay'),0x00000217,crc(native),0x01050020,1]
        if native=='cosmic_video_options':
            words.extend([0x00000507,crc('overlay'),0x00000607,crc('CosmicRestoreOptions'),0x00000207,crc('overlay'),0x00000217,crc('pushfunction'),0x02050020,1])
        words.append(0x21)
        changed.append((crc(callback),1,struct.pack('>'+len(words)*'I',*words)))
    restore=[0x00000507,crc('overlay'),0x00000207,crc('overlay'),0x00000217,crc('popfunction'),0x01050020,1,
             0x00000207,0x8610f32e,0x00000502,0x00050219,0x84fe6ee3,
             0x00000503,0,0x00050219,0xa393d2fb,
             0x00010502,0x00000207,crc('MenuStack_ShowCurrentMenu'),0x01050220,1,0x21]
    changed.append((crc('CosmicRestoreOptions'),1,struct.pack('>'+len(restore)*'I',*restore)))
    output=bytearray(struct.pack('>II',0x1badbabe,len(changed)))
    for identity,arguments,code in changed:
        output.extend(struct.pack('>III',identity,arguments,len(code)//4));output.extend(code)
    records(output)
    return bytes(output)

def main():
    root = Path(__file__).resolve().parents[1]
    source = root/'originals/global/scripts'
    script = (source/'frontend.ds').read_bytes()
    original_hash = hashlib.sha256(script).hexdigest()
    if original_hash != 'dfe41cb728d90d93fb685a5d19e1664bb08bbebc9056ef23e25e997a00eafabb':
        raise ValueError('Unsupported original frontend script')
    candidate = extend_script(script)
    text = (source/'xbx_us.txt').read_bytes()
    if hashlib.sha256(text).hexdigest() != 'fd1571d2b528a1710a73f5af4b0a8e916b6e0623db1916f615975bdc10e08365':
        raise ValueError('Unsupported original English localization')
    cursor = 4
    identities = set()
    while cursor < len(text):
        identity, size = struct.unpack_from('>II',text,cursor)
        cursor += 8+size
        identities.add(identity)
    if cursor != len(text) or crc('cosmic_quit_to_launcher_label') in identities:
        raise ValueError('Unexpected localization table')
    label = b'QUIT TO LAUNCHER\0'
    translated = text+struct.pack('>II',crc('cosmic_quit_to_launcher_label'),len(label))+label
    video=b'VIDEO\0'
    translated+=struct.pack('>II',crc('cosmic_video_label'),len(video))+video
    output = root/'data/analysis/pause-menu-candidate'
    output.mkdir(parents=True,exist_ok=True)
    (output/'frontend.ds').write_bytes(candidate)
    (output/'xbx_us.txt').write_bytes(translated)
    report = {'original_frontend_sha256':original_hash,'functions_preserved':len(records(script)),
              'original_options':5,'candidate_options':6,'new_label':'QUIT TO LAUNCHER','options_entry':'VIDEO','authored_callbacks':4,
              'original_quit_unchanged':True,'native_callback_registered':False,
              'gameplay_tested':False,'installed':False}
    (output/'validation.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps(report,indent=2))

if __name__ == '__main__':
    main()

"""Author an optional VM extension; transform only verified local game scripts.

The source contains our assembler and patch locations, not original game code.
Loading uses the original mission's loaddata/loaddesc and completion callbacks.
Private integration candidate: not a claim of gameplay compatibility.
"""
import hashlib
import json
from pathlib import Path
import struct
from prepare_pause_menu_extension import crc, records

ROOT=Path(__file__).resolve().parents[1]

class Code:
    def __init__(self):self.words=[];self.labels={};self.fixups=[]
    def emit(self,*words):self.words.extend(words);return self
    def label(self,name):
        if name in self.labels:raise ValueError('Duplicate label '+name)
        self.labels[name]=len(self.words)
    def get(self,r,name):self.emit(r<<8|7,crc(name) if isinstance(name,str) else name)
    def put(self,r,name):self.emit(r<<8|8,crc(name) if isinstance(name,str) else name)
    def num(self,r,n):self.emit(r<<8|3,struct.unpack('>I',struct.pack('>f',n))[0])
    def integer(self,r,n):self.emit(r<<8|5,n)
    def boolean(self,r,b):self.emit(int(b)<<16|r<<8|2)
    def nil(self,r):self.emit(r<<8|1)
    def table(self,r):self.emit(r<<8|0x16)
    def move(self,d,s):self.emit(d<<16|s<<8|6)
    def field(self,d,s,name):self.emit(d<<16|s<<8|0x17,crc(name) if isinstance(name,str) else name)
    def fieldput(self,s,name,v):self.emit(v<<16|s<<8|0x19,crc(name) if isinstance(name,str) else name)
    def index(self,d,a,k):self.emit(d<<24|k<<16|a<<8|0x18)
    def store(self,a,k,v):self.emit(v<<24|k<<16|a<<8|0x1a)
    def binary(self,op,d,l,r):self.emit(d<<24|r<<16|l<<8|op)
    def false(self,r,label):self.fixups.append((len(self.words),label,16));self.emit(r<<8|0x1f)
    def jump(self,label):self.fixups.append((len(self.words),label,8));self.emit(0x1d)
    def call(self,name,args=(),result=20):
        for i,r in enumerate(args):self.move(40+i,r)
        self.get(30,name);self.emit(len(args)<<24|40<<16|30<<8|0x20,result)
    def method(self,obj,name,args=(),result=20):
        for i,r in enumerate(args):self.move(40+i,r)
        self.field(30,obj,name);self.emit(len(args)<<24|40<<16|30<<8|0x20,result)
    def native(self,name,result=20):
        self.get(29,'overlay');self.method(29,name,[29],result)
    def done(self):self.emit(0x21)
    def finish(self):
        for position,label,shift in self.fixups:
            self.words[position]|=self.labels[label]<<shift
        return struct.pack('>'+str(len(self.words))+'I',*self.words)

BASE='cosmic_alien_base';EXTRA='cosmic_alien_extra';READY='cosmic_alien_ready'
PENDING='cosmic_alien_pending';PAGE='cosmic_alien_page';ACTIVE='cosmic_alien_active'

def authored(disable_rath=False):
    result=[]
    def save(name,args,c):c.done();result.append((crc(name),args,c.finish()))
    # Callback follows the original mission loader's completion convention.
    c=Code();c.get(1,PENDING);c.false(1,'end');c.num(2,0)
    c.binary(0x13,3,1,2);c.false(3,'end');c.num(2,1)
    c.binary(0x0b,1,1,2);c.put(1,PENDING);c.num(2,0)
    c.binary(0x10,3,1,2);c.false(3,'end');c.boolean(1,True);c.put(1,READY)
    c.label('end');save('CosmicAlienLoaded',1,c)

    c=Code();c.get(0,BASE);c.false(0,'end');c.get(1,'gFormHotKey')
    for i in range(1,5):
        c.num(2,i);c.index(3,0,2);c.index(4,1,2);c.fieldput(4,'formID',3)
    c.num(1,0);c.put(1,PAGE);c.label('end');save('CosmicAlienRestore',0,c)

    c=Code();c.get(0,'gFormHotKey');c.false(0,'end')
    if disable_rath:
        c.call(0xe1003dc4,result=12);c.field(12,12,'Rath')
    # Refuse an incomplete/duplicate preset rather than guessing its semantics.
    for i in range(4):
        c.num(1,i+1);c.index(2,0,1);c.field(4+i,2,'formID');c.false(4+i,'end')
        c.num(1,0);c.binary(0x13,2,4+i,1);c.false(2,'end')
        c.num(1,12);c.binary(0x12,2,4+i,1);c.false(2,'end')
        if disable_rath:c.binary(0x11,2,4+i,12);c.false(2,'end')
        for j in range(i):c.binary(0x11,2,4+i,4+j);c.false(2,'end')
    c.table(0);c.put(0,BASE)
    for i in range(4):c.num(1,i+1);c.store(0,1,4+i)
    c.table(0);c.put(0,EXTRA);c.num(8,1)
    for identity in range(1,12):
        c.num(1,identity)
        if disable_rath:c.binary(0x11,2,1,12);c.false(2,'skip'+str(identity))
        for i in range(4):c.binary(0x11,2,1,4+i);c.false(2,'skip'+str(identity))
        c.store(0,8,1);c.num(2,1);c.binary(0x0a,8,8,2);c.label('skip'+str(identity))
    c.num(1,12 if disable_rath else 14);c.put(1,PENDING);c.boolean(1,False);c.put(1,READY)
    c.boolean(1,True);c.put(1,ACTIVE);c.num(1,0);c.put(1,PAGE)
    c.get(1,0xf3d407b1);c.method(1,'getcurrentmission',[1],2)
    c.get(3,EXTRA);c.get(4,'gFormData');c.get(5,'CosmicAlienLoaded')
    for i in range(1,7 if disable_rath else 8):
        c.num(6,i);c.index(7,3,6);c.index(8,4,7)
        c.get(9,0x0bfbfd2d);c.field(10,8,'characterFile')
        c.method(2,0xbc35779c,[2,9,10,5])
        c.get(9,0x79c61e79);c.field(10,8,0x7fb535f8)
        c.method(2,0x1a7328ab,[2,9,10,5])
    c.label('end');save('CosmicAlienLoad',0,c)

    c=Code();c.get(0,ACTIVE);c.false(0,'end');c.call('CosmicAlienRestore')
    c.get(1,0xf3d407b1);c.method(1,'getcurrentmission',[1],2)
    c.get(3,EXTRA);c.get(4,'gFormData')
    for i in range(1,7 if disable_rath else 8):
        c.num(6,i);c.index(7,3,6);c.index(8,4,7)
        c.get(9,0x93aca275);c.field(10,8,'character')
        c.method(9,'removecharacter',[9,10])
        c.get(9,0x0bfbfd2d);c.field(10,8,'characterFile');c.boolean(11,True)
        c.method(2,'unloaddata',[2,9,10,11])
        c.get(9,0x79c61e79);c.field(10,8,0x7fb535f8)
        c.method(2,'unloaddesc',[2,9,10])
    for name in [BASE,EXTRA,READY,PENDING,PAGE,ACTIVE]:c.nil(1);c.put(1,name)
    c.label('end');save('CosmicAlienUnload',0,c)

    c=Code();c.native('cosmic_all_aliens_page',0)
    c.num(1,0);c.binary(0x12,2,0,1);c.false(2,'enabled')
    c.get(1,BASE);c.false(1,'end');c.call('CosmicAlienRestore');c.jump('end')
    c.label('enabled');c.get(1,ACTIVE);c.false(1,'load');c.jump('ready')
    c.label('load');c.call('CosmicAlienLoad');c.jump('end')
    c.label('ready');c.get(1,READY);c.false(1,'end');c.get(1,PAGE)
    c.binary(0x11,2,0,1);c.false(2,'end');c.put(0,PAGE)
    c.num(1,0);c.binary(0x10,2,0,1);c.false(2,'extras')
    c.call('CosmicAlienRestore');c.jump('sound')
    c.label('extras');c.get(1,EXTRA);c.get(2,'gFormHotKey')
    c.num(3,1);c.binary(0x0b,4,0,3);c.num(3,4);c.binary(0x0c,4,4,3)
    for i in range(1,5):
        c.num(3,i);c.binary(0x0a,5,4,3);c.index(6,1,5)
        c.index(7,2,3);c.fieldput(7,'formID',6)
    c.label('sound');c.get(1,0x2712f51f);c.emit(2<<8|5,0xf4b7b9fd)
    c.method(1,0x5e89deba,[1,2]);c.native('cosmic_all_aliens_trace')
    c.call('CosmicUltimateUpdate')
    c.label('end');save('CosmicAlienUpdate',0,c)

    c=Code();c.native('cosmic_ultimate_trigger',0)
    c.num(1,0);c.binary(0x13,2,0,1);c.false(2,'end')
    c.get(1,'gFormHotKey');c.get(2,'gFormData')
    pairs=[
        ('character_humongousaur','character_ulthumong'),
        ('character_swampfire','character_ultswampfire'),
        ('character_spidermonkey','character_ultspidermonkey'),
        ('character_bigchill','character_ultbigchill'),
        ('character_echo','character_ultecho'),
    ]
    for slot in range(1,5):
        next_slot=f'next_slot_{slot}'
        c.num(3,slot);c.index(4,1,3);c.field(5,4,'formID');c.false(5,next_slot)
        c.index(6,2,5);c.field(7,6,'character');c.false(7,next_slot)
        for base_name,ultimate_name in pairs:
            next_check=f'next_check_{slot}_{base_name}'
            c.integer(8,crc(base_name));c.binary(0x10,9,7,8);c.false(9,next_check)
            c.call('switch_to_ultimate');c.jump('end')
            c.label(next_check)
            c.integer(8,crc(ultimate_name));c.binary(0x10,9,7,8);c.false(9,f'next_pair_{slot}_{base_name}')
            c.call('undo_ultimate_form');c.jump('end')
            c.label(f'next_pair_{slot}_{base_name}')
        c.label(next_slot)
    c.label('end');save('CosmicUltimateUpdate',0,c)
    return result

def decode(code):
    words=list(struct.unpack('>'+str(len(code)//4)+'I',code));result=[];i=0
    while i<len(words):
        kind=words[i]&255;size=2 if kind in (3,5,7,8,0x17,0x19,0x20) else 1
        if kind==4:
            end=code.index(b'\0',i*4+4)+1;size=((end-i*4+3)//4)
        elif not 1<=kind<=0x22:raise ValueError(f'Unexpected opcode {kind:x} at {i}')
        if i+size>len(words):raise ValueError('Truncated VM token')
        result.append((i,kind));i+=size
    return words,result

def relocate(code,insertions):
    words,decoded=decode(code);boundaries={i for i,k in decoded}|{len(words)}
    if not set(insertions)<=boundaries:raise ValueError('Insertion inside VM token')
    def mapped(i):return i+sum(len(v) for k,v in insertions.items() if k<i)
    out=[]
    for i,kind in decoded:
        out.extend(insertions.get(i,[]));size=next((n-i for n,k in decoded if n>i),len(words)-i)
        value=words[i]
        if kind in (0x1d,0x1e,0x1f):
            shift=8 if kind==0x1d else 16;target=value>>shift
            if target not in boundaries:raise ValueError('Invalid original VM branch')
            value=(value&((1<<shift)-1))|(mapped(target)<<shift)
        out.extend([value,*words[i+1:i+size]])
    out.extend(insertions.get(len(words),[]))
    return struct.pack('>'+str(len(out))+'I',*out)

def encode(rows):
    out=bytearray(struct.pack('>II',0x1badbabe,len(rows)))
    for h,a,c in rows:out.extend(struct.pack('>III',h,a,len(c)//4));out.extend(c)
    records(out);return bytes(out)

def hook(name):return [0x00001b07,crc(name),0x00281b20,3]

def transform_globals(blob):
    rows=[]
    for h,a,c in records(blob):
        if h==crc('switch_form_update'):c=relocate(c,{34:hook('CosmicAlienUpdate')})
        if h==crc('destroy_form_switching'):c=relocate(c,{0:hook('CosmicAlienUnload')})
        if h==0xa67003d2:c=relocate(c,{0:hook('CosmicAlienRestore')})
        rows.append((h,a,c))
    return encode(rows+authored())

def main():
    source=ROOT/'originals/global/scripts';out=ROOT/'build/diagnostics/all-aliens-script-20261023'
    out.mkdir(parents=True,exist_ok=True)
    original=(source/'globals.ds').read_bytes();candidate=transform_globals(original)
    (out/'globals.ds').write_bytes(candidate)
    report={'original_sha256':hashlib.sha256(original).hexdigest(),
            'candidate_sha256':hashlib.sha256(candidate).hexdigest(),
            'authored_functions':len(authored()),'gameplay_validated':False,
            'installed':False,'release_eligible':False,
            'remaining':['Blank fourth slot HUD/input guards','Gameplay menu toggle','Resource lifetime gameplay test','Audible sound verification']}
    (out/'prepared.json').write_text(json.dumps(report,indent=2))
    # Only our authored functions are embedded; original game instructions are
    # transformed from the user's verified scripts at installation time.
    (ROOT/'launcher/AllAliensProgram.bin').write_bytes(encode(authored()))
    print(json.dumps(report,indent=2))

if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser()
    parser.add_argument('--program-only',action='store_true',help='Build only authored bytecode; no game files are read')
    arguments=parser.parse_args()
    if arguments.program_only:
        (ROOT/'launcher/AllAliensProgram.bin').write_bytes(encode(authored()))
        print('Authored All Aliens program generated without reading game data.')
    else:main()

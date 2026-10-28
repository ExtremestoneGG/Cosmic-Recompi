"""Expand both callback tables after local codegen; contains no game bodies."""
import re
from pathlib import Path
CAPACITY=64
RECORD_BASE=756
CONSUMED_BASE=RECORD_BASE+24*CAPACITY
OBJECT_BYTES=CONSUMED_BASE+4*CAPACITY
FIRST_BASE=2572
FIRST_PENDING=FIRST_BASE+24*CAPACITY
assert (RECORD_BASE-444)%24==0 and CONSUMED_BASE%4==0
spec={
 1:('8215B828',{444:756,452:764,449:761,460:772,464:776,19:32,440:752,684:CONSUMED_BASE,171:CONSUMED_BASE//4}),
 19:('8215D550',{444:756,448:760,449:761,452:764,460:772,464:776,19:32}),
 36:('8215ED50',{744:OBJECT_BYTES}),
 69:('8215AF68',{19:32,440:752,684:CONSUMED_BASE,171:CONSUMED_BASE//4}),
 75:('8215E208',{}),
 96:('8215E5B8',{444:756}),
}

def transform(text,function,mapping):
    start=text.index(f'DEFINE_REX_FUNC(sub_{function}) {{')
    end=text.find('\nDEFINE_REX_FUNC(',start+1)
    if end<0:end=len(text)
    body=text[start:end];counts={key:0 for key in mapping};lines=[]
    for line in body.splitlines(keepends=True):
        if not line.lstrip().startswith('//') and 'ctx.r1.' not in line:
            def replace(match):
                value=int(match[0]);counts[value]+=1;return str(mapping[value])
            if mapping:line=re.sub(r'\b(?:'+ '|'.join(map(str,mapping))+r')\b',replace,line)
        lines.append(line)
    modified=''.join(lines)
    assert all(counts.values()),(function,counts)
    if function=='8215AF68':
        needle='ctx.r30.s64 = ctx.r11.s64 + 448;'
        assert modified.count(needle)==1
        modified=modified.replace(needle,'ctx.r30.s64 = ctx.r11.s64 + 760;')
    if function=='8215E208':
        needle='REX_FUNC_PROLOGUE();';assert modified.count(needle)==1
        modified=modified.replace(needle,needle+f'\n\tstd::memset(base+ctx.r3.u32+744,0,{OBJECT_BYTES-744});',1)
    if function=='8215E5B8' and OBJECT_BYTES>3000:
        needle='ctx.r30.s64 = 10;';assert modified.count(needle)==1
        modified=modified.replace(needle,'ctx.r30.s64 = 64;')
    if mapping.get(76,76)>1000:
        if function=='8215D130':
            needle='ctx.r11.s64 = ctx.r11.s64 + 4;';assert modified.count(needle)==1
            modified=modified.replace(needle,'ctx.r11.s64 = ctx.r11.s64 + 108;')
        if function in ('8215AD38','8215E778'):
            needle='ctx.r11.s64 = ctx.r11.s64 + 72;'
            assert modified.count(needle)==(1 if function=='8215AD38' else 2)
            modified=modified.replace(needle,'ctx.r11.s64 = ctx.r11.s64 + 2568;')
    return text[:start]+modified+text[end:],counts


def apply(generated):
    global OBJECT_BYTES
    grouped={unit:[entry] for unit,entry in spec.items()}
    if True:
        OBJECT_BYTES=FIRST_PENDING+8*CAPACITY
        grouped[36]=[('8215ED50',{744:OBJECT_BYTES})]
        first={v:v+FIRST_BASE-76 for v in (76,80,81,82,83,84,88,92)}
        pending={316:FIRST_PENDING,320:FIRST_PENDING+4}
        grouped[19][0][1][84]=first[84]
        grouped[96][0][1][76]=FIRST_BASE
        grouped.update({
            6:[('8215AD38',{**first,**pending})],
            63:[('8215D130',{76:first[76],80:first[80],81:first[81],82:first[82],83:first[83],84:first[84],88:first[88],92:first[92],**pending})],
            82:[('8215ACD8',{76:first[76],80:first[80]})],
            86:[('8215D3D0',{81:first[81],316:FIRST_PENDING,40:(FIRST_PENDING+4)//8})],
            100:[('8215B7A8',{76:first[76],84:first[84]}),
                 ('8215E778',{**first,**pending,40:(FIRST_PENDING+4)//8})],
        })

    changed=0
    for unit,entries in grouped.items():
        path=Path(generated)/f'ben10_recomp.{unit}.cpp'
        text=path.read_text()
        for function,mapping in entries:
            marker='// Cosmic callback capacity 64: '+function
            if marker in text:continue
            text,_=transform(text,function,mapping)
            needle=f'DEFINE_REX_FUNC(sub_{function}) {{'
            text=text.replace(needle,needle+'\n\t'+marker,1)
            changed+=1
        if '#include <cstring>' not in text:text='#include <cstring>\n'+text
        path.write_text(text)
    return changed

"""Apply authored PC integration to locally generated game code, idempotently.

Contains patch locations and authored changes, never original function bodies.
Run after rexglue codegen and before compiling the consumer.
"""
import argparse
from pathlib import Path

parser=argparse.ArgumentParser();parser.add_argument('--generated',type=Path,required=True);args=parser.parse_args()
changes=0
for name in ['sub_821ADC40','sub_82473060']:
 matches=[]
 for p in args.generated.glob('*.cpp'):
  text=p.read_text()
  marker='DEFINE_REX_FUNC('+name+') {'
  if marker in text:matches.append((p,text,text.index(marker)))
 if len(matches)!=1:raise RuntimeError('Expected exactly one generated '+name)
 p,text,start=matches[0];end=text.find('\nDEFINE_REX_FUNC(',start+1)
 if end<0:end=len(text)
 body=text[start:end]
 if name=='sub_821ADC40':
  old='ctx.r3.s64 = 41943040;';new='ctx.r3.s64 = 100663296;'
  if new in body:continue
  if 'ctx.r3.s64 = 67108864;' in body:old='ctx.r3.s64 = 67108864;'
 else:
  old='__imp__XamInputGetState(ctx, base);'
  new='const uint32_t cosmic_state=ctx.r5.u32;\n\t'+old+'\n\tBen10FilterGameInput(ctx.r3.u32,cosmic_state,base);'
  if 'Ben10FilterGameInput(' in body:continue
 if body.count(old)!=1:raise RuntimeError('Unexpected generated body for '+name)
 body=body.replace(old,new);p.write_text(text[:start]+body+text[end:]);changes+=1

from apply_callback_capacity import apply as expand_callbacks
from apply_resource_diagnostics import apply as add_diagnostics
changes+=expand_callbacks(args.generated)
changes+=add_diagnostics(args.generated)
p=args.generated/'ben10_recomp.72.cpp';text=p.read_text()
start=text.index('DEFINE_REX_FUNC(sub_824868B8) {')
end=text.find('\nDEFINE_REX_FUNC(',start+1)
if end<0:end=len(text)
body=text[start:end]
if 'ben10_adaptive_spin::Poll' not in body:
    label='loc_824868D8:'
    assert body.count(label)==1
    if '__builtin_ia32_pause' not in body:
        body=body.replace(label,label+'\n\t__builtin_ia32_pause(); // Original loop and checks retained.')
    needle='ctx.r8.u64 = REX_LOAD_U32(ctx.r11.u32 + 0);'
    assert body.count(needle)==1
    body=body.replace(needle,needle+'\n\tben10_adaptive_spin::Poll(ctx.r31.u32,ctx.r8.u32);')
    text=text[:start]+body+text[end:]
    text='#include "../../src/adaptive_guest_spin.h"\n'+text
    p.write_text(text);changes+=1
print('PC integration applied to',changes,'locally generated functions.')

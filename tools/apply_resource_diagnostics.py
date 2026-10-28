"""Failure-only diagnostic insertions; no original function bodies."""
from pathlib import Path
ALLOCATION='\n    struct CosmicAllocationObserver {\n      decltype(ctx) context;\n      uint32_t caller, requested, argument3, argument4, argument5;\n      ~CosmicAllocationObserver() {\n        if (!context.r3.u32)\n          REXLOG_ERROR("Cosmic allocation failure: function=FUNCTION, caller={:08X}, bytes={:08X}, arg3={:08X}, arg4={:08X}, arg5={:08X}",\n            caller, requested, argument3, argument4, argument5);\n      }\n    } cosmic_allocation_observer{ctx, uint32_t(ctx.lr), ctx.SIZE.u32,\n                                ctx.r3.u32, ctx.r4.u32, ctx.r5.u32};\n'
CALLBACK='\n    if(ctx.r3.u32>=0x30000000 && ctx.r3.u32<0x70000000) {\n      REXLOG_ERROR("Cosmic boss callback invalid value: object={:08X}, slot={:08X}, value={:08X}, key={:08X}, header={:08X}, callback={:08X}, userdata={:08X}",\n        ctx.r31.u32,ctx.r30.u32,ctx.r3.u32,\n        REX_LOAD_U32(ctx.r30.u32-4),REX_LOAD_U32(ctx.r30.u32),\n        REX_LOAD_U32(ctx.r30.u32+12),REX_LOAD_U32(ctx.r30.u32+16));\n    }\n'
def apply(generated):
    changed=0
    for unit,function,register in [(49,'sub_82164248','r3'),(42,'sub_82163ED8','r4')]:
        p=Path(generated)/f'ben10_recomp.{unit}.cpp';text=p.read_text()
        if 'CosmicAllocationObserver' in text:continue
        needle=f'DEFINE_REX_FUNC({function}) {{\n\tREX_FUNC_PROLOGUE();'
        assert text.count(needle)==1
        text=text.replace(needle,needle+ALLOCATION.replace('FUNCTION',function).replace('SIZE',register))
        p.write_text(text);changed+=1
    p=Path(generated)/'ben10_recomp.69.cpp';text=p.read_text()
    if 'Cosmic boss callback invalid value:' not in text:
        needle='ctx.lr = 0x8215B1E4;\n\tsub_822368A8(ctx, base);'
        assert text.count(needle)==1
        text=text.replace(needle,'ctx.lr = 0x8215B1E4;\n'+CALLBACK+'\tsub_822368A8(ctx, base);')
        p.write_text(text);changed+=1
    return changed

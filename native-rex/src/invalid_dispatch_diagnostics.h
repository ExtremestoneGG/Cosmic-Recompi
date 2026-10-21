#pragma once
// Only runs after an invalid guest dispatch. Never suppresses or skips the call.
#include <array>
#include <cstdint>
#include <cstdio>
#if defined(_WIN32)
#include <Windows.h>
#endif

namespace ben10_diagnostics {
template<class Context,class Report>
void InvalidDispatch(const Context& ctx,const uint8_t* base,Report&& report) {
  char text[512];
  std::snprintf(text,sizeof(text),
      "Invalid dispatch context: target=%08X LR=%016llX CTR=%016llX SP=%08X r3=%016llX r4=%016llX r11=%016llX r12=%016llX",
      unsigned(ctx.last_indirect_target),static_cast<unsigned long long>(ctx.lr),
      static_cast<unsigned long long>(ctx.ctr.u64),unsigned(ctx.r1.u32),
      static_cast<unsigned long long>(ctx.r3.u64),static_cast<unsigned long long>(ctx.r4.u64),
      static_cast<unsigned long long>(ctx.r11.u64),static_cast<unsigned long long>(ctx.r12.u64));
  report(text);
#if defined(_WIN32)
  std::array<uint32_t,64> stack{};SIZE_T read=0;
  if(base && ctx.r1.u32<=UINT32_MAX-sizeof(stack) &&
     ReadProcessMemory(GetCurrentProcess(),base+ctx.r1.u32,stack.data(),sizeof(stack),&read)) {
    for(size_t i=0;i<read/sizeof(uint32_t);++i) {
      uint32_t word=stack[i];
      word=(word>>24)|((word>>8)&0xff00)|((word<<8)&0xff0000)|(word<<24);
      if(word>=0x82000000 && word<0x82700000 && !(word&3)) {
        std::snprintf(text,sizeof(text),"Invalid dispatch guest stack +%03X: %08X",unsigned(i*4),unsigned(word));
        report(text);
      }
    }
  } else report("Invalid dispatch guest stack unavailable; memory was not dereferenced.");
  void* frames[32]{};
  auto count=CaptureStackBackTrace(0,32,frames,nullptr);
  for(unsigned i=0;i<count;++i) {
    std::snprintf(text,sizeof(text),"Invalid dispatch native frame %u: %p (executable base %p)",i,frames[i],GetModuleHandleW(nullptr));
    report(text);
  }
#endif
}
}

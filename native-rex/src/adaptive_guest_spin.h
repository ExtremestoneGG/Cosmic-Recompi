#pragma once
// Bounded CPU polling: bounded processor hints after unchanged GPU progress.
// No OS yield, sleep, skipped guest check, altered deadline or memory write.
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
namespace ben10_adaptive_spin {
struct State {
  uint32_t object=0,progress=0,repeated=0;
  unsigned Select(uint32_t current_object,uint32_t current_progress) {
    if(!current_object || object!=current_object || progress!=current_progress) {
      object=current_object;progress=current_progress;repeated=0;return 0;
    }
    if(repeated<64)++repeated;
    return repeated>=64 ? 8 : 0;
  }
};
inline bool Enabled(){
  static const bool enabled=[] {
    const char* value=std::getenv("BEN10_ADAPTIVE_GUEST_SPIN");
    const bool active=!value || std::strcmp(value,"0")!=0;
    if(active)std::fputs("Cosmic adaptive guest spin active\n",stderr);
    return active;
  }();
  return enabled;
}
inline void Poll(uint32_t object,uint32_t progress) {
  if(!Enabled())return;
  thread_local State state;
  const unsigned hints=state.Select(object,progress);
  for(unsigned i=0;i<hints;++i)__builtin_ia32_pause();
}
}

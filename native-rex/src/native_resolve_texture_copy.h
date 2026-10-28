#pragma once
#include "rgba_host_representation.h"
#include <cstdlib>
#include <cstring>
namespace ben10_rgba {
inline bool NativeCopyEnabled(){
  static const bool enabled=[](){const char* v=std::getenv("REX_NATIVE_RESOLVE_TEXTURE_COPY");return v && !std::strcmp(v,"true");}();
  return enabled;
}
}

#pragma once
// Opt-in CPU metadata counters; no resource state, shader or load is changed.
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <map>
namespace ben10_profile {
struct TextureLoadAudit {
  const char* output=std::getenv("BEN10_TEXTURE_LOAD_AUDIT");
  using Key=std::array<uint32_t,18>;
  std::map<Key,uint64_t> counts;
  std::chrono::steady_clock::time_point previous=std::chrono::steady_clock::now();
  void Record(const Key& key) {
    if(!output)return;
    ++counts[key];
    const auto now=std::chrono::steady_clock::now();
    if(now-previous<std::chrono::seconds(5))return;
    static constexpr const char* names[]={"shader","format","width","height","dimension","depth_or_array",
      "tiled","endianness","mip_max","load_base","load_mips","row_pitch_bytes","base_page","mip_page",
      "packed_mips","scaled","base_bytes","mip_bytes"};
    auto unix_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream log(output,std::ios::app);
    log<<"{\"time_ms\":"<<unix_ms<<",\"interval_seconds\":"
       <<std::chrono::duration<double>(now-previous).count()<<",\"loads\":[";
    bool first=true;
    for(const auto& pair:counts) {
      log<<(first?"":",")<<"{\"count\":"<<pair.second;
      for(size_t i=0;i<pair.first.size();++i)log<<",\""<<names[i]<<"\":"<<pair.first[i];
      log<<"}";first=false;
    }
    log<<"]}\n";counts.clear();previous=now;
  }
};
inline thread_local TextureLoadAudit texture_load_audit;
}

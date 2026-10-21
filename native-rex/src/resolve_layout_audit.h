#pragma once
// Opt-in CPU metadata counters; no resource state, shader or load is changed.
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <map>
namespace ben10_profile {
struct ResolveLayoutAudit {
  const char* output=std::getenv("BEN10_RESOLVE_LAYOUT_AUDIT");
  using Key=std::array<uint32_t,23>;
  std::map<Key,uint64_t> counts;
  std::chrono::steady_clock::time_point previous=std::chrono::steady_clock::now();
  void Record(const Key& key) {
    if(!output)return;
    ++counts[key];
    const auto now=std::chrono::steady_clock::now();
    if(now-previous<std::chrono::seconds(5))return;
    static constexpr const char* names[]={"shader","width","height","source_depth","source_format","source_msaa","source_pitch_tiles","source_base_tiles","dest_info_packed","dest_coordinates_packed","source_coordinates_packed","dest_base","dest_extent_start","dest_extent_bytes","scaled","owner_rectangles","owner_key","owner_first_row","owner_rows","owner_first_start","owner_last_end","groups_x","groups_y"};
    auto unix_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream log(output,std::ios::app);
    log<<"{\"time_ms\":"<<unix_ms<<",\"interval_seconds\":"
       <<std::chrono::duration<double>(now-previous).count()<<",\"resolves\":[";
    bool first=true;
    for(const auto& pair:counts) {
      log<<(first?"":",")<<"{\"count\":"<<pair.second;
      for(size_t i=0;i<pair.first.size();++i)log<<",\""<<names[i]<<"\":"<<pair.first[i];
      log<<"}";first=false;
    }
    log<<"]}\n";counts.clear();previous=now;
  }
};
inline thread_local ResolveLayoutAudit resolve_layout_audit;
}

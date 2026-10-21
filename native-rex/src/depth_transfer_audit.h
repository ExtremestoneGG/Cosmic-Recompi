#pragma once
// Opt-in CPU metadata counters; no resource state, shader or load is changed.
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <map>
namespace ben10_profile {
struct DepthTransferAudit {
  const char* output=std::getenv("BEN10_DEPTH_TRANSFER_AUDIT");
  using Key=std::array<uint32_t,15>;
  std::map<Key,uint64_t> counts;
  std::chrono::steady_clock::time_point previous=std::chrono::steady_clock::now();
  void Record(const Key& key) {
    if(!output)return;
    ++counts[key];
    const auto now=std::chrono::steady_clock::now();
    if(now-previous<std::chrono::seconds(5))return;
    static constexpr const char* names[]={"dest_key","source_key","host_depth_key","depth_control","front_stencil_mask","back_stencil_mask","depth_may_write","stencil_may_write","readonly","transfer_count","start_tiles","end_tiles","dest_range_tiles","estimated_height","used_rt_bits"};
    auto unix_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream log(output,std::ios::app);
    log<<"{\"time_ms\":"<<unix_ms<<",\"interval_seconds\":"
       <<std::chrono::duration<double>(now-previous).count()<<",\"transfers\":[";
    bool first=true;
    for(const auto& pair:counts) {
      log<<(first?"":",")<<"{\"count\":"<<pair.second;
      for(size_t i=0;i<pair.first.size();++i)log<<",\""<<names[i]<<"\":"<<pair.first[i];
      log<<"}";first=false;
    }
    log<<"]}\n";counts.clear();previous=now;
  }
};
inline thread_local DepthTransferAudit depth_transfer_audit;
}

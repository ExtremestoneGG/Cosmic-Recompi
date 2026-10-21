#pragma once
// Observe guest depth clears; no resource or clear value is changed.
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <map>
#include <utility>
#include <array>
namespace ben10_profile {
struct DepthClearAudit {
  const char* output=std::getenv("BEN10_PROFILE_GPU_BARRIERS")?
      std::getenv("BEN10_D3D12_WORK_PROFILE"):nullptr;
  std::map<std::array<uint32_t,3>,uint64_t> counts;
  std::chrono::steady_clock::time_point previous=std::chrono::steady_clock::now();
  void Record(uint32_t format,uint32_t guest_depth,uint32_t stencil) {
    if(!output)return;
    ++counts[{format,guest_depth,stencil}];
    const auto now=std::chrono::steady_clock::now();
    if(now-previous<std::chrono::seconds(5))return;
    const auto wall=std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream log(output,std::ios::app);
    log<<"{\"depth_clear_audit\":true,\"time_ms\":"<<wall
       <<",\"interval_seconds\":"<<std::chrono::duration<double>(now-previous).count()
       <<",\"clears\":[";
    bool first=true;
    for(const auto& item:counts){
      log<<(first?"":",")<<"{\"format\":"<<item.first[0]
         <<",\"guest_depth\":"<<item.first[1]<<",\"stencil\":"<<item.first[2]<<",\"count\":"<<item.second<<"}";
      first=false;
    }
    log<<"]}\n";counts.clear();previous=now;
  }
};
inline thread_local DepthClearAudit depth_clear_audit;
}

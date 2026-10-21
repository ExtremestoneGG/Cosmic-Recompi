#pragma once
// Read-only opportunity counts. Never replaces or removes a recorded barrier.
#include <d3d12.h>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <cstdint>
namespace ben10_profile {
struct BarrierBatchCounts { uint64_t input=0,remaining=0,chains=0,cycles=0; };
inline BarrierBatchCounts AnalyzeBarrierBatch(const std::vector<D3D12_RESOURCE_BARRIER>& original) {
  BarrierBatchCounts result;result.input=original.size();
  std::vector<D3D12_RESOURCE_BARRIER> model;
  for(const auto& barrier:original) {
    bool merged=false;
    if(barrier.Type==D3D12_RESOURCE_BARRIER_TYPE_TRANSITION &&
       barrier.Flags==D3D12_RESOURCE_BARRIER_FLAG_NONE) {
      for(size_t index=model.size();index;) {
        --index;auto& previous=model[index];
        if(previous.Type!=D3D12_RESOURCE_BARRIER_TYPE_TRANSITION ||
           previous.Flags!=D3D12_RESOURCE_BARRIER_FLAG_NONE)break;
        if(previous.Transition.pResource!=barrier.Transition.pResource)continue;
        if(previous.Transition.Subresource!=barrier.Transition.Subresource) {
          if(previous.Transition.Subresource==D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES ||
             barrier.Transition.Subresource==D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)break;
          continue;
        }
        if(previous.Transition.StateAfter!=barrier.Transition.StateBefore)break;
        ++result.chains;merged=true;
        if(previous.Transition.StateBefore==barrier.Transition.StateAfter) {
          ++result.cycles;model.erase(model.begin()+index);
        } else previous.Transition.StateAfter=barrier.Transition.StateAfter;
        break;
      }
    }
    if(!merged)model.push_back(barrier);
  }
  result.remaining=model.size();return result;
}
struct BarrierBatchAudit {
  const char* output=std::getenv("BEN10_PROFILE_GPU_BARRIERS")?
      std::getenv("BEN10_D3D12_WORK_PROFILE"):nullptr;
  uint64_t packets=0,input=0,remaining=0,chains=0,cycles=0;
  std::chrono::steady_clock::time_point previous=std::chrono::steady_clock::now();
  void Record(const std::vector<D3D12_RESOURCE_BARRIER>& batch) {
    if(!output)return;
    const auto result=AnalyzeBarrierBatch(batch);
    ++packets;input+=result.input;remaining+=result.remaining;
    chains+=result.chains;cycles+=result.cycles;
    const auto now=std::chrono::steady_clock::now();
    if(now-previous<std::chrono::seconds(5))return;
    const auto wall=std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream log(output,std::ios::app);
    log<<"{\"barrier_batch_audit\":true,\"time_ms\":"<<wall
       <<",\"interval_seconds\":"<<std::chrono::duration<double>(now-previous).count()
       <<",\"packets\":"<<packets<<",\"input\":"<<input
       <<",\"model_remaining\":"<<remaining<<",\"chains\":"<<chains
       <<",\"cycles\":"<<cycles<<"}\n";
    packets=input=remaining=chains=cycles=0;previous=now;
  }
};
inline thread_local BarrierBatchAudit barrier_batch_audit;
}

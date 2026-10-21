#pragma once
// Opt-in host presentation telemetry. No screenshots, waits, or render-state changes.
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <dxgi.h>
namespace ben10_presentation_audit {
struct Audit {
  const char* output=std::getenv("BEN10_PRESENTATION_AUDIT");
  using Clock=std::chrono::steady_clock;
  Clock::time_point start=Clock::now();
  struct Record {
    int64_t begin_us=0,end_us=0;
    uint64_t guest_submission=0;
    uint32_t sync_interval=0,flags=0,last_present_count=0;
    int32_t present_result=0,statistics_result=0;
    DXGI_FRAME_STATISTICS statistics{};
  };
  std::vector<Record> records;
  bool written=false;
  void Flush() {
    if(!output || written || records.empty())return;
    written=true;
    std::ofstream file(output);
    for(const auto& r:records)file<<"{\"begin_us\":"<<r.begin_us<<",\"end_us\":"<<r.end_us
      <<",\"guest_submission\":"<<r.guest_submission<<",\"sync_interval\":"<<r.sync_interval
      <<",\"flags\":"<<r.flags<<",\"present_result\":"<<r.present_result
      <<",\"last_present_count\":"<<r.last_present_count
      <<",\"statistics_result\":"<<r.statistics_result<<",\"present_count\":"<<r.statistics.PresentCount
      <<",\"present_refresh_count\":"<<r.statistics.PresentRefreshCount
      <<",\"sync_refresh_count\":"<<r.statistics.SyncRefreshCount
      <<",\"sync_qpc_time\":"<<r.statistics.SyncQPCTime.QuadPart<<"}\n";
    file.flush();
  }
  void Observe(Clock::time_point begin,Clock::time_point end,IDXGISwapChain* chain,
               uint64_t guest_submission,uint32_t interval,uint32_t flags,HRESULT result) {
    if(!output || written)return;
    auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(begin-start).count();
    if(elapsed<185000000)return;
    if(elapsed>=225000000 || records.size()>=8192){Flush();return;}
    Record r{};r.begin_us=elapsed;
    r.end_us=std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
    r.guest_submission=guest_submission;r.sync_interval=interval;r.flags=flags;r.present_result=result;
    chain->GetLastPresentCount(&r.last_present_count);
    r.statistics_result=chain->GetFrameStatistics(&r.statistics);
    records.push_back(r);
  }
};
inline Audit audit;
}

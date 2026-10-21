#pragma once
// Observe the existing worker, including Sleep(1), without changing scheduling.
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <vector>
namespace ben10_profile {
struct VblankAudit {
  using Clock=std::chrono::steady_clock;
  const char* output=std::getenv("BEN10_VBLANK_AUDIT");
  Clock::time_point previous=Clock::now(),last_mark{};
  std::vector<double> sleeps,intervals,marks;
  uint64_t multi_batches=0,iterations=0;
  double refresh;
  explicit VblankAudit(double hz):refresh(hz) {}
  void Mark(Clock::time_point begin,Clock::time_point end) {
    if(last_mark!=Clock::time_point{}) intervals.push_back(std::chrono::duration<double,std::milli>(begin-last_mark).count());
    last_mark=begin;
    marks.push_back(std::chrono::duration<double,std::milli>(end-begin).count());
  }
  static void Stats(std::ofstream& log,const char* name,std::vector<double>& values) {
    std::sort(values.begin(),values.end());double sum=0;for(double v:values)sum+=v;
    const size_t n=values.size();
    log<<",\""<<name<<"\":{\"count\":"<<n<<",\"mean_ms\":"<<(n?sum/n:0)
       <<",\"p50_ms\":"<<(n?values[(n-1)/2]:0)
       <<",\"p95_ms\":"<<(n?values[size_t((n-1)*.95)]:0)
       <<",\"max_ms\":"<<(n?values.back():0)<<"}";
    values.clear();
  }
  void Sleep(Clock::time_point begin,Clock::time_point end,uint32_t batch) {
    sleeps.push_back(std::chrono::duration<double,std::milli>(end-begin).count());
    ++iterations;if(batch>1)++multi_batches;
    if(end-previous<std::chrono::seconds(5))return;
    auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream log(output,std::ios::app);
    log<<"{\"time_ms\":"<<now<<",\"interval_seconds\":"<<std::chrono::duration<double>(end-previous).count()
       <<",\"requested_sleep_ms\":1,\"refresh_hz\":"<<refresh
       <<",\"iterations\":"<<iterations<<",\"multi_vblank_batches\":"<<multi_batches;
    Stats(log,"sleep",sleeps);Stats(log,"vblank_interval",intervals);Stats(log,"mark_cost",marks);
    log<<"}\n";iterations=multi_batches=0;previous=end;
  }
};
}

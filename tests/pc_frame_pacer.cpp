#include "frame_pacing_probe.h"
#include <cstdio>

int main() {
  ben10_pacing_probe::Pacer pacer;
  auto measure=[&](unsigned rate,int intervals) {
    if(!pacer.SetRate(rate))return -1.0;
    pacer.Wait();const auto start=std::chrono::steady_clock::now();
    for(int i=0;i<intervals;i++)pacer.Wait();
    return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/intervals;
  };
  const double thirty=measure(30,15),sixty=measure(60,30);
  const bool invalid_rejected=!pacer.SetRate(120) && !pacer.SetRate(0) && pacer.rate()==60;
  const bool passed=thirty>=32 && thirty<45 && sixty>=16 && sixty<23 && thirty/sixty>1.6 && invalid_rejected;
  std::printf("{\"passed\":%s,\"30_hz_interval_ms\":%.4f,\"60_hz_interval_ms\":%.4f,\"uncapped_rejected\":%s,\"gameplay_performance_claim\":false}\n",passed?"true":"false",thirty,sixty,invalid_rejected?"true":"false");
  return passed?0:1;
}

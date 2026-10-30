#pragma once
// Opt-in attribution at the game's existing VdSwap hook; no scheduling changes.
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace ben10_cadence {
using Clock=std::chrono::steady_clock;
struct Audit {
  const char* output=std::getenv("BEN10_FRAME_CADENCE_LOG");
  Clock::time_point window=Clock::now(),previous_entry{},previous_exit{};
  uint64_t frames=0;
  double menu_ms=0,limiter_ms=0,outside_hook_ms=0,entry_intervals_ms=0;
  std::array<uint64_t,3> rates{};
  std::array<uint64_t,9> nearest_60hz_ticks{};
  double near_tick_intervals=0;
  static double Ms(Clock::duration d){return std::chrono::duration<double,std::milli>(d).count();}
  void Record(Clock::time_point entry,Clock::time_point before_wait,Clock::time_point exit,unsigned rate) {
    if(!output)return;
    menu_ms+=Ms(before_wait-entry);limiter_ms+=Ms(exit-before_wait);
    if(previous_exit!=Clock::time_point{})outside_hook_ms+=Ms(entry-previous_exit);
    if(previous_entry!=Clock::time_point{}) {
      const double interval=Ms(entry-previous_entry);
      entry_intervals_ms+=interval;
      const auto ticks=static_cast<unsigned>(std::max(0.0,interval/(1000.0/60.0)+0.5));
      if(ticks<nearest_60hz_ticks.size()) {
        ++nearest_60hz_ticks[ticks];
        if(std::abs(interval-ticks*(1000.0/60.0))<1.0)++near_tick_intervals;
      }
    }
    previous_entry=entry;previous_exit=exit;++frames;
    ++rates[rate==30?1:rate==60?2:0];
    if(exit-window<std::chrono::seconds(5))return;
    const auto unix_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream file(output,std::ios::app);
    file<<"{\"time_ms\":"<<unix_ms<<",\"seconds\":"<<std::chrono::duration<double>(exit-window).count()
        <<",\"frames\":"<<frames<<",\"registration_wall_ms\":"<<menu_ms
        <<",\"limiter_wall_ms\":"<<limiter_ms<<",\"outside_hook_wall_ms\":"<<outside_hook_ms
        <<",\"entry_intervals_ms\":"<<entry_intervals_ms<<",\"rate_frames_0_30_60\":["<<rates[0]<<','<<rates[1]<<','<<rates[2]
        <<"],\"intervals_within_1ms_of_60hz_multiple\":"<<near_tick_intervals<<",\"nearest_60hz_tick_histogram\":[";
    for(size_t i=0;i<nearest_60hz_ticks.size();++i){if(i)file<<',';file<<nearest_60hz_ticks[i];}
    file<<"],\"scope\":\"Hook wall time, not thread CPU. Outside hook includes game, graphics backpressure, guest sync and scheduling. Tick histogram is correlation, not proof of vsync.\"}\n";
    window=exit;frames=0;menu_ms=limiter_ms=outside_hook_ms=entry_intervals_ms=near_tick_intervals=0;
    rates.fill(0);nearest_60hz_ticks.fill(0);
  }
};
inline thread_local Audit audit;
}

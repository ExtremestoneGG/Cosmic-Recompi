#pragma once
// Playback cadence is enabled by the normal launcher policy; FPS probes leave it off.
#include <Windows.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <mutex>

namespace ben10_pacing_probe {
class Pacer {
 public:
  using Clock=std::chrono::steady_clock;
  Pacer() {
    const char* value=std::getenv("BEN10_FRAME_PACING_HZ");
    enabled_=value && std::strcmp(value,"30")==0;
    if(!enabled_)return;
    timer_=CreateWaitableTimerExW(nullptr,nullptr,0x00000002,TIMER_MODIFY_STATE|SYNCHRONIZE);
    if(!timer_)timer_=CreateWaitableTimerExW(nullptr,nullptr,0,TIMER_MODIFY_STATE|SYNCHRONIZE);
    enabled_=timer_!=nullptr;
  }
  ~Pacer(){if(timer_)CloseHandle(timer_);}
  void Wait() {
    if(!enabled_)return;
    std::lock_guard guard(mutex_);
    if(started_) {
      for(;;) {
        auto remaining=std::chrono::duration_cast<std::chrono::nanoseconds>(next_-Clock::now()).count();
        if(remaining<=0)break;
        LARGE_INTEGER due{};due.QuadPart=-((remaining+99)/100);
        if(!SetWaitableTimer(timer_,&due,0,nullptr,nullptr,FALSE))break;
        if(WaitForSingleObject(timer_,INFINITE)!=WAIT_OBJECT_0)break;
      }
    }
    started_=true;
    const auto now=Clock::now();
    const auto period=std::chrono::nanoseconds(33333333);
    // Preserve cadence across ordinary timer wake-up error, without a burst
    // after a genuinely missed frame deadline.
    if(next_==Clock::time_point{} || now-next_>std::chrono::milliseconds(2))next_=now+period;
    else next_+=period;
  }
 private:
  bool enabled_=false,started_=false;
  HANDLE timer_=nullptr;
  std::mutex mutex_;
  Clock::time_point next_{};
};
inline Pacer pacer;
}

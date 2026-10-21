#pragma once
// Playback cadence is enabled by the normal launcher policy; FPS probes leave it off.
#include <Windows.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <atomic>

namespace ben10_pacing_probe {
class Pacer {
 public:
  using Clock=std::chrono::steady_clock;
  Pacer() {
    const char* value=std::getenv("BEN10_FRAME_PACING_HZ");
    if(value && std::strcmp(value,"30")==0)SetRate(30);
    else if(value && std::strcmp(value,"60")==0)SetRate(60);
  }
  ~Pacer(){if(timer_)CloseHandle(timer_);}
  bool SetRate(unsigned rate) {
    if(rate!=30 && rate!=60)return false;
    std::lock_guard guard(mutex_);
    if(!timer_)timer_=CreateWaitableTimerExW(nullptr,nullptr,0x00000002,TIMER_MODIFY_STATE|SYNCHRONIZE);
    if(!timer_)timer_=CreateWaitableTimerExW(nullptr,nullptr,0,TIMER_MODIFY_STATE|SYNCHRONIZE);
    if(!timer_)return false;
    started_=false;next_={};rate_.store(rate,std::memory_order_relaxed);return true;
  }
  unsigned rate() const {return rate_.load(std::memory_order_relaxed);}
  void Wait() {
    if(!rate_.load(std::memory_order_relaxed))return;
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
    const auto period=std::chrono::nanoseconds(1000000000/rate_.load(std::memory_order_relaxed));
    // Preserve cadence across ordinary timer wake-up error, without a burst
    // after a genuinely missed frame deadline.
    if(next_==Clock::time_point{} || now-next_>std::chrono::milliseconds(2))next_=now+period;
    else next_+=period;
  }
 private:
  bool started_=false;
  std::atomic<unsigned> rate_{0};
  HANDLE timer_=nullptr;
  std::mutex mutex_;
  Clock::time_point next_{};
};
inline Pacer pacer;
}

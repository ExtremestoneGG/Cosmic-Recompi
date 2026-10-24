#include "frame_metrics.h"
#include "frame_pacing_probe.h"
#include "pc_menu_bridge.h"
#include <atomic>
#include <algorithm>
namespace {
std::atomic<uint64_t> submitted_frames{0};
std::mutex frame_mutex;
std::array<float,240> history{};
size_t next=0,count=0;
using Clock=std::chrono::steady_clock;
Clock::time_point previous{};
}
extern "C" void Ben10FilterGameInput(uint32_t status,uint32_t guest_state,uint8_t* base) {
  // X_INPUT_STATE: packet[4], buttons[2], LT[1], RT[1]. Keep the SDK's
  // raw controller state intact: the page selector must still see RT edges.
  if(status==0 && guest_state && ben10_pc_menu::all_aliens_enabled &&
     base[guest_state+6]>30)base[guest_state+7]=0;
}
extern "C" void Ben10FrameSubmitted() {
  ben10_pc_menu::OnFrame();
  ben10_pacing_probe::pacer.Wait();
  const auto now=Clock::now();
  std::lock_guard guard(frame_mutex);
  if(previous!=Clock::time_point{}) {
    history[next]=std::chrono::duration<float,std::milli>(now-previous).count();
    next=(next+1)%history.size();
    count=std::min(count+1,history.size());
  }
  previous=now;
  submitted_frames.fetch_add(1,std::memory_order_relaxed);
}
Ben10FrameStats Ben10ReadFrameStats() {
  Ben10FrameStats result;
  Clock::time_point latest;
  {
    std::lock_guard guard(frame_mutex);
    result.count=count;latest=previous;
    result.submitted_frames=submitted_frames.load(std::memory_order_relaxed);
    const size_t begin=(next+history.size()-count)%history.size();
    for(size_t i=0;i<count;++i)result.frame_times_ms[i]=history[(begin+i)%history.size()];
  }
  if(!result.count)return result;
  result.latest_ms=result.frame_times_ms[result.count-1];
  double duration=0;size_t samples=0;
  for(size_t i=result.count;i>0 && duration<1000;--i) {
    duration+=result.frame_times_ms[i-1];++samples;
  }
  if(duration>0 && Clock::now()-latest<std::chrono::seconds(1))result.fps=samples*1000.0/duration;
  auto sorted=result.frame_times_ms;
  std::sort(sorted.begin(),sorted.begin()+result.count);
  result.p95_ms=sorted[(result.count-1)*95/100];
  return result;
}
uint64_t Ben10SubmittedFrames() {
  return submitted_frames.load(std::memory_order_relaxed);
}

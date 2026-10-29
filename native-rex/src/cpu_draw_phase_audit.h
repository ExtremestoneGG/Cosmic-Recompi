#pragma once
// Opt-in CPU wall-time attribution only. No GPU queries or rendering changes.
#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
namespace ben10_cpu_draw {
using Clock=std::chrono::steady_clock;
enum Stage {Setup,Analysis,Primitives,Targets,Pipeline,Textures,FixedState,Bindings,VertexRanges,Memexport,Tail,Count};
inline constexpr const char* names[Count]={"setup","shader_analysis","primitives","render_targets","pipeline","textures","fixed_state","bindings","vertex_ranges","memexport","draw_tail"};
struct Totals {
  const char* output=std::getenv("BEN10_CPU_DRAW_PHASES");
  std::array<double,Count> milliseconds{};
  unsigned long long draws=0;
  Clock::time_point start=Clock::now();
  void Flush(Clock::time_point now) {
    if(!output || now-start<std::chrono::seconds(5))return;
    const auto unix_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream file(output,std::ios::app);
    file<<"{\"time_ms\":"<<unix_ms<<",\"seconds\":"<<std::chrono::duration<double>(now-start).count()<<",\"draws\":"<<draws<<",\"stage_wall_ms\":{";
    for(unsigned i=0;i<Count;++i){if(i)file<<',';file<<'\"'<<names[i]<<"\":"<<milliseconds[i];}
    file<<"},\"scope\":\"Sequential CPU draw phases, wall time including driver calls; not per-stage thread CPU or GPU time. Instrumentation overhead possible.\"}\n";
    milliseconds.fill(0);draws=0;start=now;
  }
};
inline thread_local Totals totals;
struct Cursor {
  bool enabled=totals.output!=nullptr;
  Stage stage=Setup;
  Clock::time_point previous{};
  Cursor(){if(enabled)previous=Clock::now();}
  void Mark(Stage next){if(!enabled)return;const auto now=Clock::now();totals.milliseconds[stage]+=std::chrono::duration<double,std::milli>(now-previous).count();previous=now;stage=next;}
  ~Cursor(){if(!enabled)return;Mark(stage);++totals.draws;totals.Flush(previous);}
};
}

#pragma once
// Opt-in CPU state audit. No rendering changes and no additional GPU queries.
#include <array>
#include <cstdlib>
#include <fstream>
#include <set>
#include <cstdint>

namespace ben10_profile {
struct RtvStencilAudit {
  const char* output=std::getenv("BEN10_RTV_STENCIL_AUDIT");
  uint64_t draws=0,two_sided=0,conflicts=0,last_report_frame=UINT64_MAX;
  std::set<std::array<uint64_t,6>> states;
  void Observe(uint64_t frame,bool eligible,uint32_t depth,uint32_t front,
               uint32_t back,uint32_t cull,uint64_t vs,uint64_t ps,uint32_t count) {
    if (!output) return;
    ++draws;
    front&=0x00FFFFFF;back&=0x00FFFFFF;
    if (eligible) {
      ++two_sided;
      if (front!=back) ++conflicts;
      const std::array<uint64_t,6> key{depth,front,back,cull,vs,ps};
      if (states.size()<32 && states.insert(key).second) {
        std::ofstream log(output,std::ios::app);
        log<<"{\"kind\":\"state\",\"frame\":"<<frame
           <<",\"depth_control\":"<<depth<<",\"front\":"<<front
           <<",\"back\":"<<back<<",\"cull_control\":"<<cull
           <<",\"reference_differs\":"<<((front&255)!=(back&255))
           <<",\"read_mask_differs\":"<<(((front>>8)&255)!=((back>>8)&255))
           <<",\"write_mask_differs\":"<<(((front>>16)&255)!=((back>>16)&255))
           <<",\"vs\":\""<<std::hex<<vs<<"\",\"ps\":\""<<ps<<std::dec
           <<"\",\"index_count\":"<<count<<"}\n";
      }
    }
    if (last_report_frame==UINT64_MAX || frame>=last_report_frame+60) {
      last_report_frame=frame;
      std::ofstream log(output,std::ios::app);
      log<<"{\"kind\":\"totals\",\"frame\":"<<frame<<",\"draws\":"<<draws
         <<",\"two_sided_draws\":"<<two_sided<<",\"conflicting_draws\":"<<conflicts
         <<",\"states_recorded\":"<<states.size()<<"}\n";
    }
  }
};
inline RtvStencilAudit rtv_stencil_audit;
}

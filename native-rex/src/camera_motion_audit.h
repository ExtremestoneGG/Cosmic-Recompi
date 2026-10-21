#pragma once
// Diagnostic only: snapshots of existing VS constants, no GPU readback or state changes.
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

namespace ben10_camera_audit {
struct Audit {
  const char* output=std::getenv("BEN10_CAMERA_MOTION_AUDIT");
  using Clock=std::chrono::steady_clock;
  Clock::time_point start=Clock::now();
  struct Record {
    uint64_t time_us=0,frame=0,vs=0;
    uint32_t viewport[4]{};
    float ndc[6]{};
    uint32_t constants[1024]{};
  };
  static_assert(sizeof(Record)==4160);
  struct Key { uint64_t vs;uint32_t viewport[4]; };
  std::array<Key,16> seen{};
  size_t seen_count=0;
  uint64_t previous_frame=~uint64_t(0);
  std::vector<Record> records;
  bool written=false;
  void Flush() {
    if (!output || written || records.empty())return;
    written=true;
    std::ofstream file(output,std::ios::binary);
    file.write("B10CAM01",8);
    file.write(reinterpret_cast<const char*>(records.data()),records.size()*sizeof(Record));
    file.flush();
  }
  void Observe(uint64_t frame,uint64_t vs,const uint32_t* offset,const uint32_t* extent,
               const float* scale,const float* bias,const uint32_t* constants) {
    if (!output || written)return;
    const auto us=std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-start).count();
    if (us<185000000)return;
    if (us>=225000000 || records.size()>=16000) { Flush();return; }
    if (frame!=previous_frame) { previous_frame=frame;seen_count=0; }
    Key key{};key.vs=vs;
    key.viewport[0]=offset[0];key.viewport[1]=offset[1];
    key.viewport[2]=extent[0];key.viewport[3]=extent[1];
    for(size_t i=0;i<seen_count;++i)
      if(seen[i].vs==vs && !std::memcmp(seen[i].viewport,key.viewport,sizeof(key.viewport)))return;
    if(seen_count==seen.size())return;
    seen[seen_count++]=key;
    Record record{};record.time_us=uint64_t(us);record.frame=frame;record.vs=vs;
    std::memcpy(record.viewport,key.viewport,sizeof(record.viewport));
    std::memcpy(record.ndc,scale,3*sizeof(float));
    std::memcpy(record.ndc+3,bias,3*sizeof(float));
    std::memcpy(record.constants,constants,sizeof(record.constants));
    records.push_back(record);
  }
};
inline Audit audit;
}

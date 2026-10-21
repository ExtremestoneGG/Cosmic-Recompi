#pragma once
// Sparse, opt-in timestamps of actual guest draws. Diagnostic builds only.
// Samples rotate across draw ordinals; never skip rendering or wait for results.
#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <d3d12.h>
#include <wrl/client.h>

namespace ben10_profile {
struct DrawProfile {
  const char* output=std::getenv("BEN10_D3D12_DRAW_PROFILE");
  const char* target=std::getenv("BEN10_D3D12_DRAW_TARGET");
  uint64_t target_vs=0,target_ps=0;
  bool target_valid=false;
  DrawProfile() {
    if (!target) return;
    char* end=nullptr;
    target_vs=std::strtoull(target,&end,16);
    if (end==target || *end!=':') return;
    const char* pixel=end+1;
    target_ps=std::strtoull(pixel,&end,16);
    target_valid=end!=pixel && *end=='\0';
  }
  static constexpr UINT Slots=512;
  struct Sample {
    uint64_t fence=0,vs=0,ps=0,ordinal=0;
    UINT count=0,msaa=0;
    uint64_t pixel_modification=0;
    uint32_t color_mask=0,depth_control=0,depth_format=0,written_targets=0;
    std::array<uint32_t,4> blend_controls{},color_formats{};
    std::array<uint32_t,15> draw_state{};
  };
  std::array<Sample,Slots> samples{};
  Microsoft::WRL::ComPtr<ID3D12QueryHeap> queries;
  Microsoft::WRL::ComPtr<ID3D12Resource> readback;
  uint64_t* mapped=nullptr;
  UINT64 frequency=0,ordinal=0;
  bool attempted=false;
  bool Initialize(ID3D12Device* device,ID3D12CommandQueue* queue) {
    if (attempted) return mapped && frequency;
    attempted=true;
    if (FAILED(queue->GetTimestampFrequency(&frequency)) || !frequency) return false;
    D3D12_QUERY_HEAP_DESC desc{};desc.Type=D3D12_QUERY_HEAP_TYPE_TIMESTAMP;desc.Count=Slots*2;
    if (FAILED(device->CreateQueryHeap(&desc,IID_PPV_ARGS(&queries)))) return false;
    D3D12_HEAP_PROPERTIES heap{};heap.Type=D3D12_HEAP_TYPE_READBACK;
    D3D12_RESOURCE_DESC buffer{};buffer.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer.Width=Slots*2*sizeof(uint64_t);buffer.Height=1;buffer.DepthOrArraySize=1;
    buffer.MipLevels=1;buffer.SampleDesc.Count=1;buffer.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    if (FAILED(device->CreateCommittedResource(&heap,D3D12_HEAP_FLAG_NONE,&buffer,
        D3D12_RESOURCE_STATE_COPY_DEST,nullptr,IID_PPV_ARGS(&readback)))) return false;
    D3D12_RANGE range{0,size_t(buffer.Width)};
    return SUCCEEDED(readback->Map(0,&range,reinterpret_cast<void**>(&mapped)));
  }
  void Drain(ID3D12Fence* fence) {
    if (!mapped) return;
    const auto completed=fence->GetCompletedValue();
    if (completed==UINT64_MAX) return;
    std::ofstream log;
    const auto unix_ms=std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    for (UINT i=0;i<Slots;++i) {
      auto& s=samples[i];
      if (!s.fence || s.fence>completed) continue;
      const uint64_t begin=mapped[i*2],end=mapped[i*2+1];
      if (end>=begin) {
        if (!log.is_open()) log.open(output,std::ios::app);
        log << "{\"time_ms\":" << unix_ms << ",\"ordinal\":" << s.ordinal
            << ",\"vs\":\"" << std::hex << s.vs << "\",\"ps\":\"" << s.ps << std::dec
            << "\",\"vertices_or_indices\":" << s.count << ",\"msaa_log2\":" << s.msaa
            << ",\"pixel_modification\":\"" << std::hex << s.pixel_modification << std::dec
            << "\",\"color_mask\":" << s.color_mask
            << ",\"depth_control\":" << s.depth_control
            << ",\"depth_format\":" << s.depth_format
            << ",\"written_targets\":" << s.written_targets
            << ",\"blend_controls\":[";
        for (uint32_t rt=0;rt<4;++rt) log << (rt ? "," : "") << s.blend_controls[rt];
        log << "],\"color_formats\":[";
        for (uint32_t rt=0;rt<4;++rt) log << (rt ? "," : "") << s.color_formats[rt];
        log << "],\"submission\":" << s.fence << ",\"draw_state\":[";
        for (uint32_t k=0;k<s.draw_state.size();++k) log << (k ? "," : "") << s.draw_state[k];
        log << "],\"gpu_ms\":" << double(end-begin)*1000.0/frequency << "}\n";
      }
      s.fence=0;
    }
  }
  template<class Deferred> int Begin(Deferred& commands,ID3D12Device* device,
      ID3D12CommandQueue* queue,ID3D12Fence* fence,uint64_t submission,
      uint64_t vs,uint64_t ps,UINT count,UINT msaa) {
    if (!output) return -1;
    ++ordinal;
    // Targeted mode measures every matching draw; it never skips rendering.
    // The default remains sparse rotating sampling across all shaders.
    if (target) {
      if (!target_valid || vs!=target_vs || ps!=target_ps) return -1;
    } else if (ordinal%127) return -1;
    if (!Initialize(device,queue)) return -1;
    Drain(fence);
    for (UINT i=0;i<Slots;++i) if (!samples[i].fence) {
      samples[i]={submission,vs,ps,ordinal,count,msaa};
      commands.D3DEndQuery(queries.Get(),D3D12_QUERY_TYPE_TIMESTAMP,i*2);
      return int(i);
    }
    return -1;
  }
  template<class Deferred> void End(Deferred& commands,int slot) {
    if (slot<0) return;
    commands.D3DEndQuery(queries.Get(),D3D12_QUERY_TYPE_TIMESTAMP,UINT(slot)*2+1);
    commands.D3DResolveQueryData(queries.Get(),D3D12_QUERY_TYPE_TIMESTAMP,UINT(slot)*2,2,
                               readback.Get(),UINT64(slot)*2*sizeof(uint64_t));
  }
  void Shutdown(ID3D12Fence* fence) {
    Drain(fence);
    if (mapped) {D3D12_RANGE written{0,0};readback->Unmap(0,&written);mapped=nullptr;}
    readback.Reset();queries.Reset();
  }
};
inline thread_local DrawProfile draw_profile;
} // namespace ben10_profile

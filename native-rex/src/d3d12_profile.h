#pragma once
// Opt-in profiling of the existing SDK path. Never changes guest registers.
#include <array>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <d3d12.h>
#include <wrl/client.h>
#include <Windows.h>

namespace ben10_profile {
using Clock=std::chrono::steady_clock;
enum Metric { Draw, Submit, FenceWait, MetricCount };
struct Profile {
  const char* output=std::getenv("BEN10_D3D12_PROFILE");
  std::array<double,MetricCount> wall_ms{};
  uint64_t frames=0,draws=0,submits=0,gpu_samples=0,gpu_dropped=0;
  std::array<uint64_t,3> msaa{};
  std::array<uint64_t,8> rov_format_candidates{};
  uint64_t rtv_zero_lod_candidates=0;
  uint64_t direct_resolve_copies=0;
  std::array<uint64_t,4> rov_depth_candidates{};
  std::array<uint64_t,9> rov_depth_control_candidates{};
  std::array<uint64_t,2> rov_alpha_candidates{};
  Clock::time_point previous_frame=Clock::now(),report_start=previous_frame;
  std::vector<double> frame_times;
  uint64_t previous_cpu=0;
  double gpu_ms=0;
  static constexpr UINT Slots=256;
  Microsoft::WRL::ComPtr<ID3D12QueryHeap> queries;
  Microsoft::WRL::ComPtr<ID3D12Resource> readback;
  std::array<uint64_t,Slots> pending{};
  uint64_t* mapped=nullptr;
  UINT64 frequency=0;
  bool initialized=false;
  static uint64_t ThreadCpu() {
    FILETIME a,b,k,u;
    if (!GetThreadTimes(GetCurrentThread(),&a,&b,&k,&u)) return 0;
    return ((uint64_t(k.dwHighDateTime)<<32)|k.dwLowDateTime)+
           ((uint64_t(u.dwHighDateTime)<<32)|u.dwLowDateTime);
  }
  bool Initialize(ID3D12Device* device,ID3D12CommandQueue* queue) {
    if (initialized) return mapped && frequency;
    initialized=true;
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
    if (completed==UINT64_MAX) return; // Device removed; results are unavailable.
    for (UINT i=0;i<Slots;++i) if (pending[i] && pending[i]<=completed) {
      const uint64_t begin=mapped[i*2],end=mapped[i*2+1];
      if (end>=begin) {gpu_ms+=double(end-begin)*1000.0/frequency;++gpu_samples;}
      pending[i]=0;
    }
  }
  int BeginGpu(ID3D12Device* device,ID3D12CommandQueue* queue,
               ID3D12GraphicsCommandList* commands,ID3D12Fence* fence,uint64_t submission) {
    if (!output) return -1;
    ++submits;
    if (!Initialize(device,queue)) return -1;
    Drain(fence);
    for (UINT i=0;i<Slots;++i) if (!pending[i]) {
      pending[i]=submission;
      commands->EndQuery(queries.Get(),D3D12_QUERY_TYPE_TIMESTAMP,i*2);
      return int(i);
    }
    ++gpu_dropped;return -1;
  }
  void EndGpu(ID3D12GraphicsCommandList* commands,int slot) {
    if (slot<0) return;
    commands->EndQuery(queries.Get(),D3D12_QUERY_TYPE_TIMESTAMP,UINT(slot)*2+1);
    commands->ResolveQueryData(queries.Get(),D3D12_QUERY_TYPE_TIMESTAMP,UINT(slot)*2,2,
                              readback.Get(),UINT64(slot)*2*sizeof(uint64_t));
  }
  void Frame(uint32_t width,uint32_t height,ID3D12Fence* fence) {
    if (!output) return;
    const auto now=Clock::now();
    const auto cpu=ThreadCpu();
    if (!previous_cpu) { previous_cpu=cpu;report_start=now;previous_frame=now;return; }
    ++frames;
    frame_times.push_back(std::chrono::duration<double,std::milli>(now-previous_frame).count());
    previous_frame=now;
    const double seconds=std::chrono::duration<double>(now-report_start).count();
    if (seconds<5.0) return;
    Drain(fence);
    std::sort(frame_times.begin(),frame_times.end());
    auto percentile=[&](double p) {return frame_times[size_t((frame_times.size()-1)*p)];};
    std::ofstream log(output,std::ios::app);
    const uint64_t unix_ms=std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    log << "{\"time_ms\":" << unix_ms << ",\"frames\":" << frames
        << ",\"width\":" << width << ",\"height\":" << height
        << ",\"fps\":" << frames/seconds << ",\"frame_ms\":" << seconds*1000/frames
        << ",\"frame_p50_ms\":" << percentile(.50)
        << ",\"frame_p95_ms\":" << percentile(.95)
        << ",\"frame_p99_ms\":" << percentile(.99)
        << ",\"gpu_worker_cpu_ms_per_frame\":" << double(cpu-previous_cpu)/10000.0/frames
        << ",\"draw_calls_per_frame\":" << double(draws)/frames
        << ",\"draw_wall_ms_per_frame\":" << wall_ms[Draw]/frames
        << ",\"submit_wall_ms_per_frame\":" << wall_ms[Submit]/frames
        << ",\"fence_wait_ms_per_frame\":" << wall_ms[FenceWait]/frames
        << ",\"submissions_per_frame\":" << double(submits)/frames
        << ",\"gpu_guest_commands_ms_per_frame\":";
    if (gpu_samples) log << gpu_ms/frames;else log << "null";
    log << ",\"direct_resolve_copies\":" << direct_resolve_copies
        << ",\"rtv_zero_lod_candidates\":" << rtv_zero_lod_candidates
        << ",\"gpu_query_samples\":" << gpu_samples << ",\"gpu_query_dropped\":" << gpu_dropped
        << ",\"msaa_draw_counts\":[" << msaa[0] << ',' << msaa[1] << ',' << msaa[2]
        << "],\"rov_format_candidates\":[";
    for (size_t i=0;i<rov_format_candidates.size();++i) {
      if (i) log << ',';
      log << rov_format_candidates[i];
    }
    log << "],\"rov_depth_candidates\":[";
    for (size_t i=0;i<rov_depth_candidates.size();++i) {
      if (i) log << ',';
      log << rov_depth_candidates[i];
    }
    log << "],\"rov_depth_control_candidates\":[";
    for (size_t i=0;i<rov_depth_control_candidates.size();++i) {
      if (i) log << ',';
      log << rov_depth_control_candidates[i];
    }
    log << "],\"rov_alpha_candidates\":[" << rov_alpha_candidates[0] << ',' << rov_alpha_candidates[1];
    log << "],\"scope\":\"GPU timestamps cover guest command lists, exclude presentation and idle gaps. CPU is graphics worker only. Wall scopes overlap; do not sum. State counters are pipeline candidates, not hardware draw counts.\"}\n";
    frames=draws=submits=gpu_samples=gpu_dropped=0;gpu_ms=0;wall_ms.fill(0);msaa.fill(0);
    rov_format_candidates.fill(0);
    rtv_zero_lod_candidates=0;
    direct_resolve_copies=0;
    rov_depth_candidates.fill(0);
    rov_depth_control_candidates.fill(0);
    rov_alpha_candidates.fill(0);
    frame_times.clear();previous_cpu=cpu;report_start=now;
  }
  void Shutdown() {
    if (mapped) { D3D12_RANGE written{0,0};readback->Unmap(0,&written);mapped=nullptr; }
    readback.Reset();queries.Reset();
  }
};
inline thread_local Profile profile;
struct Scope {
  Metric metric;bool enabled;Clock::time_point begin;
  explicit Scope(Metric m):metric(m),enabled(profile.output!=nullptr) {
    if (enabled) begin=Clock::now();
  }
  ~Scope() {if (enabled) profile.wall_ms[metric]+=std::chrono::duration<double,std::milli>(Clock::now()-begin).count();}
};
} // namespace ben10_profile

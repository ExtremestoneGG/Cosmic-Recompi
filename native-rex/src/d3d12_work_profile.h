#pragma once
// Opt-in GPU timeline groups; never changes rendering state or omits work.
#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <d3d12.h>
#include <wrl/client.h>
namespace ben10_profile {
struct WorkProfile {
  const char* output=std::getenv("BEN10_D3D12_WORK_PROFILE");
  const bool profile_barriers=std::getenv("BEN10_PROFILE_GPU_BARRIERS")!=nullptr;
  static constexpr UINT Slots=4096;
  struct Sample { uint64_t fence=0,pipeline=0;UINT kind=0,count=0;uint64_t bytes=0; };
  std::array<Sample,Slots> samples{};
  Microsoft::WRL::ComPtr<ID3D12QueryHeap> queries;
  Microsoft::WRL::ComPtr<ID3D12Resource> readback;
  uint64_t* mapped=nullptr;UINT64 frequency=0;
  bool attempted=false;int active=-1;
  uint64_t last_completed=0;
  struct Total { double ms=0;uint64_t groups=0,commands=0,bytes=0; };
  std::map<std::pair<UINT,uint64_t>,Total> totals;
  std::set<uint64_t> named_pipelines;
  void NamePipeline(uint64_t pipeline) {
    if(!pipeline || !named_pipelines.insert(pipeline).second)return;
    auto* state=reinterpret_cast<ID3D12PipelineState*>(pipeline);
    UINT bytes=0;
    std::string name;
    if(SUCCEEDED(state->GetPrivateData(WKPDID_D3DDebugObjectNameW,&bytes,nullptr)) && bytes) {
      std::vector<wchar_t> wide(bytes/sizeof(wchar_t)+1,0);
      if(SUCCEEDED(state->GetPrivateData(WKPDID_D3DDebugObjectNameW,&bytes,wide.data()))) {
        for(wchar_t ch:wide) {
          if(!ch)break;
          name+=ch>=32 && ch<127 && ch!='"' && ch!='\\' ? char(ch) : '?';
        }
      }
    }
    std::ofstream log(output,std::ios::app);
    log<<"{\"pipeline_name\":\""<<name<<"\",\"pipeline\":\""<<std::hex<<pipeline<<"\"}\n";
  }
  std::chrono::steady_clock::time_point previous=std::chrono::steady_clock::now();
  bool Initialize(ID3D12Device* device,ID3D12CommandQueue* queue) {
    if(attempted)return mapped && frequency;
    attempted=true;
    if(FAILED(queue->GetTimestampFrequency(&frequency)) || !frequency)return false;
    D3D12_QUERY_HEAP_DESC desc{};desc.Type=D3D12_QUERY_HEAP_TYPE_TIMESTAMP;desc.Count=Slots*2;
    if(FAILED(device->CreateQueryHeap(&desc,IID_PPV_ARGS(&queries))))return false;
    D3D12_HEAP_PROPERTIES heap{};heap.Type=D3D12_HEAP_TYPE_READBACK;
    D3D12_RESOURCE_DESC buffer{};buffer.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer.Width=Slots*2*sizeof(uint64_t);buffer.Height=1;buffer.DepthOrArraySize=1;
    buffer.MipLevels=1;buffer.SampleDesc.Count=1;buffer.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    if(FAILED(device->CreateCommittedResource(&heap,D3D12_HEAP_FLAG_NONE,&buffer,
        D3D12_RESOURCE_STATE_COPY_DEST,nullptr,IID_PPV_ARGS(&readback))))return false;
    D3D12_RANGE range{0,size_t(buffer.Width)};
    return SUCCEEDED(readback->Map(0,&range,reinterpret_cast<void**>(&mapped)));
  }
  void Drain(uint64_t completed) {
    if(!mapped || completed==UINT64_MAX || completed==last_completed)return;
    last_completed=completed;
    const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    for(UINT i=0;i<Slots;++i) {
      auto& s=samples[i];if(!s.fence || s.fence>completed || int(i)==active)continue;
      const uint64_t begin=mapped[i*2],end=mapped[i*2+1];
      if(end>=begin) {
        auto& total=totals[{s.kind,s.pipeline}];
        total.ms+=double(end-begin)*1000.0/frequency;
        ++total.groups;total.commands+=s.count;total.bytes+=s.bytes;
      }
      s.fence=0;
    }
    const auto clock=std::chrono::steady_clock::now();
    if(clock-previous>=std::chrono::seconds(5)) {
      std::ofstream log(output,std::ios::app);
      log<<"{\"time_ms\":"<<now<<",\"interval_seconds\":"
         <<std::chrono::duration<double>(clock-previous).count()<<",\"groups\":[";
      bool first=true;
      for(const auto& [key,total]:totals) {
        log<<(first?"":",")<<"{\"kind\":"<<key.first<<",\"pipeline\":\""
           <<std::hex<<key.second<<std::dec<<"\",\"commands\":"<<total.commands
           <<",\"bytes\":"<<total.bytes<<",\"groups\":"<<total.groups<<",\"gpu_ms\":"<<total.ms<<"}";
        first=false;
      }
      log<<"]}\n";totals.clear();previous=clock;
    }
  }
  void End(ID3D12GraphicsCommandList* commands) {
    if(active<0)return;
    const UINT i=UINT(active);
    commands->EndQuery(queries.Get(),D3D12_QUERY_TYPE_TIMESTAMP,i*2+1);
    commands->ResolveQueryData(queries.Get(),D3D12_QUERY_TYPE_TIMESTAMP,i*2,2,
                              readback.Get(),UINT64(i)*2*sizeof(uint64_t));
    active=-1;
  }
  void Mark(ID3D12GraphicsCommandList* commands,ID3D12Device* device,
      ID3D12CommandQueue* queue,uint64_t completed,uint64_t submission,
      UINT kind,uint64_t pipeline,uint64_t bytes=0) {
    if(!output)return;
    NamePipeline(pipeline);
    if(active>=0 && samples[size_t(active)].kind==kind && samples[size_t(active)].pipeline==pipeline) {
      ++samples[size_t(active)].count;samples[size_t(active)].bytes+=bytes;return;
    }
    End(commands);
    if(!Initialize(device,queue))return;
    Drain(completed);
    for(UINT i=0;i<Slots;++i)if(!samples[i].fence) {
      samples[i]={submission,pipeline,kind,1,bytes};active=int(i);
      commands->EndQuery(queries.Get(),D3D12_QUERY_TYPE_TIMESTAMP,i*2);return;
    }
    std::ofstream log(output,std::ios::app);log<<"{\"dropped\":true}\n";
  }
  void Shutdown(uint64_t completed) {
    Drain(completed);
    if(mapped){D3D12_RANGE range{0,0};readback->Unmap(0,&range);mapped=nullptr;}
    readback.Reset();queries.Reset();
  }
};
inline thread_local WorkProfile work_profile;
} // namespace ben10_profile

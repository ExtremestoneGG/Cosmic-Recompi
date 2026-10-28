#pragma once
#include <Windows.h>
#include <psapi.h>
#include <dxgi1_4.h>
#include <chrono>
#include <cstdint>

// Process counters, sampled at most once per second; no GPU synchronization.
// DXGI local/non-local usage is process usage across adapters, not total board use.
class Ben10MemoryMetrics {
 public:
  uint64_t ram=0, committed=0, gpu_local=0, gpu_shared=0;
  bool ram_available=false, gpu_available=false;
  Ben10MemoryMetrics()=default;
  Ben10MemoryMetrics(const Ben10MemoryMetrics&)=delete;
  Ben10MemoryMetrics& operator=(const Ben10MemoryMetrics&)=delete;
  ~Ben10MemoryMetrics() {
    if(factory_)factory_->Release();
    if(module_)FreeLibrary(module_);
  }
  void Refresh() {
    const auto now=std::chrono::steady_clock::now();
    if(now-last_<std::chrono::seconds(1))return;
    last_=now;
    PROCESS_MEMORY_COUNTERS_EX counters{};counters.cb=sizeof(counters);
    ram_available=K32GetProcessMemoryInfo(GetCurrentProcess(),
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),sizeof(counters))!=FALSE;
    if(ram_available){ram=counters.WorkingSetSize;committed=counters.PrivateUsage;}
    if(!attempted_) {
      attempted_=true;
      // Keep the system module loaded for the lifetime of its COM objects.
      module_=LoadLibraryW(L"dxgi.dll");
      if(module_) {
        using Create=HRESULT(WINAPI*)(REFIID,void**);
        auto create=reinterpret_cast<Create>(GetProcAddress(module_,"CreateDXGIFactory1"));
        if(create)create(__uuidof(IDXGIFactory1),reinterpret_cast<void**>(&factory_));
      }
    }
    gpu_local=gpu_shared=0;gpu_available=false;
    if(!factory_)return;
    for(UINT index=0;;++index) {
      IDXGIAdapter1* adapter=nullptr;
      if(factory_->EnumAdapters1(index,&adapter)!=S_OK)break;
      IDXGIAdapter3* memory=nullptr;
      if(SUCCEEDED(adapter->QueryInterface(__uuidof(IDXGIAdapter3),reinterpret_cast<void**>(&memory)))) {
        DXGI_QUERY_VIDEO_MEMORY_INFO local{},shared{};
        if(SUCCEEDED(memory->QueryVideoMemoryInfo(0,DXGI_MEMORY_SEGMENT_GROUP_LOCAL,&local)) &&
           SUCCEEDED(memory->QueryVideoMemoryInfo(0,DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL,&shared))) {
          gpu_local+=local.CurrentUsage;gpu_shared+=shared.CurrentUsage;gpu_available=true;
        }
        memory->Release();
      }
      adapter->Release();
    }
  }
 private:
  HMODULE module_=nullptr;
  IDXGIFactory1* factory_=nullptr;
  bool attempted_=false;
  std::chrono::steady_clock::time_point last_{};
};

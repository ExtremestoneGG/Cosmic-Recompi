#pragma once
#include <Windows.h>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "../../third_party/rexglue-sdk/thirdparty/renderdoc/renderdoc_app.h"

// Internal diagnostic only. Never enabled by ordinary launchers.
inline RENDERDOC_API_1_0_0*& Ben10FrameCaptureStorage() {
  static RENDERDOC_API_1_0_0* api=nullptr;
  return api;
}
inline void Ben10InitializeFrameCapture() {
  if (!std::getenv("BEN10_RENDERDOC_CAPTURE")) return;
  if (!std::getenv("BEN10_DIAGNOSTIC_CACHE"))
    throw std::runtime_error("GPU frame capture requires diagnostic build");
  std::filesystem::create_directories("D:/Ben 10 recomp/data/analysis/renderdoc");
  // Keep the module loaded until process exit: D3D12 objects retain hooks.
  HMODULE module=GetModuleHandleW(L"renderdoc.dll");
  auto get=module?reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(module,"RENDERDOC_GetAPI")):nullptr;
  auto& api=Ben10FrameCaptureStorage();
  if (!get || !get(eRENDERDOC_API_Version_1_0_0,reinterpret_cast<void**>(&api)) || !api)
    throw std::runtime_error("RenderDoc API unavailable");
  api->SetLogFilePathTemplate("D:/Ben 10 recomp/data/analysis/renderdoc/ben10-rtv");
  api->MaskOverlayBits(0,0);
}
inline bool Ben10TriggerFrameCapture() {
  if (auto* api=Ben10FrameCaptureStorage()) {
    // The presenter runs independently at monitor refresh rate. Capturing the
    // next Present alone can miss every guest GPU command list. Use a short
    // explicit interval spanning several native guest frames, all queues.
    api->StartFrameCapture(nullptr,nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return api->EndFrameCapture(nullptr,nullptr)!=0;
  }
  return false;
}

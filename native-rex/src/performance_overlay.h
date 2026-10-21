#pragma once
#include <rex/ui/imgui_dialog.h>
#include <imgui.h>
#include "frame_metrics.h"
#include <chrono>
#include <algorithm>
#include <fstream>

// Uses the SDK's existing UI renderer. Never generates independent UI frames.
class Ben10PerformanceOverlay final : public rex::ui::ImGuiDialog {
 public:
  explicit Ben10PerformanceOverlay(rex::ui::ImGuiDrawer* drawer):ImGuiDialog(drawer){}
  bool NeedsContinuousRepaint() override { return false; }
 protected:
  void OnDraw(ImGuiIO& io) override {
    const auto now=std::chrono::steady_clock::now();
    const bool refreshed=now-last_refresh_>=std::chrono::milliseconds(250);
    if(refreshed) {
      stats_=Ben10ReadFrameStats();last_refresh_=now;
    }
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x-12,12),ImGuiCond_Always,ImVec2(1,0));
    ImGui::SetNextWindowBgAlpha(.80f);
    const auto flags=ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoInputs|
      ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize|
      ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoNav;
    if(ImGui::Begin("Desempenho do jogo",nullptr,flags)) {
      ImGui::Text("FPS do jogo: %.1f",stats_.fps);
      ImGui::Text("Frame time: %.2f ms | p95: %.2f ms",stats_.latest_ms,stats_.p95_ms);
      if(stats_.count) {
        float maximum=50;
        for(size_t i=0;i<stats_.count;++i)maximum=std::max(maximum,stats_.frame_times_ms[i]);
        ImGui::PlotLines("##tempo",stats_.frame_times_ms.data(),int(stats_.count),0,nullptr,
            0,std::min(maximum,250.0f),ImVec2(300,64));
      }
      ImGui::TextDisabled("30 FPS = 33,3 ms | 60 FPS = 16,7 ms");
      ImGui::TextDisabled("Quadros do jogo (VdSwap), sem repeticoes da janela");
      if(refreshed) {
        if(const char* path=std::getenv("BEN10_OVERLAY_AUDIT")) {
          const auto pos=ImGui::GetWindowPos(),size=ImGui::GetWindowSize();
          std::ofstream file(path);
          file<<"{\"fps\":"<<stats_.fps<<",\"last_ms\":"<<stats_.latest_ms
              <<",\"p95_ms\":"<<stats_.p95_ms<<",\"history_count\":"<<stats_.count
              <<",\"submitted_frames\":"<<stats_.submitted_frames
              <<",\"vertices\":"<<ImGui::GetWindowDrawList()->VtxBuffer.Size
              <<",\"x\":"<<pos.x<<",\"y\":"<<pos.y
              <<",\"width\":"<<size.x<<",\"height\":"<<size.y
              <<",\"display_width\":"<<io.DisplaySize.x<<",\"display_height\":"<<io.DisplaySize.y<<"}";
        }
      }
    }
    ImGui::End();
  }
 private:
  Ben10FrameStats stats_{};
  std::chrono::steady_clock::time_point last_refresh_{};
};

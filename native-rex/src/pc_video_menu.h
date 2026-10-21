#pragma once
#include <rex/ui/imgui_dialog.h>
#include <rex/input/input_system.h>
#include <imgui.h>
#include "frame_pacing_probe.h"
#include <atomic>
#include <filesystem>
#include <fstream>

namespace ben10_pc_video {
inline std::atomic<bool> visible{false};
inline std::atomic<bool> release_guard{false};
inline std::atomic<bool> opening_guard{false};
inline HANDLE closed_event=nullptr;
inline thread_local bool polling_ui=false;
inline bool CapturesGameInput(){return visible.load() || release_guard.load();}
class Dialog final:public rex::ui::ImGuiDialog {
 public:
  Dialog(rex::ui::ImGuiDrawer* drawer,rex::input::InputSystem* input,const std::filesystem::path& user,ImFont* font=nullptr)
      :ImGuiDialog(drawer),input_(input),path_(user/"pc-video.ini"),font_(font) {
    std::ifstream file(path_);std::string line;
    while(std::getline(file,line)) {
      if(line=="frame_limit=60")rate_=1;
      if(line=="resolution=1080p")resolution_=1;
      if(line=="resolution=1440p")resolution_=2;
      if(line=="resolution=4k")resolution_=3;
    }
  }
  bool NeedsContinuousRepaint() override{return visible.load();}
 protected:
  void OnDraw(ImGuiIO& io) override {
    if(!CapturesGameInput())return;
    rex::input::X_INPUT_STATE state{};
    polling_ui=true;
    const bool connected=input_ && input_->GetState(0,&state)==0;
    polling_ui=false;
    if(release_guard && !state.gamepad.buttons)release_guard=false;
    if(!visible)return;
    if(opening_guard && !state.gamepad.buttons)opening_guard=false;
    io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_NavEnableGamepad;
    if(connected) {
      io.BackendFlags|=ImGuiBackendFlags_HasGamepad;
      const uint16_t buttons=opening_guard.load()?uint16_t(0):uint16_t(state.gamepad.buttons);
      io.AddKeyEvent(ImGuiKey_GamepadDpadUp,(buttons&1)!=0);
      io.AddKeyEvent(ImGuiKey_GamepadDpadDown,(buttons&2)!=0);
      io.AddKeyEvent(ImGuiKey_GamepadDpadLeft,(buttons&4)!=0);
      io.AddKeyEvent(ImGuiKey_GamepadDpadRight,(buttons&8)!=0);
      io.AddKeyEvent(ImGuiKey_GamepadFaceDown,(buttons&0x1000)!=0);
      io.AddKeyEvent(ImGuiKey_GamepadFaceRight,(buttons&0x2000)!=0);
    }
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x*.5f,io.DisplaySize.y*.5f),ImGuiCond_Always,ImVec2(.5f,.5f));
    ImGui::SetNextWindowSize(ImVec2(600,0),ImGuiCond_Always);
    if(font_)ImGui::PushFont(font_);
    ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(.025f,.08f,.055f,.98f));
    bool open=true;
    if(ImGui::Begin("VIDEO",&open,ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoCollapse)) {
      ImGui::TextUnformatted("Resolution");
      if(ImGui::IsWindowAppearing())ImGui::SetKeyboardFocusHere();
      ImGui::SetNextItemWidth(-1);
      ImGui::Combo("##resolution",&resolution_,"1280 x 720 (native)\0" "1920 x 1080\0" "2560 x 1440\0" "3840 x 2160\0");
      ImGui::SetItemDefaultFocus();
      ImGui::TextDisabled("Resolution changes apply the next time the game starts.");
      ImGui::Spacing();ImGui::TextUnformatted("Frame limit");
      ImGui::SetNextItemWidth(-1);
      ImGui::Combo("##limit",&rate_,"30 FPS\0" "60 FPS\0");
      ImGui::BeginDisabled();bool uncapped=false;ImGui::Checkbox("Uncapped (not available)",&uncapped);ImGui::EndDisabled();
      ImGui::TextDisabled("60 FPS is a limit, not a performance guarantee.");
      ImGui::TextDisabled("Camera stability above 30 FPS is still under investigation.");
      if(ImGui::Button("Apply",ImVec2(120,36))) {
        try {
        static const char* resolutions[]={"720p","1080p","1440p","4k"};
        std::filesystem::create_directories(path_.parent_path());
        const auto temporary=path_.wstring()+L".tmp";
        {std::ofstream file(std::filesystem::path(temporary),std::ios::binary|std::ios::trunc);
         file<<"resolution="<<resolutions[resolution_]<<"\nframe_limit="<<(rate_?60:30)<<"\n";
         file.flush();failed_=!file;}
        if(!failed_)failed_=!MoveFileExW(temporary.c_str(),path_.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
        if(!failed_)failed_=!ben10_pacing_probe::pacer.SetRate(rate_?60:30);
        saved_=!failed_;
        REXLOG_INFO("Cosmic VIDEO settings: resolution={}, cap={}, saved={}",resolutions[resolution_],rate_?60:30,saved_);
        } catch(const std::exception&) {failed_=true;saved_=false;}
      }
      ImGui::SameLine();if(ImGui::Button("Back",ImVec2(120,36)))open=false;
      if(saved_)ImGui::TextUnformatted("Saved. Restart the game to apply the selected resolution.");
      if(failed_)ImGui::TextUnformatted("Could not save or apply settings. Check folder permissions.");
      if(ImGui::IsKeyPressed(ImGuiKey_Escape)||ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight))open=false;
    }
    ImGui::End();ImGui::PopStyleColor();
    if(font_)ImGui::PopFont();
    if(!open){REXLOG_INFO("Cosmic VIDEO closed");visible=false;release_guard=true;if(closed_event)SetEvent(closed_event);}
  }
 private:
  rex::input::InputSystem* input_;
  std::filesystem::path path_;
  ImFont* font_;
  int resolution_=0,rate_=0;
  bool saved_=false,failed_=false;
};
}

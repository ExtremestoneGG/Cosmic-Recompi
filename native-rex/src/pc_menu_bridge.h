#pragma once
// Original-menu integration, enabled by the launcher's handoff event or an
// explicit private diagnostic. No host popup replaces the game's menu.
// Registration uses the existing game script registry and SDK thunk allocator.
#include <rex/runtime.h>
#include <rex/system/function_dispatcher.h>
#include <rex/system/thread_state.h>
#include <rex/system/xmemory.h>
#include <rex/ui/window.h>
#include <rex/ppc.h>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include "pc_video_menu.h"
#include "all_aliens_paging.h"
#include "performance_visibility.h"
#include <vector>
#include <mutex>
#include <rex/input/input_system.h>

namespace ben10_pc_menu {
inline std::atomic<rex::Runtime*> runtime{nullptr};
inline std::atomic<rex::ui::Window*> window{nullptr};
inline std::atomic<bool> registered{false};
inline ben10::AlienPages alien_pages;
inline bool all_aliens_enabled=false;
inline bool hero_time_enabled=false;
inline bool hero_button_armed=false;
inline bool hero_input_observed=false;
inline std::mutex video_settings_mutex;
inline void ReturnVoid(PPCContext& ctx,uint8_t* base);
inline void SaveVideoKey(const std::filesystem::path& path,const std::string& key,const std::string& value);
inline void ReturnNumber(PPCContext& ctx,uint8_t* base,float value) {
  // Original sub_820F9090: BE u16 type=2, u16 flags=0, f32 payload.
  // Native script return slots are distinct from the PPC integer return value.
  if(ctx.r4.s32<0 || ctx.r4.u32>=1024)return;
  const uint32_t slot=0x826F8718+ctx.r4.u32*8;
  uint32_t bits;std::memcpy(&bits,&value,4);
  const uint32_t encoded_type=_byteswap_ulong(0x00020000);
  bits=_byteswap_ulong(bits);
  std::memcpy(base+slot,&encoded_type,4);std::memcpy(base+slot+4,&bits,4);
}
inline void AllAliensPage(PPCContext& ctx,uint8_t* base) {
  if(!all_aliens_enabled){alien_pages.Poll(false,false,0,0);ReturnNumber(ctx,base,-1);return;}
  auto* host=runtime.load();
  rex::input::X_INPUT_STATE state{};
  auto* input=host?static_cast<rex::input::InputSystem*>(host->input_system()):nullptr;
  const bool connected=input && input->GetState(0,&state)==0;
  ReturnNumber(ctx,base,float(alien_pages.Poll(all_aliens_enabled,connected,
      state.gamepad.left_trigger,state.gamepad.right_trigger)));
}
inline void AllAliensEnabled(PPCContext& ctx,uint8_t* base) {
  if(std::getenv("BEN10_MENU_AUDIT"))REXLOG_INFO("Cosmic menu audit: Gameplay rows created");
  ReturnNumber(ctx,base,all_aliens_enabled?1.0f:0.0f);
}
inline void AllAliensTrace(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);
  REXLOG_INFO("Cosmic All Aliens: applied page change through original game HUD");
}
inline void ToggleAllAliens(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);
  auto* host=runtime.load();if(!host)return;
  const bool next=!all_aliens_enabled;
  try {
    std::lock_guard guard(video_settings_mutex);
    SaveVideoKey(host->user_data_root()/"pc-gameplay.ini","all_aliens",next?"on":"off");
    all_aliens_enabled=next;
    REXLOG_INFO("Cosmic Gameplay: All Aliens {}",next?"ON":"OFF");
  }catch(const std::exception& e){REXLOG_ERROR("Cosmic Gameplay settings: {}",e.what());}
}
inline bool HeroTimeDiagnostic() {
  const char* value=std::getenv("BEN10_HERO_TIME_DIAGNOSTIC");
  return value && std::strcmp(value,"1")==0;
}
inline void HeroTimeEnabled(PPCContext& ctx,uint8_t* base) {
  ReturnNumber(ctx,base,hero_time_enabled?1.0f:0.0f);
}
inline void ToggleHeroTime(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);auto* host=runtime.load();if(!host)return;
  try {
    std::lock_guard guard(video_settings_mutex);
    const bool next=!hero_time_enabled;
    SaveVideoKey(host->user_data_root()/"pc-gameplay.ini","hero_time",next?"on":"off");
    hero_time_enabled=next;hero_button_armed=false;
    REXLOG_INFO("Cosmic private Hero Time: {}",next?"ON":"OFF");
  }catch(const std::exception& error){REXLOG_ERROR("Cosmic Hero Time settings: {}",error.what());}
}
inline void HeroTimeButton(PPCContext& ctx,uint8_t* base) {
  auto* host=runtime.load();rex::input::X_INPUT_STATE state{};
  auto* input=host?static_cast<rex::input::InputSystem*>(host->input_system()):nullptr;
  const bool connected=input && input->GetState(0,&state)==0;
  if(!hero_input_observed) {
    hero_input_observed=true;
    REXLOG_INFO("Cosmic private Hero Time: input callback observed; enabled={}, connected={}",hero_time_enabled,connected);
  }
  if(!hero_time_enabled || !connected){hero_button_armed=false;ReturnNumber(ctx,base,0);return;}
  const bool down=(state.gamepad.buttons & 0x0040)!=0; // L3; preserve R3/LT/RT.
  const bool pressed=down && hero_button_armed;
  hero_button_armed=!down;
  if(pressed)REXLOG_INFO("Cosmic private Hero Time: L3 edge observed");
  ReturnNumber(ctx,base,pressed?1.0f:0.0f);
}
inline void HeroTimeDelta(PPCContext& ctx,uint8_t* base) {
  // Original guest simulation delta. The authored gameplay update must gate
  // pauses/cutscenes/loading; presentation callbacks must never advance it.
  uint32_t encoded;std::memcpy(&encoded,base+0x826EBFC0,4);
  const uint32_t bits=_byteswap_ulong(encoded);float seconds;
  std::memcpy(&seconds,&bits,4);
  ReturnNumber(ctx,base,std::isfinite(seconds) && seconds>0?seconds:0.0f);
}
template<int Event> void HeroTimeTrace(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);
  const char* events[]={"resources ready","Ultimate confirmed","return requested","Ben confirmed; recharge started","recharge complete"};
  REXLOG_INFO("Cosmic private Hero Time: {}",events[Event]);
}
inline bool enabled() {
  if(std::getenv("BEN10_RETURN_TO_LAUNCHER_EVENT"))return true;
  const char* value=std::getenv("BEN10_PC_MENU_DIAGNOSTIC");
  return value && std::strcmp(value,"1")==0;
}
inline void ReturnVoid(PPCContext& ctx,uint8_t* base) {
  // The game's native callback ABI returns through an indexed VM value,
  // not the PPC r3 integer return register (see original clearfunctions).
  if(ctx.r4.s32>=0 && ctx.r4.u32<1024) {
    uint32_t empty=0;
    std::memcpy(base+0x826F8718+ctx.r4.u32*8,&empty,sizeof(empty));
  }
}
inline void QuitToLauncher(PPCContext& ctx, uint8_t* base) {
  REXLOG_INFO("Cosmic PC menu: QUIT TO LAUNCHER selected");
  ReturnVoid(ctx,base);
  auto* current=window.load();
  if(!current)return;
  current->app_context().CallInUIThreadDeferred([] {
    auto* current=window.load();
    if(!current)return;
    wchar_t name[256]{};
    DWORD size=GetEnvironmentVariableW(L"BEN10_RETURN_TO_LAUNCHER_EVENT",name,256);
    if(size && size<256) {
      HANDLE event=OpenEventW(EVENT_MODIFY_STATE,FALSE,name);
      if(event){SetEvent(event);CloseHandle(event);}
    }
    current->RequestClose();
  });
}
inline void WipeProgress(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);
  wchar_t name[256]{};
  DWORD size=GetEnvironmentVariableW(L"BEN10_RESET_PROGRESS_EVENT",name,256);
  if(!size || size>=256){REXLOG_ERROR("Cosmic reset requires the matching launcher");return;}
  HANDLE event=OpenEventW(EVENT_MODIFY_STATE,FALSE,name);
  if(!event){REXLOG_ERROR("Cosmic reset event unavailable; progress preserved");return;}
  SetEvent(event);CloseHandle(event);
  REXLOG_INFO("Cosmic Gameplay: confirmed progress reset; archive deferred until game exits");
  QuitToLauncher(ctx,base);
}
inline void WipePrompt(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);
  if(std::getenv("BEN10_MENU_AUDIT"))REXLOG_INFO("Cosmic menu audit: Wipe Data confirmation opened");
}
template<int Resolution,int Rate> void SetVideo(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);
  auto* host=runtime.load();if(!host)return;
  try {
    std::lock_guard guard(video_settings_mutex);
    auto path=host->user_data_root()/"pc-video.ini";
    std::string resolution="720p",rate="30",line;std::vector<std::string> other;
    {std::ifstream file(path);while(std::getline(file,line)) {
      if(!line.empty() && line.back()=='\r')line.pop_back();
      if(line=="resolution=1080p")resolution="1080p";
      if(line=="resolution=1440p")resolution="1440p";
      if(line=="resolution=4k")resolution="4k";
      if(line=="frame_limit=60")rate="60";
      if(line=="frame_limit=unlimited")rate="unlimited";
      if(line.rfind("resolution=",0)!=0 && line.rfind("frame_limit=",0)!=0)other.push_back(line);
    }}
    if constexpr(Resolution>=0){const char* choices[]={"720p","1080p","1440p","4k"};resolution=choices[Resolution];}
    if constexpr(Rate>=0)rate=Rate==0?"unlimited":Rate==60?"60":"30";
    std::filesystem::create_directories(path.parent_path());auto temporary=path;temporary+=L".native.tmp";
    {std::ofstream file(temporary,std::ios::binary|std::ios::trunc);file<<"resolution="<<resolution<<"\nframe_limit="<<rate<<"\n";for(const auto& extra:other)file<<extra<<'\n';file.flush();if(!file)throw std::runtime_error("Could not write video settings");}
    if(!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Could not commit video settings");
    if constexpr(Rate>=0)if(!ben10_pacing_probe::pacer.SetRate(Rate))throw std::runtime_error("Could not apply frame limit");
    REXLOG_INFO("Cosmic native VIDEO: resolution={}, frame_limit={}, saved=true",resolution,rate);
  }catch(const std::exception& error){REXLOG_ERROR("Cosmic native VIDEO failed: {}",error.what());}
}
inline std::string ReadVideoSetting(const std::filesystem::path& path,const std::string& key,const char* fallback) {
  std::ifstream input(path);std::string line,value=fallback;
  while(std::getline(input,line)) {
    if(!line.empty()&&line.back()=='\r')line.pop_back();
    if(line.rfind(key+"=",0)==0)value=line.substr(key.size()+1);
  }
  return value;
}
inline void SaveVideoKey(const std::filesystem::path& path,const std::string& key,const std::string& value) {
  std::vector<std::string> lines;std::string line;
  {std::ifstream input(path);while(std::getline(input,line)) {
    if(!line.empty()&&line.back()=='\r')line.pop_back();
    if(line.rfind(key+"=",0)!=0)lines.push_back(line);
  }}
  auto temporary=path;temporary+=L".presentation.tmp";
  {std::ofstream output(temporary,std::ios::trunc);for(const auto& extra:lines)output<<extra<<'\n';
    output<<key<<'='<<value<<'\n';output.flush();
    if(!output)throw std::runtime_error("Could not save presentation settings");}
  if(!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
    throw std::runtime_error("Could not commit presentation settings");
}
inline int DisplayMode(const std::filesystem::path& path) {
  const auto mode=ReadVideoSetting(path,"display_mode","windowed");
  return mode=="exclusive"?2:mode=="borderless"?1:0;
}
inline int ResolutionIndex(const std::filesystem::path& path) {
  const auto value=ReadVideoSetting(path,"resolution","720p");
  return value=="4k"?3:value=="1440p"?2:value=="1080p"?1:0;
}
inline void ApplyPresentation(rex::ui::Window* active,int mode,int resolution) {
  using Configure=int(*)(void*,int,int,int,char*,size_t);
  auto configure=reinterpret_cast<Configure>(GetProcAddress(GetModuleHandleW(L"rexruntime.dll"),"Ben10ConfigurePresentation"));
  if(!configure)throw std::runtime_error("Matching SDK presentation extension is required");
  const int widths[]={1280,1920,2560,3840},heights[]={720,1080,1440,2160};
  const bool previous=active->IsFullscreen();active->SetFullscreen(false);
  char error[512]{};
  if(!configure(active->GetNativeWindowHandle(),mode,widths[resolution],heights[resolution],error,sizeof(error))) {
    active->SetFullscreen(previous);throw std::runtime_error(error);
  }
  active->SetFullscreen(mode!=0);
  REXLOG_INFO("Cosmic presentation: mode={}, requested output={}x{}, desktop size retained in borderless",mode,widths[resolution],heights[resolution]);
}
template<int Resolution> void SetResolution(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);auto* host=runtime.load();auto* current=window.load();if(!host || !current)return;
  auto path=host->user_data_root()/"pc-video.ini";
  current->app_context().CallInUIThreadDeferred([path] {
    auto* active=window.load();if(!active)return;
    try {
      std::lock_guard guard(video_settings_mutex);
      ApplyPresentation(active,DisplayMode(path),Resolution);
      const char* choices[]={"720p","1080p","1440p","4k"};
      SaveVideoKey(path,"resolution",choices[Resolution]);
      REXLOG_INFO("Cosmic native VIDEO: output_resolution={}, applied_without_restart=true",choices[Resolution]);
    }catch(const std::exception& error){REXLOG_ERROR("Cosmic resolution failed: {}",error.what());}
  });
}
template<int Mode> void SetDisplayMode(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);auto* host=runtime.load();auto* current=window.load();
  if(!host || !current)return;
  const auto path=host->user_data_root()/"pc-video.ini";
  current->app_context().CallInUIThreadDeferred([path] {
    auto* active=window.load();if(!active)return;
    try {
      std::lock_guard guard(video_settings_mutex);
      ApplyPresentation(active,Mode,ResolutionIndex(path));
      const char* names[]={"windowed","borderless","exclusive"};
      SaveVideoKey(path,"display_mode",names[Mode]);
      REXLOG_INFO("Cosmic native VIDEO: display_mode={}, applied=true",names[Mode]);
    }catch(const std::exception& error){REXLOG_ERROR("Cosmic display mode failed: {}",error.what());}
  });
}
template<bool Visible> void SetPerformanceOverlay(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);auto* host=runtime.load();if(!host)return;
  try {
    std::lock_guard guard(video_settings_mutex);
    auto path=host->user_data_root()/"pc-video.ini";auto temporary=path;temporary+=L".overlay.tmp";
    std::vector<std::string> lines;std::string line;
    {std::ifstream input(path);while(std::getline(input,line)) {
      if(!line.empty()&&line.back()=='\r')line.pop_back();
      if(line.rfind("performance_overlay=",0)!=0)lines.push_back(line);
    }}
    {std::ofstream output(temporary,std::ios::trunc);for(const auto& extra:lines)output<<extra<<'\n';
      output<<"performance_overlay="<<(Visible?"on":"off")<<'\n';output.flush();
      if(!output)throw std::runtime_error("Could not save overlay visibility");}
    if(!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
      throw std::runtime_error("Could not commit overlay visibility");
    ben10::performance_visible.store(Visible,std::memory_order_relaxed);
    REXLOG_INFO("Cosmic native VIDEO: performance_overlay={}",Visible?"on":"off");
  }catch(const std::exception& error){REXLOG_ERROR("Cosmic overlay setting failed: {}",error.what());}
}
inline void OnFrame() {
  auto* host=runtime.load();
  if(!host || registered.load())return;
  auto* thread=rex::runtime::ThreadState::Get();
  if(!thread)return;
  auto* base=host->memory()->virtual_membase();
  // sub_821C1140 stores the script registry here, before registering overlays.
  uint32_t encoded_registry;
  std::memcpy(&encoded_registry,base+0x826EC6A0,sizeof(encoded_registry));
  const uint32_t registry=_byteswap_ulong(encoded_registry);
  if(!registry)return;
  bool expected=false;
  if(!registered.compare_exchange_strong(expected,true))return;
  auto* registration=host->function_dispatcher()->GetFunction(0x82237E78);
  if(!registration) {
    REXLOG_ERROR("Cosmic PC menu: native registration unavailable");return;
  }
  auto add=[&](const char* name,PPCFunc* function) {
    const uint32_t callback=host->function_dispatcher()->AllocateThunk(function,0);
    const uint32_t text=host->memory()->SystemHeapAlloc(uint32_t(std::strlen(name)+1));
    if(!callback || !text)throw std::runtime_error("PC menu callback allocation failed");
    std::memcpy(base+text,name,std::strlen(name)+1);
    // A local context keeps all live guest registers intact across the nested call.
    PPCContext context=*thread->context();
    context.r3.u64=registry;context.r4.u64=text;context.r5.u64=text;
    context.r6.u64=1;context.r7.u64=callback;context.r8.u64=0;
    registration(context,base);
    REXLOG_INFO("Cosmic PC menu: registered {} callback {:08X}, registry {:08X}",name,callback,registry);
  };
  add("cosmic_quit_to_launcher",QuitToLauncher);
  add("cosmic_set_720",SetResolution<0>);
  add("cosmic_set_1080",SetResolution<1>);
  add("cosmic_set_1440",SetResolution<2>);
  add("cosmic_set_4k",SetResolution<3>);
  add("cosmic_set_30",SetVideo<-1,30>);
  add("cosmic_set_60",SetVideo<-1,60>);
  add("cosmic_set_unlimited",SetVideo<-1,0>);
  add("cosmic_overlay_on",SetPerformanceOverlay<true>);
  add("cosmic_overlay_off",SetPerformanceOverlay<false>);
  add("cosmic_windowed",SetDisplayMode<0>);
  add("cosmic_borderless",SetDisplayMode<1>);
  add("cosmic_exclusive",SetDisplayMode<2>);
  add("cosmic_wipe_progress",WipeProgress);
  add("cosmic_wipe_prompt",WipePrompt);
  {std::ifstream settings(host->user_data_root()/"pc-video.ini");std::string line;
    while(std::getline(settings,line)){if(!line.empty()&&line.back()=='\r')line.pop_back();
      if(line=="performance_overlay=off")ben10::performance_visible=false;
      if(line=="performance_overlay=on")ben10::performance_visible=true;}}
  {std::ifstream settings(host->user_data_root()/"pc-gameplay.ini");std::string line;
    while(std::getline(settings,line)) {
      if(!line.empty() && line.back()=='\r')line.pop_back();
      if(line=="all_aliens=on")all_aliens_enabled=true;
      if(line=="all_aliens=off")all_aliens_enabled=false;
      if(HeroTimeDiagnostic() && line=="hero_time=on")hero_time_enabled=true;
      if(line=="hero_time=off")hero_time_enabled=false;
    }}
  add("cosmic_all_aliens_page",AllAliensPage);
  add("cosmic_toggle_all_aliens",ToggleAllAliens);
  add("cosmic_all_aliens_enabled",AllAliensEnabled);
  add("cosmic_all_aliens_trace",AllAliensTrace);
  if(HeroTimeDiagnostic()) {
    add("cosmic_hero_enabled",HeroTimeEnabled);
    add("cosmic_toggle_hero_time",ToggleHeroTime);
    add("cosmic_hero_button",HeroTimeButton);
    add("cosmic_hero_delta",HeroTimeDelta);
    add("cosmic_hero_loaded",HeroTimeTrace<0>);
    add("cosmic_hero_active",HeroTimeTrace<1>);
    add("cosmic_hero_return",HeroTimeTrace<2>);
    add("cosmic_hero_recharge",HeroTimeTrace<3>);
    add("cosmic_hero_ready",HeroTimeTrace<4>);
  }
}
}

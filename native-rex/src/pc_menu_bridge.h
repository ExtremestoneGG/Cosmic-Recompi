#pragma once
// Private integration probe. Enabled only with BEN10_PC_MENU_DIAGNOSTIC=1.
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
#include <Windows.h>
#include "pc_video_menu.h"
#include "all_aliens_paging.h"
#include <rex/input/input_system.h>

namespace ben10_pc_menu {
inline std::atomic<rex::Runtime*> runtime{nullptr};
inline std::atomic<rex::ui::Window*> window{nullptr};
inline std::atomic<bool> registered{false};
inline ben10::AlienPages alien_pages;
inline bool all_aliens_enabled=false;
inline void ReturnVoid(PPCContext& ctx,uint8_t* base);
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
    auto path=host->user_data_root()/"pc-gameplay.ini",temporary=path;
    temporary+=L".tmp";
    {std::ofstream file(temporary,std::ios::binary|std::ios::trunc);
      file<<"all_aliens="<<(next?"on":"off")<<"\n";file.flush();
      if(!file)throw std::runtime_error("Could not write gameplay settings");}
    if(!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
      throw std::runtime_error("Could not commit gameplay settings");
    all_aliens_enabled=next;
    REXLOG_INFO("Cosmic Gameplay: All Aliens {}",next?"ON":"OFF");
  }catch(const std::exception& e){REXLOG_ERROR("Cosmic Gameplay settings: {}",e.what());}
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
template<int Resolution,int Rate> void SetVideo(PPCContext& ctx,uint8_t* base) {
  ReturnVoid(ctx,base);
  auto* host=runtime.load();if(!host)return;
  try {
    auto path=host->user_data_root()/"pc-video.ini";
    std::string resolution="720p",rate="30",line;
    {std::ifstream file(path);while(std::getline(file,line)) {
      if(!line.empty() && line.back()=='\r')line.pop_back();
      if(line=="resolution=1080p")resolution="1080p";
      if(line=="resolution=1440p")resolution="1440p";
      if(line=="resolution=4k")resolution="4k";
      if(line=="frame_limit=60")rate="60";
      if(line=="frame_limit=unlimited")rate="unlimited";
    }}
    if constexpr(Resolution>=0){const char* choices[]={"720p","1080p","1440p","4k"};resolution=choices[Resolution];}
    if constexpr(Rate>=0)rate=Rate==0?"unlimited":Rate==60?"60":"30";
    std::filesystem::create_directories(path.parent_path());auto temporary=path;temporary+=L".native.tmp";
    {std::ofstream file(temporary,std::ios::binary|std::ios::trunc);file<<"resolution="<<resolution<<"\nframe_limit="<<rate<<"\n";file.flush();if(!file)throw std::runtime_error("Could not write video settings");}
    if(!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Could not commit video settings");
    if constexpr(Rate>=0)if(!ben10_pacing_probe::pacer.SetRate(Rate))throw std::runtime_error("Could not apply frame limit");
    REXLOG_INFO("Cosmic native VIDEO: resolution={}, frame_limit={}, saved=true",resolution,rate);
  }catch(const std::exception& error){REXLOG_ERROR("Cosmic native VIDEO failed: {}",error.what());}
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
  add("cosmic_set_720",SetVideo<0,-1>);
  add("cosmic_set_1080",SetVideo<1,-1>);
  add("cosmic_set_1440",SetVideo<2,-1>);
  add("cosmic_set_4k",SetVideo<3,-1>);
  add("cosmic_set_30",SetVideo<-1,30>);
  add("cosmic_set_60",SetVideo<-1,60>);
  add("cosmic_set_unlimited",SetVideo<-1,0>);
  {std::ifstream settings(host->user_data_root()/"pc-gameplay.ini");std::string line;
    while(std::getline(settings,line)) {
      if(!line.empty() && line.back()=='\r')line.pop_back();
      if(line=="all_aliens=on")all_aliens_enabled=true;
      if(line=="all_aliens=off")all_aliens_enabled=false;
    }}
  add("cosmic_all_aliens_page",AllAliensPage);
  add("cosmic_toggle_all_aliens",ToggleAllAliens);
  add("cosmic_all_aliens_enabled",AllAliensEnabled);
  add("cosmic_all_aliens_trace",AllAliensTrace);
}
}

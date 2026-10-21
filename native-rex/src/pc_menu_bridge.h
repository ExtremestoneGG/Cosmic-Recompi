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

namespace ben10_pc_menu {
inline std::atomic<rex::Runtime*> runtime{nullptr};
inline std::atomic<rex::ui::Window*> window{nullptr};
inline std::atomic<bool> registered{false};
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
inline void OpenVideo(PPCContext& ctx,uint8_t* base) {
  REXLOG_INFO("Cosmic PC menu: VIDEO selected");
  ReturnVoid(ctx,base);
  if(!ben10_pc_video::closed_event)return;
  ResetEvent(ben10_pc_video::closed_event);
  ben10_pc_video::opening_guard=true;ben10_pc_video::visible=true;
  if(auto* current=window.load())current->app_context().CallInUIThreadDeferred([] {
    if(auto* current=window.load())current->RequestPresenterUIPaintFromUIThread();
  });
  // Keep the guest menu callback alive while the SDK renders the modal dialog.
  // The authored wrapper restores the original Options menu after this returns.
  WaitForSingleObject(ben10_pc_video::closed_event,INFINITE);
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
  add("cosmic_video_options",OpenVideo);
}
}

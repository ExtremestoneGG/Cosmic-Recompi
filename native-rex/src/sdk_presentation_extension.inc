// Compiled into the SDK's existing SDL window translation unit. Uses that
// runtime's SDL instance; linking a second SDL into the consumer cannot safely
// enumerate windows created by the runtime DLL.
extern "C" __declspec(dllexport) int Ben10ConfigurePresentation(
    void* native_window,int mode,int width,int height,char* error,size_t capacity) {
  auto fail=[&](const char* message) {
    if(error && capacity)SDL_strlcpy(error,message,capacity);
    return 0;
  };
  if(mode<0 || mode>2 || width<1280 || height<720 || width>3840 || height>2160)
    return fail("Unsupported presentation settings");
  int count=0;SDL_Window** windows=SDL_GetWindows(&count);SDL_Window* target=nullptr;
  if(!windows)return fail(SDL_GetError());
  for(int i=0;i<count;i++) {
    void* handle=SDL_GetPointerProperty(SDL_GetWindowProperties(windows[i]),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER,nullptr);
    if(handle==native_window){target=windows[i];break;}
  }
  SDL_free(windows);
  if(!target)return fail("The SDK does not own this window");
  SDL_DisplayMode exclusive{};
  if(mode==2) {
    if(!SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(target),
        width,height,0.0f,true,&exclusive))return fail(SDL_GetError());
    if(exclusive.w!=width || exclusive.h!=height)
      return fail("This monitor does not support the selected fullscreen resolution");
  }
  // The consumer has already left fullscreen through Window::SetFullscreen,
  // keeping the SDK's desired state and presenter callbacks synchronized.
  if(!SDL_SetWindowFullscreenMode(target,mode==2?&exclusive:nullptr))return fail(SDL_GetError());
  if(mode==0) {
    const float density=SDL_GetWindowPixelDensity(target);
    if(density<=0.0f)return fail("Invalid window pixel density");
    if(SDL_GetWindowFlags(target)&SDL_WINDOW_MAXIMIZED)
      if(!SDL_RestoreWindow(target))return fail(SDL_GetError());
    if(!SDL_SetWindowSize(target,int(std::lround(width/density)),
        int(std::lround(height/density))))return fail(SDL_GetError());
  }
  // Check SDL's actual request result rather than relying only on the SDK
  // Window's desired fullscreen flag (its existing setter returns void).
  if(!SDL_SetWindowFullscreen(target,mode!=0))return fail(SDL_GetError());
  return 1;
}

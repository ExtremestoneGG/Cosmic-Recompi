// Cosmic-specific host for the same Dear ImGui / SDL / OpenGL stack as Full Emerald.
// Layout inspired by its floating menu; animation loader reused under MIT.
#include <windows.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"
#include "common/launcher_gl.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <charconv>

namespace fs = std::filesystem;
static fs::path root;
static std::string read(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(file), {});
}
static fs::path installation() {
    std::string chosen = read(root / "installation-path.txt");
    while (!chosen.empty() && (chosen.back() == '\r' || chosen.back() == '\n')) chosen.pop_back();
    return chosen.empty() ? root / "installation" : fs::u8path(chosen);
}
static bool ready() {
    auto folder = installation();
    return fs::exists(folder / "installed.txt") && fs::exists(folder / "game/default.xex") && fs::exists(root / "runtime/ben10.exe");
}
static PROCESS_INFORMATION worker{};
static bool game = false;
static std::string status = "Select your own ISO in Settings to get started.";
static bool spawn(const wchar_t* action) {
    std::wstring command = L"\"" + (root / "CosmicBridge.exe").wstring() + L"\" " + action;
    if(std::wstring(action)==L"play")command+=L" --parent "+std::to_wstring(GetCurrentProcessId());
    STARTUPINFOW startup{}; startup.cb = sizeof(startup);
    if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, 0, nullptr, root.c_str(), &startup, &worker)) {
        status = "Could not start CosmicBridge.exe. Extract the complete package."; return false;
    }
    CloseHandle(worker.hThread); worker.hThread = nullptr;
    game = std::wstring(action) == L"play"; return true;
}
static bool nav(const char* text, float width, bool selected = false) {
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::PushID(text);
    bool clicked = ImGui::InvisibleButton("##navigation", ImVec2(width, 48), ImGuiButtonFlags_EnableNav);
    if (ImGui::IsItemHovered() || ImGui::IsItemFocused() || selected) {
        auto* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilledMultiColor(pos, ImVec2(pos.x + width, pos.y + 48),
            IM_COL32(52,190,133,175),IM_COL32(52,190,133,0),
            IM_COL32(52,190,133,0),IM_COL32(52,190,133,175));
    }
    ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x+16,pos.y+(48-ImGui::GetTextLineHeight())*.5f),
        ImGui::GetColorU32(ImGuiCol_Text),text);
    ImGui::PopID();
    return clicked;
}
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR args, int) {
    wchar_t module[32768]; GetModuleFileNameW(nullptr, module, 32768);
    root = fs::path(module).parent_path();
    bool smoke = std::wstring(args).find(L"--smoke-test") != std::wstring::npos;
    // A process-scoped mutex protects preferences and import status from races.
    std::wstring mutexName = L"Local\\CosmicRecompNative-" + std::to_wstring(std::hash<std::wstring>{}(root.wstring()));
    HANDLE instance = CreateMutexW(nullptr, TRUE, mutexName.c_str());
    if (!instance || GetLastError() == ERROR_ALREADY_EXISTS) { if (instance) CloseHandle(instance); return 0; }
    fs::current_path(root);
    fs::create_directories(root/"temp");
    SetEnvironmentVariableW(L"TEMP",(root/"temp").c_str());
    SetEnvironmentVariableW(L"TMP",(root/"temp").c_str());
    // Always show the launcher. Legacy direct-start preferences are ignored.
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) { MessageBoxA(nullptr, SDL_GetError(), "Cosmic Recomp", MB_ICONERROR); CloseHandle(instance); return 1; }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_Window* window = SDL_CreateWindow("Cosmic Recomp", 1100, 840, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | (smoke ? SDL_WINDOW_HIDDEN : 0));
    if (!window) { MessageBoxA(nullptr, SDL_GetError(), "Cosmic Recomp", MB_ICONERROR); SDL_Quit(); CloseHandle(instance); return 1; }
    SDL_SetWindowMinimumSize(window, 680, 520);
    SDL_Rect usable{};
    if(SDL_GetDisplayUsableBounds(SDL_GetDisplayForWindow(window),&usable))
        SDL_SetWindowSize(window,std::min(1100,std::max(680,usable.w-60)),std::min(840,std::max(520,usable.h-60)));
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) { MessageBoxA(nullptr, SDL_GetError(), "Cosmic Recomp", MB_ICONERROR); SDL_DestroyWindow(window); SDL_Quit(); CloseHandle(instance); return 1; }
    SDL_GL_MakeCurrent(window, context); SDL_GL_SetSwapInterval(1);
    ImGui::CreateContext(); auto& io = ImGui::GetIO();
    io.IniFilename = nullptr; io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    // Load the OS font read-only; all writable state remains beside the launcher.
    wchar_t windows[MAX_PATH]; GetWindowsDirectoryW(windows, MAX_PATH);
    fs::path font = fs::path(windows) / "Fonts/segoeui.ttf";
    io.Fonts->AddFontFromFileTTF(font.u8string().c_str(), 19);
    ImFont* heading = io.Fonts->AddFontFromFileTTF(font.u8string().c_str(), 48);
    ImFont* subtitle = io.Fonts->AddFontFromFileTTF(font.u8string().c_str(), 32);
    ImGui::StyleColorsDark(); auto& style = ImGui::GetStyle();
    style.WindowRounding = 0; style.FrameRounding = 6; style.FramePadding = ImVec2(16, 12);
    style.ItemSpacing = ImVec2(12, 12); style.Colors[ImGuiCol_Header] = ImVec4(.13f,.65f,.44f,.35f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(.23f,.85f,.60f,.65f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(.23f,.85f,.60f,.85f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(.27f,.95f,.67f,1);
    ImGui_ImplSDL3_InitForOpenGL(window, context); ImGui_ImplOpenGL3_Init("#version 130");
    auto background = launcher_animation_load_gif("assets/launcher/background.gif");
    bool quit = false, settings = std::wstring(args).find(L"--settings") != std::wstring::npos; int frames = 0;
    int resolution = 0, rate = 0, displayMode=0;bool performanceOverlay=true;
    {
        std::ifstream file(installation()/"user/pc-video.ini"); std::string line;
        while(std::getline(file,line)) {
            if(!line.empty() && line.back()=='\r')line.pop_back();
            if(line=="resolution=1080p")resolution=1;
            if(line=="resolution=1440p")resolution=2;
            if(line=="resolution=4k")resolution=3;
            if(line=="frame_limit=60")rate=1;
            if(line=="frame_limit=unlimited")rate=2;
            if(line=="performance_overlay=off")performanceOverlay=false;
            if(line=="performance_overlay=on")performanceOverlay=true;
            if(line=="display_mode=borderless")displayMode=1;
            if(line=="display_mode=exclusive")displayMode=2;
        }
    }
    Uint64 nextStatusRead=0;
    if(ready())status="Ready to play.";
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if ((event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) && !worker.hProcess) quit = true;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) settings = false;
        }
        if (worker.hProcess && SDL_GetTicks()>=nextStatusRead) {
            nextStatusRead=SDL_GetTicks()+250;
            DWORD code;
            if (GetExitCodeProcess(worker.hProcess, &code) && code != STILL_ACTIVE) {
                CloseHandle(worker.hProcess); worker = {}; game = false;
                SDL_ShowWindow(window); SDL_RaiseWindow(window);
            }
            auto update = read(root / "launcher-status.txt"); if (!update.empty()) status = update;
        }
        if (game) { SDL_Delay(50); continue; } // No UI rendering/animation cost during gameplay.
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplSDL3_NewFrame(); ImGui::NewFrame();
        auto* viewport = ImGui::GetMainViewport(); ImVec2 size = viewport->Size;
        auto* draw = ImGui::GetBackgroundDrawList();
        draw->AddRectFilled(ImVec2(0,0), size, IM_COL32(5,22,18,255));
        if (background.id) {
            launcher_animation_update(&background, SDL_GetTicks());
            float source = float(background.w) / background.h, target = size.x / size.y;
            ImVec2 uv0(0,0), uv1(1,1);
            if (source > target) { uv0.x = (1-target/source)*.5f; uv1.x = 1-uv0.x; }
            else { uv0.y = (1-source/target)*.5f; uv1.y = 1-uv0.y; }
            draw->AddImage((ImTextureID)(intptr_t)background.id, ImVec2(0,0), size, uv0, uv1);
        } else {
            draw->AddRectFilledMultiColor(ImVec2(0,0), size, IM_COL32(10,31,27,255), IM_COL32(27,72,55,255), IM_COL32(5,23,16,255), IM_COL32(2,14,12,255));
        }
        draw->AddRectFilledMultiColor(ImVec2(0,0), size, IM_COL32(0,10,8,190), IM_COL32(0,10,8,65), IM_COL32(0,10,8,65), IM_COL32(0,10,8,190));
        ImGui::SetNextWindowPos(ImVec2(0,0)); ImGui::SetNextWindowSize(size);
        ImGui::Begin("Cosmic", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        float left = size.x < 760 ? 34 : 76;
        ImGui::SetCursorPos(ImVec2(left, 50)); ImGui::PushFont(heading);
        ImGui::TextColored(ImVec4(.52f,1,.69f,1), "COSMIC"); ImGui::PopFont();
        ImGui::PushFont(subtitle); ImGui::SetCursorPosX(left); ImGui::TextUnformatted("RECOMP"); ImGui::PopFont();
        const float menuWidth=std::min(380.f,size.x-left*2);
        const float menuTop=settings?std::max(185.f,size.y*.26f):std::max(210.f,size.y*.39f);
        ImGui::SetCursorPos(ImVec2(left,menuTop));
        ImGui::BeginChild("menu", ImVec2(settings?size.x-left*2:menuWidth, std::max(80.f,size.y-menuTop-148)), ImGuiChildFlags_None);
        if (!settings) {
            ImGui::BeginDisabled(worker.hProcess != nullptr);
            if (nav("Play", ImGui::GetContentRegionAvail().x)) {
                if (!ready()) { settings = true; status = "Select and validate your own ISO to get started."; }
                else if (spawn(L"play")) {
                    // The independent bridge owns the game's lifetime and lock.
                    // Destroy this frontend completely rather than keeping a hidden renderer.
                    CloseHandle(worker.hProcess); worker = {}; game = false; quit = true;
                }
            }
            ImGui::SetItemDefaultFocus();
            if (nav("Settings", ImGui::GetContentRegionAvail().x)) settings = true;
            ImGui::Dummy(ImVec2(0,18)); if (nav("Quit", ImGui::GetContentRegionAvail().x)) quit = true;
            ImGui::EndDisabled();
        } else {
            ImGui::BeginDisabled(worker.hProcess != nullptr);
            ImGui::TextUnformatted("GAME SETUP");
            if (ImGui::Button("Select ISO")) spawn(L"select-iso");
            ImGui::BeginDisabled(ready() || !fs::exists(root/"selected-iso.txt"));
            ImGui::SameLine(); if(ImGui::Button("Prepare game"))spawn(L"import");
            ImGui::EndDisabled();
            ImGui::TextWrapped("Select ISO, wait for validation, then prepare your game. Original game files stay on this computer.");
            ImGui::Separator(); ImGui::TextUnformatted("VIDEO");
            ImGui::SetNextItemWidth(-1);
            ImGui::Combo("##resolution",&resolution,"1280 x 720 (native)\0" "1920 x 1080\0" "2560 x 1440\0" "3840 x 2160\0");
            ImGui::SetNextItemWidth(-1);
            ImGui::Combo("##frame-limit",&rate,"30 FPS\0" "60 FPS\0" "Unlimited\0");
            ImGui::SetNextItemWidth(-1);
            ImGui::Combo("##display-mode",&displayMode,"Windowed\0" "Borderless fullscreen\0" "Exclusive fullscreen\0");
            ImGui::Checkbox("Show FPS and frame time",&performanceOverlay);
            ImGui::TextWrapped("Applies when the game starts. 60 FPS is a limit, not a performance guarantee.");
            if(ImGui::Button("Save video settings")) {
                const char* resolutions[]={"720p","1080p","1440p","4k"};
                auto path=installation()/"user/pc-video.ini";
                std::error_code error; fs::create_directories(path.parent_path(),error);
                bool saved=false;
                if(!error) {
                    auto temporary=path;temporary+=L".launcher.tmp";
                    {std::ofstream file(temporary,std::ios::binary|std::ios::trunc);
                     file<<"resolution="<<resolutions[resolution]<<"\nframe_limit="<<(rate==2?"unlimited":rate==1?"60":"30")<<"\n";
                     file<<"performance_overlay="<<(performanceOverlay?"on":"off")<<"\n";
                     file<<"display_mode="<<(displayMode==2?"exclusive":displayMode==1?"borderless":"windowed")<<"\n";
                     file.flush();saved=bool(file);}
                    if(saved)saved=MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
                }
                status=saved?"Video settings saved.":"Could not save video settings. Check folder permissions.";
            }
            ImGui::Separator();
            ImGui::TextUnformatted("PATCHES");
            ImGui::BeginDisabled(!ready());
            if(ImGui::Button("Add translation patch"))spawn(L"patch");
            ImGui::EndDisabled();
            ImGui::TextWrapped("Select the patch's global/scripts/xbx_us.txt file. Patches are supplied separately by their authors.");
            ImGui::Separator();
            ImGui::TextWrapped("Save folder: %s",(installation()/"user").u8string().c_str());
            ImGui::TextWrapped("Keep this folder when updating. To reset saves, close the game and rename user to user-backup.");
            if(ImGui::Button("Open saves"))spawn(L"saves");
            ImGui::SameLine(); if(ImGui::Button("Back")) settings=false;
            ImGui::EndDisabled();
        }
        ImGui::EndChild();
        ImGui::SetCursorPos(ImVec2(30, size.y-132));
        if(status.rfind("Preparing: ",0)==0) {
            int percent=0;std::from_chars(status.data()+11,status.data()+status.size(),percent);
            ImGui::ProgressBar(std::clamp(percent,0,100)/100.f,ImVec2(size.x-60,16));
        }
        ImGui::SetCursorPos(ImVec2(30,size.y-110));
        ImGui::BeginChild("status",ImVec2(size.x-60,48),ImGuiChildFlags_None);
        ImGui::PushTextWrapPos(0);
        ImGui::TextColored(ImVec4(.72f,.82f,.77f,1), "%s", status.c_str()); ImGui::PopTextWrapPos();
        ImGui::EndChild();
        ImGui::End(); ImGui::Render();
        int width, height; SDL_GetWindowSizeInPixels(window, &width, &height);
        glViewport(0,0,width,height); glClearColor(.02f,.05f,.04f,1); glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (smoke && ++frames == 3) {
            std::vector<unsigned char> pixels(size_t(width)*height*4); glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
            std::vector<unsigned char> flipped(pixels.size());
            for (int y=0;y<height;y++) std::copy_n(pixels.data()+size_t(height-1-y)*width*4,size_t(width)*4,flipped.data()+size_t(y)*width*4);
            SDL_Surface* surface = SDL_CreateSurfaceFrom(width,height,SDL_PIXELFORMAT_RGBA32,flipped.data(),width*4);
            if (surface) { SDL_SaveBMP(surface,"launcher-preview.bmp"); SDL_DestroySurface(surface); } quit = true;
        }
        SDL_GL_SwapWindow(window); SDL_Delay(8);
    }
    launcher_animation_free(&background); ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplSDL3_Shutdown(); ImGui::DestroyContext();
    SDL_GL_DestroyContext(context); SDL_DestroyWindow(window); SDL_Quit(); ReleaseMutex(instance); CloseHandle(instance); return 0;
}

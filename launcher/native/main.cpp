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
    bool clicked = ImGui::Selectable(text, selected, ImGuiSelectableFlags_None, ImVec2(width, 48));
    if (ImGui::IsItemHovered() || ImGui::IsItemFocused() || selected) {
        auto* draw = ImGui::GetWindowDrawList();
        draw->AddRect(pos, ImVec2(pos.x + width, pos.y + 48), IM_COL32(87, 241, 179, 255), 6, 0, 1.5f);
    }
    return clicked;
}
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR args, int) {
    wchar_t module[32768]; GetModuleFileNameW(nullptr, module, 32768);
    root = fs::path(module).parent_path();
    bool smoke = std::wstring(args).find(L"--smoke-test") != std::wstring::npos;
    bool forceLauncher = std::wstring(args).find(L"--launcher") != std::wstring::npos;
    // A process-scoped mutex protects preferences and import status from races.
    std::wstring mutexName = L"Local\\CosmicRecompNative-" + std::to_wstring(std::hash<std::wstring>{}(root.wstring()));
    HANDLE instance = CreateMutexW(nullptr, TRUE, mutexName.c_str());
    if (!instance || GetLastError() == ERROR_ALREADY_EXISTS) { if (instance) CloseHandle(instance); return 0; }
    fs::current_path(root);
    fs::create_directories(root/"temp");
    SetEnvironmentVariableW(L"TEMP",(root/"temp").c_str());
    SetEnvironmentVariableW(L"TMP",(root/"temp").c_str());
    std::string preference = read(root / "launcher-preferences.ini");
    while (!preference.empty() && (preference.back() == '\r' || preference.back() == '\n')) preference.pop_back();
    bool direct = preference == "start_directly=1";
    // Direct boot does not create a window, graphics context or decode artwork.
    if (direct && ready() && !forceLauncher && !smoke) {
        bool started = spawn(L"play");
        if (started) CloseHandle(worker.hProcess);
        ReleaseMutex(instance); CloseHandle(instance); return started ? 0 : 1;
    }
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
    bool quit = false, settings = false; int frames = 0;
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
        const float menuWidth=settings?std::min(320.f,size.x*.42f):std::min(380.f,size.x-left*2);
        const float menuTop=settings?std::max(185.f,size.y*.26f):std::max(210.f,size.y*.39f);
        ImGui::SetCursorPos(ImVec2(left,menuTop));
        ImGui::BeginChild("menu", ImVec2(menuWidth, size.y*.43f), ImGuiChildFlags_None);
        if (!settings) {
            ImGui::BeginDisabled(worker.hProcess != nullptr);
            if (nav("Play", ImGui::GetContentRegionAvail().x, !io.NavActive)) {
                if (!ready()) { settings = true; status = "Select your ISO and installation folder."; }
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
            if (nav("Select ISO", ImGui::GetContentRegionAvail().x)) spawn(L"import");
            if (nav("Installation folder", ImGui::GetContentRegionAvail().x)) spawn(L"folder");
            if (nav("Open saves", ImGui::GetContentRegionAvail().x)) spawn(L"saves");
            if (nav("Back", ImGui::GetContentRegionAvail().x)) settings = false;
            ImGui::EndDisabled();
        }
        ImGui::EndChild();
        if(settings) {
            ImGui::SetCursorPos(ImVec2(left+menuWidth+24,menuTop));
            ImGui::PushTextWrapPos(size.x-30);
            const std::string saves=(installation()/"user").u8string();
            ImGui::TextWrapped("Save folder: %s",saves.c_str());
            ImGui::TextDisabled("Keep this folder when updating.");
            ImGui::TextWrapped("To reset: close the game, then rename user to user-backup.");
            ImGui::PopTextWrapPos();
        }
        ImGui::SetCursorPos(ImVec2(30, size.y-110)); ImGui::PushTextWrapPos(size.x-30);
        ImGui::TextColored(ImVec4(.72f,.82f,.77f,1), "%s", status.c_str()); ImGui::PopTextWrapPos();
        ImGui::SetCursorPos(ImVec2(30,size.y-54));
        if (ImGui::Checkbox("Start directly in game", &direct)) {
            std::ofstream preference(root / "launcher-preferences.ini", std::ios::binary | std::ios::trunc);
            preference << (direct ? "start_directly=1\n" : "start_directly=0\n");
            if (!preference) { direct = false; status = "Could not save the preference. Check folder permissions."; }
        }
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

cmake_minimum_required(VERSION 3.24)
project(CosmicRecompLauncher LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(RUI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/full-emerald-ui" CACHE PATH "Pinned Full Emerald recomp-ui checkout")
# The game SDK's SDL intentionally excludes OpenGL. Build an isolated UI copy;
# never overwrite the SDK's libraries or link the launcher into the game.
set(COSMIC_SDL_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/rexglue-sdk/thirdparty/sdl3" CACHE PATH "SDL3 source for the standalone launcher")
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_OPENGL ON CACHE BOOL "" FORCE)
set(SDL_OPENGLES OFF CACHE BOOL "" FORCE)
set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
set(SDL_GPU OFF CACHE BOOL "" FORCE)
set(SDL_AUDIO OFF CACHE BOOL "" FORCE)
set(SDL_RENDER OFF CACHE BOOL "" FORCE)
add_subdirectory("${COSMIC_SDL_SOURCE}" "sdl-ui" EXCLUDE_FROM_ALL)
set(IMGUI "${RUI_ROOT}/src/third_party/imgui")
add_executable(CosmicRecomp WIN32 main.cpp
  "${IMGUI}/imgui.cpp" "${IMGUI}/imgui_draw.cpp"
  "${IMGUI}/imgui_tables.cpp" "${IMGUI}/imgui_widgets.cpp"
  "${IMGUI}/backends/imgui_impl_sdl3.cpp"
  "${IMGUI}/backends/imgui_impl_opengl3.cpp"
  "${RUI_ROOT}/src/common/launcher_gl.c")
target_include_directories(CosmicRecomp PRIVATE "${IMGUI}" "${RUI_ROOT}/src")
target_compile_definitions(CosmicRecomp PRIVATE LNG_SDL3=1 SDL_MAIN_HANDLED UNICODE _UNICODE NOMINMAX)
target_link_libraries(CosmicRecomp PRIVATE SDL3::SDL3 opengl32)

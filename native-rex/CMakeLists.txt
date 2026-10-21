# ben10 - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(ben10 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(BEN10_SOURCES
    src/main.cpp
    src/frame_metrics.cpp
)

if(WIN32)
    add_executable(ben10 WIN32 ${BEN10_SOURCES})
else()
    add_executable(ben10 ${BEN10_SOURCES})
endif()

rexglue_setup_target(ben10 GPU_PLUGINS xenos)
target_compile_options(ben10 PRIVATE /EHsc)
target_compile_options(ben10_recomp PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/frame_metrics.h")

# Optional isolated equivalence target. It reuses the original-byte oracle
# corpus; it is deliberately separate from gameplay compatibility checks.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../build/rex-leaf-tests/main.cpp")
    add_executable(ben10_leaf_tests EXCLUDE_FROM_ALL
        ../build/rex-leaf-tests/main.cpp ../build/rex-leaf-tests/selected.cpp)
    target_include_directories(ben10_leaf_tests PRIVATE generated/default)
    target_link_libraries(ben10_leaf_tests PRIVATE rex::runtime)
    rexglue_apply_target_settings(ben10_leaf_tests)
endif()

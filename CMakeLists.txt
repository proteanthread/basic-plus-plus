# =====================================================================
# BASIC++ v6.5.0 - CMake Build System Configuration
# =====================================================================
#
# SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
# - What it does: Configures targets, compilation standards, compile defines,
#   and includes subdirectories for compiler and testing builds.
# - Why it exists: Establishes a portable build system for MSVC and GCC.
#   The engine lives in engine/ with directory-organized headers.
# - Why it works this way: It enforces strict C17 standards (-std=c17)
#   and disables extensions. It builds the engine as a sub-project
#   under engine/ which defines libbasicpp and all micro-libraries.
#
# SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
# - What can be changed: Compiler flags, default target profiles, new libs.
# - What cannot be changed: C17 compliance, directory-organized includes.
# - What to expect: CMake generates native projects in build directories.
# - What to do if something breaks: Check engine/CMakeLists.txt for
#   missing source files or include path issues.
#
# =====================================================================

cmake_minimum_required(VERSION 3.16)
project(basicpp VERSION 6.5.0 LANGUAGES C CXX)

# Strict C17 Compliance
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF) # Disable GNU and MSVC language extensions

if(NOT MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build_linux/Release)
endif()

# Enforce compiler warnings for type-safety and standard conformance
if(MSVC)
    add_compile_options(/W4 /WX /wd4100) # Wall, treat warnings as errors, disable unused param
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    if(ENABLE_ASAN)
        add_compile_options(/fsanitize=address /Zi /wd5072)
        add_link_options(/DEBUG)
    endif()
else()
    add_compile_options(-Wall -Wextra -Werror -Wno-unused-parameter)
endif()

# Global include directories - NEW directory-organized headers
include_directories(engine/include)

option(BASIC_USE_CUSTOM_STATIC_DIALECT "Enable custom static compile-time dialect for blite/bscript" OFF)
if(BASIC_USE_CUSTOM_STATIC_DIALECT)
    add_compile_definitions(BASIC_USE_CUSTOM_STATIC_DIALECT)
endif()

# Add the engine subdirectory (replaces old src/ subdirectory)
add_subdirectory(engine)

# SDL2 configuration
if(MSVC)
    include_directories(${CMAKE_SOURCE_DIR}/sdl2/include)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        link_directories(${CMAKE_SOURCE_DIR}/sdl2/lib/x64)
    else()
        link_directories(${CMAKE_SOURCE_DIR}/sdl2/lib/x86)
    endif()
    set(SDL2_LIBRARIES SDL2 SDL2main Shell32 User32 Gdi32 Winmm)
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(SDL2 REQUIRED sdl2)
    include_directories(${SDL2_INCLUDE_DIRS})
    link_directories(${SDL2_LIBRARY_DIRS})
    set(SDL2_LIBRARIES ${SDL2_LIBRARIES})
endif()

# --- Binary Target 1: baspp (Standard Console & SDL Combined) ---
add_executable(baspp engine/src/bootstrap/main_baspp.c engine/src/device/vdev_gfx.c)
target_include_directories(baspp PRIVATE engine/include engine/src/device/bgi/aalib/include)
target_compile_definitions(baspp PRIVATE BASIC_SDL_BUILD BGI_HAVE_AALIB)
if(NOT MSVC)
    target_link_libraries(baspp PRIVATE
        -Wl,--whole-archive libbasicpp -Wl,--no-whole-archive ${SDL2_LIBRARIES})
else()
    target_link_libraries(baspp PRIVATE libbasicpp ${SDL2_LIBRARIES})
endif()

# --- Binary Target 2: blite (Headless Lite Edition REPL) ---
add_executable(blite engine/src/bootstrap/main_blite.c)
target_include_directories(blite PRIVATE engine/include)
target_compile_definitions(blite PRIVATE BASIC_LITE_BUILD NO_SDL2)
if(NOT MSVC)
    target_link_libraries(blite PRIVATE
        -Wl,--whole-archive libbasicpp_lite -Wl,--no-whole-archive)
else()
    target_link_libraries(blite PRIVATE libbasicpp_lite)
endif()

# --- Binary Target 3: bscript (Headless Lite Script Runner) ---
add_executable(bscript engine/src/bootstrap/main_bscript.c)
target_include_directories(bscript PRIVATE engine/include)
target_compile_definitions(bscript PRIVATE BASIC_LITE_BUILD NO_SDL2)
if(NOT MSVC)
    target_link_libraries(bscript PRIVATE
        -Wl,--whole-archive libbasicpp_lite -Wl,--no-whole-archive)
else()
    target_link_libraries(bscript PRIVATE libbasicpp_lite)
endif()

# --- Binary Target 4: bppc (Transpiler/Compiler Tool) ---
add_executable(bppc engine/src/tools/bppc.c)
target_include_directories(bppc PRIVATE engine/include)
if(NOT MSVC)
    target_link_libraries(bppc PRIVATE
        -Wl,--whole-archive libbasicpp -Wl,--no-whole-archive)
else()
    target_link_libraries(bppc PRIVATE libbasicpp)
endif()

# --- Binary Target 5: detok (GW-BASIC Detokenizer Tool) ---
add_executable(detok engine/src/tools/detok.c)
target_include_directories(detok PRIVATE engine/include)
if(NOT MSVC)
    target_link_libraries(detok PRIVATE
        -Wl,--whole-archive libbasicpp -Wl,--no-whole-archive)
else()
    target_link_libraries(detok PRIVATE libbasicpp)
endif()

# --- Binary Target 6: basstub (Headless Standalone Runner Stub) ---
add_executable(basstub engine/src/bootstrap/basstub.c)
target_include_directories(basstub PRIVATE engine/include)
target_compile_definitions(basstub PRIVATE BASIC_LITE_BUILD NO_SDL2)
if(NOT MSVC)
    target_link_libraries(basstub PRIVATE
        -Wl,--whole-archive libbasicpp_lite -Wl,--no-whole-archive)
else()
    target_link_libraries(basstub PRIVATE libbasicpp_lite)
endif()

# Size Optimization configurations for Lite targets
if(MSVC)
    target_compile_options(libbasicpp_lite PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(blite PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(bscript PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(basstub PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_link_options(blite PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_link_options(bscript PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_link_options(basstub PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_compile_options(baspp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_link_options(baspp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG>)
else()
    target_compile_options(libbasicpp_lite PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(blite PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(bscript PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(basstub PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_link_options(blite PRIVATE -Os -flto -Wl,--gc-sections)
    target_link_options(bscript PRIVATE -Os -flto -Wl,--gc-sections)
    target_link_options(basstub PRIVATE -Os -flto -Wl,--gc-sections)
endif()

# --- Standalone Text Editor Targets ---
add_executable(vi engine/src/editor/mod_vi.c engine/lib/platform/plat_console.c engine/lib/platform/plat_fs.c engine/lib/platform/plat_sys.c engine/lib/platform/plat_time.c engine/lib/platform/plat_thread.c engine/lib/platform/plat_dl.c engine/lib/platform/plat_net.c engine/lib/platform/plat_regex.c engine/lib/platform/plat_clipboard.c)
target_compile_definitions(vi PRIVATE STANDALONE_EDITOR)
target_include_directories(vi PRIVATE engine/include)
set_target_properties(vi PROPERTIES SUFFIX ".exe")
if(UNIX)
    target_compile_definitions(vi PRIVATE _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(vi PRIVATE pthread dl)
endif()
if(WIN32)
    target_link_libraries(vi PRIVATE ws2_32)
endif()

add_executable(ws engine/src/editor/mod_ws.c engine/lib/platform/plat_console.c engine/lib/platform/plat_fs.c engine/lib/platform/plat_sys.c engine/lib/platform/plat_time.c engine/lib/platform/plat_thread.c engine/lib/platform/plat_dl.c engine/lib/platform/plat_net.c engine/lib/platform/plat_regex.c engine/lib/platform/plat_clipboard.c)
target_compile_definitions(ws PRIVATE STANDALONE_EDITOR)
target_include_directories(ws PRIVATE engine/include)
set_target_properties(ws PROPERTIES SUFFIX ".exe")
if(UNIX)
    target_compile_definitions(ws PRIVATE _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(ws PRIVATE pthread dl)
endif()
if(WIN32)
    target_link_libraries(ws PRIVATE ws2_32)
endif()

add_executable(edit engine/src/editor/mod_edit.c engine/lib/platform/plat_console.c engine/lib/platform/plat_fs.c engine/lib/platform/plat_sys.c engine/lib/platform/plat_time.c engine/lib/platform/plat_thread.c engine/lib/platform/plat_dl.c engine/lib/platform/plat_net.c engine/lib/platform/plat_regex.c engine/lib/platform/plat_clipboard.c)
target_compile_definitions(edit PRIVATE STANDALONE_EDITOR)
target_include_directories(edit PRIVATE engine/include)
set_target_properties(edit PROPERTIES SUFFIX ".exe")
if(UNIX)
    target_compile_definitions(edit PRIVATE _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(edit PRIVATE pthread dl)
endif()
if(WIN32)
    target_link_libraries(edit PRIVATE ws2_32)
endif()

add_executable(edlin engine/src/editor/mod_edlin.c engine/lib/platform/plat_console.c engine/lib/platform/plat_fs.c engine/lib/platform/plat_sys.c engine/lib/platform/plat_time.c engine/lib/platform/plat_thread.c engine/lib/platform/plat_dl.c engine/lib/platform/plat_net.c engine/lib/platform/plat_regex.c engine/lib/platform/plat_clipboard.c)
target_compile_definitions(edlin PRIVATE STANDALONE_EDITOR)
target_include_directories(edlin PRIVATE engine/include)
set_target_properties(edlin PROPERTIES SUFFIX ".exe")
if(UNIX)
    target_compile_definitions(edlin PRIVATE _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(edlin PRIVATE pthread dl)
endif()
if(WIN32)
    target_link_libraries(edlin PRIVATE ws2_32)
endif()

# --- Shared Target: mockbios (Shared Library for Windows/Linux) ---
add_library(mockbios SHARED engine/src/bios/mock_bios.c)
target_include_directories(mockbios PRIVATE engine/include)
set_target_properties(mockbios PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build_linux"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build_linux"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build_linux"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build_linux"
    PREFIX ""
)


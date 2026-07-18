# =====================================================================
# BASIC++ v6.1.0 — CMake Build System Configuration
# =====================================================================
#
# SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
# - What it does: Configures targets, compilation standards, compile defines,
#   and includes subdirectories for compiler and testing builds.
# - Why it exists: Establishes a portable build system replacing the old monolithic
#   Watcom-specific Makefile. Enables native builds on MSVC, gcc, clang, and Watcom.
# - Why it works this way: It enforces strict C17 standards (-std=c17) and disables extensions.
#   It splits the codebase into a core library (libbasicpp) and builds three thin wrappers
#   (blite, baspp-console, and baspp).
#
# SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
# - What can be changed: Compiler flags, default target profiles, addition of libraries/plugins.
# - What cannot be changed: C17 compiler requirement and strict extension disable flags.
# - What to expect: CMake generates native projects (Ninja, Makefiles, MSVC solutions) in a build directory.
# - What to do if something breaks: If compilation fails due to unknown standard flags, ensure the compiler
#   conforms to ISO/IEC 9899:2018 (C17).
#
# =====================================================================

cmake_minimum_required(VERSION 3.16)
project(basicpp VERSION 6.4.0 LANGUAGES C CXX)

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
    add_compile_options(/W4 /WX /wd4100) # Wall, treat warnings as errors, disable unused param warnings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # Enable Address Sanitizer for bug detection
    if(ENABLE_ASAN)
        add_compile_options(/fsanitize=address /Zi /wd5072)
        add_link_options(/DEBUG)
    endif()
else()
    add_compile_options(-Wall -Wextra -Werror -Wno-unused-parameter)
endif()

# Global include directories
include_directories(include)

option(BPP_USE_CUSTOM_STATIC_DIALECT "Enable custom static compile-time dialect for blite/bscript" OFF)
if(BPP_USE_CUSTOM_STATIC_DIALECT)
    add_compile_definitions(BPP_USE_CUSTOM_STATIC_DIALECT)
endif()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests/c_unit)

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

# --- Binary Target 1: blite (Headless Lite Edition REPL) ---
add_executable(blite src/bootstrap/main_blite.c)
target_link_libraries(blite PRIVATE libbasicpp_lite)
target_compile_definitions(blite PRIVATE BPP_LITE_BUILD NO_SDL2)

# --- Binary Target 2: bscript (Headless Lite Script Runner) ---
add_executable(bscript src/bootstrap/main_bscript.c)
target_link_libraries(bscript PRIVATE libbasicpp_lite)
target_compile_definitions(bscript PRIVATE BPP_LITE_BUILD NO_SDL2)

# --- Binary Target 7: basstub (Headless Standalone Runner Stub) ---
add_executable(basstub src/bootstrap/basstub.c)
target_link_libraries(basstub PRIVATE libbasicpp_lite)
target_compile_definitions(basstub PRIVATE BPP_LITE_BUILD NO_SDL2)

# Size Optimization configurations for Lite targets
if(MSVC)
    target_compile_options(libbasicpp_lite PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(blite PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(bscript PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(basstub PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_link_options(blite PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_link_options(bscript PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_link_options(basstub PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
else()
    target_compile_options(libbasicpp_lite PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(blite PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(bscript PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(basstub PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_link_options(blite PRIVATE -Os -flto -Wl,--gc-sections)
    target_link_options(bscript PRIVATE -Os -flto -Wl,--gc-sections)
    target_link_options(basstub PRIVATE -Os -flto -Wl,--gc-sections)
endif()


# --- Binary Target 3: baspp (Standard Console & SDL Combined) ---
add_executable(baspp src/bootstrap/main_baspp.c src/device/vdev_gfx.c)
target_link_libraries(baspp PRIVATE libbasicpp ${SDL2_LIBRARIES})
target_compile_definitions(baspp PRIVATE BPP_SDL_BUILD)

# --- Standalone Text Editor Targets ---
# For '--vi', compile and place 'vi.exe' into the release directory
add_executable(vi src/editor/mod_vi.c src/platform/platform.c)
target_compile_definitions(vi PRIVATE STANDALONE_EDITOR)
set_target_properties(vi PROPERTIES SUFFIX ".exe")
if(UNIX)
    target_compile_definitions(vi PRIVATE _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(vi PRIVATE pthread dl)
endif()
if(WIN32)
    target_link_libraries(vi PRIVATE ws2_32)
endif()

# For '--ws', compile and place 'ws.exe' into the release directory
add_executable(ws src/editor/mod_ws.c src/platform/platform.c)
target_compile_definitions(ws PRIVATE STANDALONE_EDITOR)
set_target_properties(ws PROPERTIES SUFFIX ".exe")
if(UNIX)
    target_compile_definitions(ws PRIVATE _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(ws PRIVATE pthread dl)
endif()
if(WIN32)
    target_link_libraries(ws PRIVATE ws2_32)
endif()

# For '--edit', compile and place 'edit.exe' into the release directory
add_executable(edit src/editor/mod_edit.c src/platform/platform.c)
target_compile_definitions(edit PRIVATE STANDALONE_EDITOR)
set_target_properties(edit PROPERTIES SUFFIX ".exe")
if(UNIX)
    target_compile_definitions(edit PRIVATE _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(edit PRIVATE pthread dl)
endif()
if(WIN32)
    target_link_libraries(edit PRIVATE ws2_32)
endif()

# For '--edlin', compile and place 'edlin.exe' into the release directory
add_executable(edlin src/editor/mod_edlin.c src/platform/platform.c)
target_compile_definitions(edlin PRIVATE STANDALONE_EDITOR)
set_target_properties(edlin PROPERTIES SUFFIX ".exe")
if(UNIX)
    target_compile_definitions(edlin PRIVATE _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(edlin PRIVATE pthread dl)
endif()
if(WIN32)
    target_link_libraries(edlin PRIVATE ws2_32)
endif()

# --- Binary Target 4: bppc (Transpiler/Compiler Tool) ---
add_executable(bppc src/tools/bppc.c)

# --- Binary Target 5: detok (GW-BASIC Detokenizer Tool) ---
add_executable(detok src/tools/detok.c)

# --- Shared Target 6: mockbios (Shared Library for Windows/Linux) ---
add_library(mockbios SHARED src/bios/mock_bios.c)
target_include_directories(mockbios PRIVATE include)
set_target_properties(mockbios PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build_linux"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build_linux"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build_linux"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build_linux"
    PREFIX ""
)



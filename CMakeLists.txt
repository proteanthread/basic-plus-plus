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
#   under engine/ which defines ultra-granular micro-libraries and presets.
#
# SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
# - What can be changed: Compiler flags, default target profiles, new libs.
# - What cannot be changed: C17 compliance, directory-organized includes.
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
    add_compile_options(/W4 /WX /wd4100 /wd4201 /wd4996) # Wall, treat warnings as errors, disable unused param
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

# Add the engine subdirectory
add_subdirectory(engine)

# System libraries for sockets and math
if(WIN32)
    set(SYS_LIBS ws2_32)
else()
    set(SYS_LIBS m)
endif()

# Convert micro-library target list to object targets for executable linkage
function(get_target_objects out_var lib_list)
    set(obj_list "")
    foreach(lib ${lib_list})
        list(APPEND obj_list $<TARGET_OBJECTS:${lib}>)
    endforeach()
    set(${out_var} ${obj_list} PARENT_SCOPE)
endfunction()

get_target_objects(FULL_STD_OBJECTS "${SET_FULL_STANDARD_LIBS}")
get_target_objects(HEADLESS_OBJECTS "${SET_HEADLESS_LIBS}")

# --- Binary Target 1: baspp (Desktop Standard Edition) ---
add_executable(baspp engine/src/bootstrap/desktop/desktop.c ${FULL_STD_OBJECTS})
set_target_properties(baspp PROPERTIES ENABLE_EXPORTS TRUE)
target_include_directories(baspp PRIVATE engine/include engine/src/device/bgi/aalib/include)
target_compile_definitions(baspp PRIVATE BASIC_SDL_BUILD BGI_HAVE_AALIB BASIC_STANDARD_BUILD BASIC_CORE_BUILD)
target_link_libraries(baspp PRIVATE libadvanced ${SDL2_LIBRARIES} ${SYS_LIBS})
if(MSVC)
    target_link_libraries(baspp PRIVATE delayimp.lib)
    target_link_options(baspp PRIVATE "/DELAYLOAD:SDL2.dll")
endif()

# --- Shared Library Target: basicpp (C SDK Shared Library) ---
add_library(basicpp SHARED engine/src/core/bpp_api.c ${FULL_STD_OBJECTS})
set_target_properties(basicpp PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
target_include_directories(basicpp PRIVATE engine/include engine/src/device/bgi/aalib/include)
target_compile_definitions(basicpp PRIVATE BASIC_STANDARD_BUILD BASIC_CORE_BUILD BGI_HAVE_AALIB)
target_link_libraries(basicpp PRIVATE libadvanced ${SDL2_LIBRARIES} ${SYS_LIBS})
if(MSVC)
    target_link_libraries(basicpp PRIVATE delayimp.lib)
    target_link_options(basicpp PRIVATE "/DELAYLOAD:SDL2.dll")
endif()

# --- Binary Target 2: bpp (Lite REPL Edition) ---
add_executable(bpp engine/src/bootstrap/iot/iot.c ${HEADLESS_OBJECTS})
target_include_directories(bpp PRIVATE engine/include)
target_compile_definitions(bpp PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
target_link_libraries(bpp PRIVATE libflex ${SYS_LIBS})

# --- Shared Library Target: iob (Internet of BASIC - IoT & Edge Edition) ---
add_library(iob SHARED engine/src/core/bpp_api.c ${HEADLESS_OBJECTS})
set_target_properties(iob PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE OUTPUT_NAME "iob")
target_include_directories(iob PRIVATE engine/include)
target_compile_definitions(iob PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
target_link_libraries(iob PRIVATE libflex ${SYS_LIBS})

# --- WebAssembly Target: wap (Web Application BASIC - WASM Edition) ---
if(EMSCRIPTEN)
    add_executable(wap engine/src/core/bpp_api.c ${HEADLESS_OBJECTS})
    target_include_directories(wap PRIVATE engine/include)
    target_compile_definitions(wap PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2 WASM_BUILD)
    set_target_properties(wap PROPERTIES LINK_FLAGS "-s WASM=1 -s EXPORTED_FUNCTIONS='[\"_bpp_init\",\"_bpp_exec_string\",\"_bpp_eval_expr\",\"_bpp_get_var_num\",\"_bpp_set_var_num\",\"_bpp_get_var_str\",\"_bpp_set_var_str\",\"_bpp_shutdown\"]' -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'")
endif()

# --- Binary Target 3: bs (Batch Script Runner) ---
add_executable(bs engine/src/bootstrap/server/server.c ${HEADLESS_OBJECTS})
target_include_directories(bs PRIVATE engine/include)
target_compile_definitions(bs PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
target_link_libraries(bs PRIVATE libscript ${SYS_LIBS})

# --- Binary Target 4: bppc (Transpiler/Compiler Tool) ---
add_executable(bppc engine/src/tools/bppc.c ${FULL_STD_OBJECTS})
target_include_directories(bppc PRIVATE engine/include)
target_link_libraries(bppc PRIVATE libadvanced ${SYS_LIBS})

# --- Binary Target 5: detok (GW-BASIC Detokenizer Tool) ---
add_executable(detok engine/src/tools/detok.c ${FULL_STD_OBJECTS})
target_include_directories(detok PRIVATE engine/include)
target_link_libraries(detok PRIVATE libadvanced ${SDL2_LIBRARIES} ${SYS_LIBS})

# Note: basstub, baspp_headless, and baspp_mobile targets are excluded per user directive.

# Size Optimization configurations for Lite targets
if(MSVC)
    target_compile_options(bpp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(bs PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_link_options(bpp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_link_options(bs PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_compile_options(baspp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_link_options(baspp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG>)
else()
    target_compile_options(bpp PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(bs PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_link_options(bpp PRIVATE -Os -flto -Wl,--gc-sections)
    target_link_options(bs PRIVATE -Os -flto -Wl,--gc-sections)
endif()

# Post-build copy rules for standalone execution
if(WIN32)
    add_custom_command(TARGET baspp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/SDL2.dll"
        $<TARGET_FILE_DIR:baspp>
    )
endif()

add_custom_command(TARGET baspp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:baspp>
    "${CMAKE_SOURCE_DIR}/"
)
add_custom_command(TARGET bpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:bpp>
    "${CMAKE_SOURCE_DIR}/"
)
add_custom_command(TARGET bs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:bs>
    "${CMAKE_SOURCE_DIR}/"
)

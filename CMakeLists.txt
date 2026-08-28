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
include_directories(engine/include engine/iot/include)

# SDL2 configuration
if(WIN32)
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
    set(SYS_LIBS ws2_32 winmm)
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
target_link_libraries(baspp PRIVATE libadvanced libinterop_core ${SDL2_LIBRARIES} ${SYS_LIBS})
if(MSVC)
    target_link_libraries(baspp PRIVATE delayimp.lib)
    target_link_options(baspp PRIVATE "/DELAYLOAD:SDL2.dll")
else()
    set_target_properties(baspp PROPERTIES LINK_FLAGS "-Wl,--start-group")
endif()

# --- Shared Library Target: basicpp (C SDK Shared Library) ---
add_library(basicpp SHARED engine/src/core/bpp_api.c ${FULL_STD_OBJECTS})
set_target_properties(basicpp PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
target_include_directories(basicpp PRIVATE engine/include engine/src/device/bgi/aalib/include)
target_compile_definitions(basicpp PRIVATE BASIC_STANDARD_BUILD BASIC_CORE_BUILD BGI_HAVE_AALIB)
target_link_libraries(basicpp PRIVATE libadvanced libinterop_core ${SDL2_LIBRARIES} ${SYS_LIBS})
if(MSVC)
    target_link_libraries(basicpp PRIVATE delayimp.lib)
    target_link_options(basicpp PRIVATE "/DELAYLOAD:SDL2.dll")
endif()

# --- Binary Target 2: bpp (Lite REPL Edition) ---
add_executable(bpp engine/src/bootstrap/iot/iot.c ${HEADLESS_OBJECTS})
target_include_directories(bpp PRIVATE engine/include)
target_compile_definitions(bpp PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
target_link_libraries(bpp PRIVATE libflex ${SYS_LIBS})
if(NOT MSVC)
    set_target_properties(bpp PROPERTIES LINK_FLAGS "-Wl,--start-group")
endif()

# --- Shared Library Target: iob (Internet of BASIC - IoT & Edge Edition) ---
add_library(iob SHARED engine/src/core/bpp_api.c ${HEADLESS_OBJECTS})
set_target_properties(iob PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE OUTPUT_NAME "iob")
target_include_directories(iob PRIVATE engine/include)
target_compile_definitions(iob PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
target_link_libraries(iob PRIVATE libflex libinterop_core ${SYS_LIBS})
if(NOT MSVC)
    set_target_properties(iob PROPERTIES LINK_FLAGS "-Wl,--start-group")
endif()

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
if(NOT MSVC)
    set_target_properties(bs PROPERTIES LINK_FLAGS "-Wl,--start-group")
endif()

# --- Binary Target 4: bppc (Transpiler/Compiler Tool) ---
add_executable(bppc
    engine/src/tools/bppc.c
    engine/src/tools/bppc/bppc_transpile.c
    engine/src/tools/bppc/bppc_detok.c
    engine/src/tools/bppc/bppc_bytecode.c
    engine/src/tools/bppc/bppc_main.c
    ${FULL_STD_OBJECTS}
)
target_include_directories(bppc PRIVATE engine/include)
target_link_libraries(bppc PRIVATE libadvanced ${SDL2_LIBRARIES} ${SYS_LIBS})
if(MSVC)
    target_link_libraries(bppc PRIVATE delayimp.lib)
    target_link_options(bppc PRIVATE "/DELAYLOAD:SDL2.dll")
endif()

# --- Binary Target 5: detok (GW-BASIC Detokenizer Tool) ---
add_executable(detok engine/src/tools/detok.c ${FULL_STD_OBJECTS})
target_include_directories(detok PRIVATE engine/include)
target_link_libraries(detok PRIVATE libadvanced ${SDL2_LIBRARIES} ${SYS_LIBS})

# --- Binary Target 6: trans (Source-to-Source Transpiler Tool) ---
add_executable(trans
    engine/src/tools/trans.c
    engine/src/tools/bppc/bppc_transpile.c
    engine/src/tools/bppc/bppc_detok.c
    engine/src/tools/bppc/bppc_bytecode.c
    ${HEADLESS_OBJECTS}
)
target_include_directories(trans PRIVATE engine/include)
target_compile_definitions(trans PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
target_link_libraries(trans PRIVATE libcompiler_ir libscript ${SYS_LIBS})
if(NOT MSVC)
    set_target_properties(trans PROPERTIES LINK_FLAGS "-Wl,--start-group")
endif()

# --- Binary Target 7: iot (Microcontroller & IoT Edition) ---
add_executable(iot
    engine/iot/src/iot_main.c
    ${HEADLESS_OBJECTS}
)
target_include_directories(iot PRIVATE engine/include engine/iot/include)
target_compile_definitions(iot PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
target_link_libraries(iot PRIVATE libiot libscript ${SYS_LIBS})
if(NOT MSVC)
    set_target_properties(iot PROPERTIES LINK_FLAGS "-Wl,--start-group")
endif()

# Note: basstub, baspp_headless, and baspp_mobile targets are excluded per user directive.

# Size Optimization configurations for Lite targets
if(MSVC)
    target_compile_options(bpp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(bs PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(trans PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_compile_options(iot PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_link_options(bpp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_link_options(bs PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_link_options(trans PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_link_options(iot PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG> $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>)
    target_compile_options(baspp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/O1> $<$<NOT:$<CONFIG:Debug>>:/GL>)
    target_link_options(baspp PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG>)
else()
    target_compile_options(bpp PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(bs PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(trans PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_compile_options(iot PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    target_link_options(bpp PRIVATE -Os -flto -Wl,--gc-sections)
    target_link_options(bs PRIVATE -Os -flto -Wl,--gc-sections)
    target_link_options(trans PRIVATE -Os -flto -Wl,--gc-sections)
    target_link_options(iot PRIVATE -Os -flto -Wl,--gc-sections)
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
add_custom_command(TARGET bppc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:bppc>
    "${CMAKE_SOURCE_DIR}/"
)
add_custom_command(TARGET trans POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:trans>
    "${CMAKE_SOURCE_DIR}/"
)
add_custom_command(TARGET detok POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:detok>
    "${CMAKE_SOURCE_DIR}/"
)
add_custom_command(TARGET iot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:iot>
    "${CMAKE_SOURCE_DIR}/"
)

# Option to build C freestanding unit test targets (OFF by default for clean production builds)
option(BUILD_TESTING "Build C freestanding unit test targets" OFF)

if(BUILD_TESTING)
    # --- Unit Test Target: runtime_freestanding_test ---
    add_executable(runtime_freestanding_test tests/runtime_freestanding_test.c)
    target_include_directories(runtime_freestanding_test PRIVATE engine/include)
    target_link_libraries(runtime_freestanding_test PRIVATE libhal libplatform libcore_runtime ${SYS_LIBS})

    # --- Unit Test Target: boot_freestanding_test ---
    add_executable(boot_freestanding_test tests/boot_freestanding_test.c ${HEADLESS_OBJECTS})
    target_include_directories(boot_freestanding_test PRIVATE engine/include)
    target_compile_definitions(boot_freestanding_test PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
    target_link_libraries(boot_freestanding_test PRIVATE libflex ${SYS_LIBS})
    if(NOT MSVC)
        set_target_properties(boot_freestanding_test PROPERTIES LINK_FLAGS "-Wl,--start-group")
    endif()

    # --- Unit Test Target: kernel_freestanding_test ---
    add_executable(kernel_freestanding_test tests/kernel_freestanding_test.c ${HEADLESS_OBJECTS})
    target_include_directories(kernel_freestanding_test PRIVATE engine/include)
    target_compile_definitions(kernel_freestanding_test PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
    target_link_libraries(kernel_freestanding_test PRIVATE libflex ${SYS_LIBS})
    if(NOT MSVC)
        set_target_properties(kernel_freestanding_test PROPERTIES LINK_FLAGS "-Wl,--start-group")
    endif()

    # --- Unit Test Target: hardware_freestanding_test ---
    add_executable(hardware_freestanding_test tests/hardware_freestanding_test.c ${HEADLESS_OBJECTS})
    target_include_directories(hardware_freestanding_test PRIVATE engine/include)
    target_compile_definitions(hardware_freestanding_test PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
    target_link_libraries(hardware_freestanding_test PRIVATE libflex ${SYS_LIBS})
    if(NOT MSVC)
        set_target_properties(hardware_freestanding_test PROPERTIES LINK_FLAGS "-Wl,--start-group")
    endif()

    # --- Unit Test Target: server_freestanding_test ---
    add_executable(server_freestanding_test tests/server_freestanding_test.c ${HEADLESS_OBJECTS})
    target_include_directories(server_freestanding_test PRIVATE engine/include)
    target_compile_definitions(server_freestanding_test PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
    target_link_libraries(server_freestanding_test PRIVATE libflex ${SYS_LIBS})
    if(NOT MSVC)
        set_target_properties(server_freestanding_test PROPERTIES LINK_FLAGS "-Wl,--start-group")
    endif()

    # --- Unit Test Target: script_freestanding_test ---
    add_executable(script_freestanding_test tests/script_freestanding_test.c ${HEADLESS_OBJECTS})
    target_include_directories(script_freestanding_test PRIVATE engine/include)
    target_compile_definitions(script_freestanding_test PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
    target_link_libraries(script_freestanding_test PRIVATE libflex ${SYS_LIBS})
    if(NOT MSVC)
        set_target_properties(script_freestanding_test PROPERTIES LINK_FLAGS "-Wl,--start-group")
    endif()

    # --- Unit Test Target: core_flex_freestanding_test ---
    add_executable(core_flex_freestanding_test tests/core_flex_freestanding_test.c ${HEADLESS_OBJECTS})
    target_include_directories(core_flex_freestanding_test PRIVATE engine/include)
    target_compile_definitions(core_flex_freestanding_test PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
    target_link_libraries(core_flex_freestanding_test PRIVATE libflex ${SYS_LIBS})
    if(NOT MSVC)
        set_target_properties(core_flex_freestanding_test PROPERTIES LINK_FLAGS "-Wl,--start-group")
    endif()

    # --- Unit Test Target: standard_tui_freestanding_test ---
    add_executable(standard_tui_freestanding_test tests/standard_tui_freestanding_test.c ${HEADLESS_OBJECTS})
    target_include_directories(standard_tui_freestanding_test PRIVATE engine/include)
    target_compile_definitions(standard_tui_freestanding_test PRIVATE BASIC_STANDARD_BUILD BASIC_CORE_BUILD BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
    target_link_libraries(standard_tui_freestanding_test PRIVATE libstandard ${SYS_LIBS})
    if(NOT MSVC)
        set_target_properties(standard_tui_freestanding_test PROPERTIES LINK_FLAGS "-Wl,--start-group")
    endif()

    # --- Unit Test Target: compiler_freestanding_test ---
    add_executable(compiler_freestanding_test
        tests/compiler_freestanding_test.c
        engine/src/tools/bppc/bppc_transpile.c
        engine/src/tools/bppc/bppc_detok.c
        engine/src/tools/bppc/bppc_bytecode.c
        ${HEADLESS_OBJECTS}
    )
    target_include_directories(compiler_freestanding_test PRIVATE engine/include)
    target_compile_definitions(compiler_freestanding_test PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
    target_link_libraries(compiler_freestanding_test PRIVATE libcompiler_ir libscript ${SYS_LIBS})
    if(NOT MSVC)
        set_target_properties(compiler_freestanding_test PROPERTIES LINK_FLAGS "-Wl,--start-group")
    endif()
endif()


# Custom target for rapid intermediate cleanup across platforms
if(WIN32)
    add_custom_target(clean_intermediates
        COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/tools/clean_intermediates.ps1"
        COMMENT "Pruning intermediate compiler artifacts from build_win and repository root..."
    )
else()
    add_custom_target(clean_intermediates
        COMMAND "${CMAKE_SOURCE_DIR}/tools/clean_intermediates.sh"
        COMMENT "Pruning intermediate compiler artifacts from build_linux and repository root..."
    )
endif()


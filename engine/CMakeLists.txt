# =====================================================================
# BASIC++ v6.5.0 - engine/ CMake Configuration
# =====================================================================
#
# SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
# - What it does: Compiles the core library libbasicpp and libbasicpp_lite,
#   defines all micro-library targets for platform, VM, editor, and
#   statement subsystems under the engine/ directory tree.
# - Why it exists: Centralizes all engine build configuration after the
#   v6 restructure that moved src/, include/, and lib/ under engine/.
# - Why it works this way: Uses directory-organized includes
#   (engine/include/<category>/) instead of flat bpp_*.h headers.
#   Each subsystem is compiled as a separate static micro-library for
#   maximum granularity and reusability.
#
# SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
# - What can be changed: Adding new source files, new micro-libraries,
#   new statement categories, or new include subdirectories.
# - What cannot be changed: The directory-organized include structure
#   (engine/include/<category>/), C17 compliance requirements.
# - What to expect: All include paths are relative to engine/include/.
#   Source files use #include "vm/vm.h" not #include "bpp_vm.h".
# - What to do if something breaks: Check that include_directories
#   points to engine/include/ and all subdirectories exist.
#
# =====================================================================

# --- Include Directories ---
# All targets can find headers via #include "category/name.h"
set(ENGINE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${ENGINE_INCLUDE_DIR})

# Also expose aalib private headers for BGI subsystem
set(AALIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/device/bgi/aalib/include)

# =====================================================================
# Core Library: libbasicpp (Full Standard Edition)
# =====================================================================
add_library(libbasicpp STATIC
    src/bootstrap/boot.c
    src/vm/vm_stack.c
    src/vm/vm_host.c
    src/memory/mem_system.c
    src/memory/segmented_mem.c
    src/lexer/lexer.c
    src/core/dialect.c
    src/core/struct.c
    src/parser/parser.c
    src/eval/eval.c
    src/runtime/variables.c
    src/runtime/strings.c
    src/runtime/arrays.c
    src/runtime/file.c
    src/runtime/print_using.c
    src/runtime/using_engine.c
    src/runtime/task.c
    src/runtime/metadata.c
    src/runtime/funcreg.c
    src/runtime/map.c
    src/runtime/map_serialize.c
    src/runtime/crypto.c
    src/runtime/string_ext.c
    src/runtime/array_sort.c
    src/module/mod_mathext.c
    src/module/mod_arrayext.c
    src/security/security.c
    src/module/module.c
    src/module/mod_regex.c
    src/device/vdev.c
    src/device/vdev_console.c
    src/device/vdev_usb.c
    src/device/vdev_vcon.c
    src/device/vdev_bus.c
    src/runtime/vfs.c
    src/runtime/vnet.c
    src/bios/mock_bios.c
    src/runtime/spec.c
    src/editor/editor.c
    src/editor/editor_manager.c
    src/editor/mod_edit.c
    src/editor/mod_ws.c
    src/editor/mod_vi.c
    src/editor/mod_edlin.c
    src/editor/tui_multiplexer.c
    src/debug/dap_server.c
    src/debug/logger.c
    src/runtime/state.c
    src/device/vdev_fujinet.c
    src/runtime/gemini.c
    src/device/bgi/bgi_core.c
    src/device/bgi/bgi_raster.c
    src/device/bgi/bgi_font.c
    src/device/bgi/bgi_modes.c
    src/device/bgi/aalib/aalib.c
)

target_include_directories(libbasicpp PUBLIC ${ENGINE_INCLUDE_DIR} ${AALIB_INCLUDE_DIR})
target_compile_definitions(libbasicpp PRIVATE BGI_HAVE_AALIB)

# =====================================================================
# Core Library: libbasicpp_lite (Headless Lite Edition)
# =====================================================================
add_library(libbasicpp_lite STATIC
    src/bootstrap/boot.c
    src/vm/vm_stack.c
    src/memory/mem_system.c
    src/lexer/lexer.c
    src/core/dialect.c
    src/parser/parser.c
    src/eval/eval.c
    src/runtime/variables.c
    src/runtime/strings.c
    src/runtime/arrays.c
    src/runtime/file.c
    src/runtime/print_using.c
    src/runtime/using_engine.c
    src/runtime/task.c
    src/runtime/metadata.c
    src/runtime/funcreg.c
    src/runtime/map.c
    src/runtime/map_serialize.c
    src/runtime/crypto.c
    src/runtime/string_ext.c
    src/runtime/array_sort.c
    src/module/module.c
    src/module/mod_regex.c
    src/device/vdev.c
    src/device/vdev_console.c
    src/device/vdev_usb.c
    src/device/vdev_vcon.c
    src/device/vdev_bus.c
    src/runtime/vfs.c
    src/runtime/spec.c
    src/debug/logger.c
    src/runtime/state.c
    src/device/bgi/bgi_core.c
    src/device/bgi/bgi_raster.c
    src/device/bgi/bgi_font.c
    src/device/bgi/bgi_modes.c
    src/device/bgi/aalib/aalib.c
)
target_include_directories(libbasicpp_lite PUBLIC ${ENGINE_INCLUDE_DIR} ${AALIB_INCLUDE_DIR})
target_compile_definitions(libbasicpp_lite PRIVATE BPP_LITE_BUILD NO_SDL2 BGI_HAVE_AALIB)

# =====================================================================
# POSIX Definitions for UNIX builds
# =====================================================================
if(UNIX)
    target_compile_definitions(libbasicpp PUBLIC _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_compile_definitions(libbasicpp_lite PUBLIC _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
endif()

# =====================================================================
# Platform Micro-Libraries
# =====================================================================
add_library(plat_console STATIC lib/platform/plat_console.c)
target_include_directories(plat_console PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(plat_fs STATIC lib/platform/plat_fs.c)
target_include_directories(plat_fs PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(plat_sys STATIC lib/platform/plat_sys.c)
target_include_directories(plat_sys PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(plat_time STATIC lib/platform/plat_time.c)
target_include_directories(plat_time PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(plat_thread STATIC lib/platform/plat_thread.c)
target_include_directories(plat_thread PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(plat_dl STATIC lib/platform/plat_dl.c)
target_include_directories(plat_dl PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(plat_net STATIC lib/platform/plat_net.c)
target_include_directories(plat_net PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(plat_regex STATIC lib/platform/plat_regex.c)
target_include_directories(plat_regex PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(plat_clipboard STATIC lib/platform/plat_clipboard.c)
target_include_directories(plat_clipboard PUBLIC ${ENGINE_INCLUDE_DIR})

if(UNIX)
    set(PLAT_TARGETS plat_console plat_fs plat_sys plat_time
        plat_thread plat_dl plat_net plat_regex plat_clipboard)
    foreach(tgt ${PLAT_TARGETS})
        target_compile_definitions(${tgt} PRIVATE
            _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    endforeach()
endif()

# =====================================================================
# VM Micro-Libraries
# =====================================================================
add_library(vm_context STATIC src/vm/vm_context.c)
target_include_directories(vm_context PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_exec STATIC src/vm/vm_exec.c)
target_include_directories(vm_exec PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_data STATIC src/vm/vm_data.c)
target_include_directories(vm_data PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_error STATIC src/vm/vm_error.c)
target_include_directories(vm_error PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_control STATIC src/vm/vm_control.c)
target_include_directories(vm_control PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_events STATIC src/vm/vm_events.c)
target_include_directories(vm_events PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_math STATIC src/vm/vm_math.c)
target_include_directories(vm_math PUBLIC ${ENGINE_INCLUDE_DIR})

# =====================================================================
# Editor Micro-Libraries
# =====================================================================
add_library(editor_buffer STATIC lib/editor/editor_buffer.c)
target_include_directories(editor_buffer PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(editor_render STATIC lib/editor/editor_render.c)
target_include_directories(editor_render PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(editor_selection STATIC lib/editor/editor_selection.c)
target_include_directories(editor_selection PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(editor_term STATIC lib/editor/editor_term.c)
target_include_directories(editor_term PUBLIC ${ENGINE_INCLUDE_DIR})

if(UNIX)
    target_compile_definitions(editor_render PRIVATE
        _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
endif()

# =====================================================================
# Statement Micro-Libraries
# =====================================================================
# stmt_advanced/
add_library(stmt_module STATIC src/statements/stmt_advanced/stmt_module.c)
target_include_directories(stmt_module PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_security STATIC src/statements/stmt_advanced/stmt_security.c)
target_include_directories(stmt_security PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_task STATIC src/statements/stmt_advanced/stmt_task.c)
target_include_directories(stmt_task PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_txn STATIC src/statements/stmt_advanced/stmt_txn.c)
target_include_directories(stmt_txn PUBLIC ${ENGINE_INCLUDE_DIR})

# stmt_core/
add_library(stmt_data STATIC src/statements/stmt_core/stmt_data.c)
target_include_directories(stmt_data PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_dim STATIC src/statements/stmt_core/stmt_dim.c)
target_include_directories(stmt_dim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_flow STATIC src/statements/stmt_core/stmt_flow.c)
target_include_directories(stmt_flow PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_input STATIC src/statements/stmt_core/stmt_input.c)
target_include_directories(stmt_input PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_let STATIC src/statements/stmt_core/stmt_let.c)
target_include_directories(stmt_let PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_loop STATIC src/statements/stmt_core/stmt_loop.c)
target_include_directories(stmt_loop PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_print STATIC src/statements/stmt_core/stmt_print.c)
target_include_directories(stmt_print PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_program STATIC src/statements/stmt_core/stmt_program.c)
target_include_directories(stmt_program PUBLIC ${ENGINE_INCLUDE_DIR})

# stmt_dialect/
add_library(stmt_dialect STATIC src/statements/stmt_dialect/stmt_dialect.c)
target_include_directories(stmt_dialect PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_error STATIC src/statements/stmt_dialect/stmt_error.c)
target_include_directories(stmt_error PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_help STATIC src/statements/stmt_dialect/stmt_help.c)
target_include_directories(stmt_help PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_introspection STATIC src/statements/stmt_dialect/stmt_introspection.c)
target_include_directories(stmt_introspection PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_remove STATIC src/statements/stmt_dialect/stmt_remove.c)
target_include_directories(stmt_remove PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_selftest STATIC src/statements/stmt_dialect/stmt_selftest.c)
target_include_directories(stmt_selftest PUBLIC ${ENGINE_INCLUDE_DIR})

# stmt_extended/
add_library(stmt_arrayext STATIC src/statements/stmt_extended/stmt_arrayext.c)
target_include_directories(stmt_arrayext PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_def STATIC src/statements/stmt_extended/stmt_def.c)
target_include_directories(stmt_def PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_event STATIC src/statements/stmt_extended/stmt_event.c)
target_include_directories(stmt_event PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat STATIC src/statements/stmt_extended/stmt_mat.c)
target_include_directories(stmt_mat PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_randomize STATIC src/statements/stmt_extended/stmt_randomize.c)
target_include_directories(stmt_randomize PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_select STATIC src/statements/stmt_extended/stmt_select.c)
target_include_directories(stmt_select PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_struct STATIC src/statements/stmt_extended/stmt_struct.c)
target_include_directories(stmt_struct PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_sub STATIC src/statements/stmt_extended/stmt_sub.c)
target_include_directories(stmt_sub PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_try STATIC src/statements/stmt_extended/stmt_try.c)
target_include_directories(stmt_try PUBLIC ${ENGINE_INCLUDE_DIR})

# stmt_graphics/
add_library(stmt_beep STATIC src/statements/stmt_graphics/stmt_beep.c)
target_include_directories(stmt_beep PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_bgi STATIC src/statements/stmt_graphics/stmt_bgi.c)
target_include_directories(stmt_bgi PUBLIC ${ENGINE_INCLUDE_DIR} ${AALIB_INCLUDE_DIR})

add_library(stmt_compat STATIC src/statements/stmt_graphics/stmt_compat.c)
target_include_directories(stmt_compat PUBLIC ${ENGINE_INCLUDE_DIR})

# stmt_io/
add_library(stmt_defseg STATIC src/statements/stmt_io/stmt_defseg.c)
target_include_directories(stmt_defseg PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_file STATIC src/statements/stmt_io/stmt_file.c)
target_include_directories(stmt_file PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_filemgmt STATIC src/statements/stmt_io/stmt_filemgmt.c)
target_include_directories(stmt_filemgmt PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_io_devices STATIC src/statements/stmt_io/stmt_io_devices.c)
target_include_directories(stmt_io_devices PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_lset STATIC src/statements/stmt_io/stmt_lset.c)
target_include_directories(stmt_lset PUBLIC ${ENGINE_INCLUDE_DIR})

# stmt_system/
add_library(stmt_bios STATIC src/statements/stmt_system/stmt_bios.c)
target_include_directories(stmt_bios PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_debug STATIC src/statements/stmt_system/stmt_debug.c)
target_include_directories(stmt_debug PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_infra STATIC src/statements/stmt_system/stmt_infra.c)
target_include_directories(stmt_infra PUBLIC ${ENGINE_INCLUDE_DIR})

# =====================================================================
# Link Micro-Libraries into Core Libraries
# =====================================================================
set(ALL_PLAT_LIBS plat_console plat_fs plat_sys plat_time
    plat_thread plat_dl plat_net plat_regex plat_clipboard)

set(ALL_VM_LIBS vm_context vm_exec vm_data vm_error vm_control vm_events vm_math)

set(ALL_EDITOR_LIBS editor_buffer editor_render editor_selection editor_term)

set(ALL_STMT_LIBS
    stmt_print stmt_let stmt_lset stmt_input stmt_randomize
    stmt_introspection stmt_beep stmt_flow stmt_program stmt_loop
    stmt_dim stmt_def stmt_data stmt_error stmt_try stmt_struct
    stmt_selftest stmt_file stmt_filemgmt stmt_select stmt_sub
    stmt_security stmt_module stmt_task stmt_mat stmt_arrayext
    stmt_help stmt_infra stmt_defseg stmt_compat stmt_remove
    stmt_dialect stmt_txn stmt_event stmt_bios stmt_debug
    stmt_io_devices stmt_bgi
)

target_link_libraries(libbasicpp PUBLIC
    ${ALL_PLAT_LIBS} ${ALL_VM_LIBS} ${ALL_STMT_LIBS} ${ALL_EDITOR_LIBS})
target_link_libraries(libbasicpp_lite PUBLIC
    ${ALL_PLAT_LIBS} ${ALL_VM_LIBS})

if(UNIX)
    target_link_libraries(libbasicpp PUBLIC m ncurses)
    target_link_libraries(libbasicpp_lite PUBLIC m)
endif()

if(WIN32)
    target_link_libraries(libbasicpp PUBLIC ws2_32)
    target_link_libraries(libbasicpp_lite PUBLIC ws2_32)
endif()

# NOTE: Executable targets (baspp, blite, bscript, bppc, detok) are
# defined in the root CMakeLists.txt where SDL2 and platform-specific
# configurations are available.

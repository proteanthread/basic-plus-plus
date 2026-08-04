# =====================================================================
# BASIC++ v6.0.0 — src/ CMake Configuration
# =====================================================================
#
# SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
# - What it does: Compiles the core library libbasicpp containing the memory manager,
#   ephemeral lexer, statement registries, expression evaluators, VDS, and VM context.
# - Why it exists: Aggregates core subsystems into a single compilation unit, ensuring
#   that all three binary targets use the identical interpreter runtime.
# - Why it works this way: It specifies all source files for the static target libbasicpp.
#   It links include directories cleanly.
#
# SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
# - What can be changed: Adding new command/statement source files to the sources list.
# - What cannot be changed: Registry bindings or core libraries linkages.
# - What to expect: Changes to source files list will regenerate the static compilation output.
# - What to do if something breaks: Ensure the file path is correct and relative to src/.
#
# =====================================================================

add_library(libbasicpp STATIC
    bootstrap/boot.c
    vm/vm_stack.c
    vm/vm_host.c
    memory/mem_system.c
    memory/segmented_mem.c
    lexer/lexer.c
    core/dialect.c
    core/struct.c
    parser/parser.c
    expression/eval.c
    runtime/variables.c
    runtime/strings.c
    runtime/arrays.c
    runtime/file.c
    runtime/print_using.c
    runtime/num_format.c
    runtime/using_engine.c
    runtime/task.c
    runtime/metadata.c
    runtime/funcreg.c
    runtime/map.c
    runtime/map_serialize.c
    runtime/crypto.c
    runtime/string_ext.c
    runtime/array_sort.c
    module/mod_mathext.c
    module/mod_arrayext.c
    security/security.c
    module/module.c
    module/mod_regex.c
    device/vdev.c
    device/vdev_console.c
    device/vdev_usb.c
    device/vdev_vcon.c
    device/vdev_bus.c
    runtime/vfs.c
    runtime/vnet.c
    bios/mock_bios.c
    runtime/spec.c
    editor/editor.c
    editor/editor_manager.c
    editor/mod_edit.c
    editor/mod_ws.c
    editor/mod_vi.c
    editor/mod_edlin.c
    editor/tui_multiplexer.c
    debug/dap_server.c
    debug/bpp_logger.c
    runtime/state.c
    device/vdev_fujinet.c
    runtime/gemini.c
    device/bgi/bgi_core.c
    device/bgi/bgi_raster.c
    device/bgi/bgi_font.c
    device/bgi/bgi_modes.c
    device/bgi/aalib/aalib.c
)

target_include_directories(libbasicpp PUBLIC ../include device/bgi/aalib/include)
target_compile_definitions(libbasicpp PRIVATE BGI_HAVE_AALIB)

add_library(libbasicpp_lite STATIC
    bootstrap/boot.c
    vm/vm_stack.c
    memory/mem_system.c
    lexer/lexer.c
    core/dialect.c
    parser/parser.c
    expression/eval.c
    runtime/variables.c
    runtime/strings.c
    runtime/arrays.c
    runtime/file.c
    runtime/print_using.c
    runtime/num_format.c
    runtime/using_engine.c
    runtime/task.c
    runtime/metadata.c
    runtime/funcreg.c
    runtime/map.c
    runtime/map_serialize.c
    runtime/crypto.c
    runtime/string_ext.c
    runtime/array_sort.c
    module/module.c
    module/mod_regex.c
    device/vdev.c
    device/vdev_console.c
    device/vdev_usb.c
    device/vdev_vcon.c
    device/vdev_bus.c
    runtime/vfs.c
    runtime/spec.c
    debug/bpp_logger.c
    runtime/state.c
    device/bgi/bgi_core.c
    device/bgi/bgi_raster.c
    device/bgi/bgi_font.c
    device/bgi/bgi_modes.c
    device/bgi/aalib/aalib.c
)
target_include_directories(libbasicpp_lite PUBLIC ../include device/bgi/aalib/include)
target_compile_definitions(libbasicpp_lite PRIVATE BPP_LITE_BUILD NO_SDL2 BGI_HAVE_AALIB)

if(UNIX)
    target_compile_definitions(libbasicpp PUBLIC _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_compile_definitions(libbasicpp_lite PUBLIC _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(libbasicpp PUBLIC
    bpp_plat_console
    bpp_plat_fs
    bpp_plat_sys
    bpp_plat_time
    bpp_plat_thread
    bpp_plat_dl
    bpp_plat_net
    bpp_plat_regex
    bpp_plat_clipboard
    bpp_vm_context
    bpp_vm_exec
    bpp_vm_data
    bpp_vm_error
    bpp_vm_control
    bpp_vm_events
    bpp_vm_math m ncurses)
    target_link_libraries(libbasicpp_lite PUBLIC
    bpp_plat_console
    bpp_plat_fs
    bpp_plat_sys
    bpp_plat_time
    bpp_plat_thread
    bpp_plat_dl
    bpp_plat_net
    bpp_plat_regex
    bpp_plat_clipboard
    bpp_vm_context
    bpp_vm_exec
    bpp_vm_data
    bpp_vm_error
    bpp_vm_control
    bpp_vm_events
    bpp_vm_math m)
endif()

if(WIN32)
    target_link_libraries(libbasicpp PUBLIC
    bpp_plat_console
    bpp_plat_fs
    bpp_plat_sys
    bpp_plat_time
    bpp_plat_thread
    bpp_plat_dl
    bpp_plat_net
    bpp_plat_regex
    bpp_plat_clipboard
    bpp_vm_context
    bpp_vm_exec
    bpp_vm_data
    bpp_vm_error
    bpp_vm_control
    bpp_vm_events
    bpp_vm_math ws2_32)
    target_link_libraries(libbasicpp_lite PUBLIC
    bpp_plat_console
    bpp_plat_fs
    bpp_plat_sys
    bpp_plat_time
    bpp_plat_thread
    bpp_plat_dl
    bpp_plat_net
    bpp_plat_regex
    bpp_plat_clipboard
    bpp_vm_context
    bpp_vm_exec
    bpp_vm_data
    bpp_vm_error
    bpp_vm_control
    bpp_vm_events
    bpp_vm_math ws2_32)
endif()


# Platform Micro-Libraries
add_library(bpp_plat_console STATIC ../lib/platform/plat_console.c)
target_include_directories(bpp_plat_console PUBLIC ../include)

add_library(bpp_plat_fs STATIC ../lib/platform/plat_fs.c)
target_include_directories(bpp_plat_fs PUBLIC ../include)

add_library(bpp_plat_sys STATIC ../lib/platform/plat_sys.c)
target_include_directories(bpp_plat_sys PUBLIC ../include)

add_library(bpp_plat_time STATIC ../lib/platform/plat_time.c)
target_include_directories(bpp_plat_time PUBLIC ../include)

add_library(bpp_plat_thread STATIC ../lib/platform/plat_thread.c)
target_include_directories(bpp_plat_thread PUBLIC ../include)

add_library(bpp_plat_dl STATIC ../lib/platform/plat_dl.c)
target_include_directories(bpp_plat_dl PUBLIC ../include)

add_library(bpp_plat_net STATIC ../lib/platform/plat_net.c)
target_include_directories(bpp_plat_net PUBLIC ../include)

add_library(bpp_plat_regex STATIC ../lib/platform/plat_regex.c)
target_include_directories(bpp_plat_regex PUBLIC ../include)

add_library(bpp_plat_clipboard STATIC ../lib/platform/plat_clipboard.c)
target_include_directories(bpp_plat_clipboard PUBLIC ../include)

if(UNIX)
    set(BPP_PLAT_TARGETS
        bpp_plat_console bpp_plat_fs bpp_plat_sys bpp_plat_time
        bpp_plat_thread bpp_plat_dl bpp_plat_net bpp_plat_regex
        bpp_plat_clipboard)
    foreach(tgt ${BPP_PLAT_TARGETS})
        target_compile_definitions(${tgt} PRIVATE
            _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    endforeach()
endif()

# VM Micro-Libraries
add_library(bpp_vm_context STATIC vm/vm_context.c)
target_include_directories(bpp_vm_context PUBLIC ../include)

add_library(bpp_vm_exec STATIC vm/vm_exec.c)
target_include_directories(bpp_vm_exec PUBLIC ../include)

add_library(bpp_vm_data STATIC vm/vm_data.c)
target_include_directories(bpp_vm_data PUBLIC ../include)

add_library(bpp_vm_error STATIC vm/vm_error.c)
target_include_directories(bpp_vm_error PUBLIC ../include)

add_library(bpp_vm_control STATIC vm/vm_control.c)
target_include_directories(bpp_vm_control PUBLIC ../include)

add_library(bpp_vm_events STATIC vm/vm_events.c)
target_include_directories(bpp_vm_events PUBLIC ../include)

add_library(bpp_vm_math STATIC vm/vm_math.c)
target_include_directories(bpp_vm_math PUBLIC ../include)

# Editor Micro-Libraries
add_library(bpp_editor_buffer STATIC ../lib/editor/editor_buffer.c)
target_include_directories(bpp_editor_buffer PUBLIC ../include)

add_library(bpp_editor_render STATIC ../lib/editor/editor_render.c)
target_include_directories(bpp_editor_render PUBLIC ../include)

add_library(bpp_editor_selection STATIC ../lib/editor/editor_selection.c)
target_include_directories(bpp_editor_selection PUBLIC ../include)

add_library(bpp_editor_term STATIC ../lib/editor/editor_term.c)
target_include_directories(bpp_editor_term PUBLIC ../include)

if(UNIX)
    target_compile_definitions(bpp_editor_render PRIVATE
        _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
endif()

# Statement Micro-Libraries
add_library(bpp_stmt_module STATIC statements/stmt_advanced/stmt_module.c)
target_include_directories(bpp_stmt_module PUBLIC ../include)

add_library(bpp_stmt_security STATIC statements/stmt_advanced/stmt_security.c)
target_include_directories(bpp_stmt_security PUBLIC ../include)

add_library(bpp_stmt_task STATIC statements/stmt_advanced/stmt_task.c)
target_include_directories(bpp_stmt_task PUBLIC ../include)

add_library(bpp_stmt_txn STATIC statements/stmt_advanced/stmt_txn.c)
target_include_directories(bpp_stmt_txn PUBLIC ../include)

add_library(bpp_stmt_data STATIC statements/stmt_core/stmt_data.c)
target_include_directories(bpp_stmt_data PUBLIC ../include)

add_library(bpp_stmt_dim STATIC statements/stmt_core/stmt_dim.c)
target_include_directories(bpp_stmt_dim PUBLIC ../include)

add_library(bpp_stmt_flow STATIC statements/stmt_core/stmt_flow.c)
target_include_directories(bpp_stmt_flow PUBLIC ../include)

add_library(bpp_stmt_input STATIC statements/stmt_core/stmt_input.c)
target_include_directories(bpp_stmt_input PUBLIC ../include)

add_library(bpp_stmt_let STATIC statements/stmt_core/stmt_let.c)
target_include_directories(bpp_stmt_let PUBLIC ../include)

add_library(bpp_stmt_loop STATIC statements/stmt_core/stmt_loop.c)
target_include_directories(bpp_stmt_loop PUBLIC ../include)

add_library(bpp_stmt_print STATIC statements/stmt_core/stmt_print.c)
target_include_directories(bpp_stmt_print PUBLIC ../include)

add_library(bpp_stmt_program STATIC statements/stmt_core/stmt_program.c)
target_include_directories(bpp_stmt_program PUBLIC ../include)

add_library(bpp_stmt_dialect STATIC statements/stmt_dialect/stmt_dialect.c)
target_include_directories(bpp_stmt_dialect PUBLIC ../include)

add_library(bpp_stmt_error STATIC statements/stmt_dialect/stmt_error.c)
target_include_directories(bpp_stmt_error PUBLIC ../include)

add_library(bpp_stmt_help STATIC statements/stmt_dialect/stmt_help.c)
target_include_directories(bpp_stmt_help PUBLIC ../include)

add_library(bpp_stmt_introspection STATIC statements/stmt_dialect/stmt_introspection.c)
target_include_directories(bpp_stmt_introspection PUBLIC ../include)

add_library(bpp_stmt_remove STATIC statements/stmt_dialect/stmt_remove.c)
target_include_directories(bpp_stmt_remove PUBLIC ../include)

add_library(bpp_stmt_selftest STATIC statements/stmt_dialect/stmt_selftest.c)
target_include_directories(bpp_stmt_selftest PUBLIC ../include)

add_library(bpp_stmt_arrayext STATIC statements/stmt_extended/stmt_arrayext.c)
target_include_directories(bpp_stmt_arrayext PUBLIC ../include)

add_library(bpp_stmt_def STATIC statements/stmt_extended/stmt_def.c)
target_include_directories(bpp_stmt_def PUBLIC ../include)

add_library(bpp_stmt_event STATIC statements/stmt_extended/stmt_event.c)
target_include_directories(bpp_stmt_event PUBLIC ../include)

add_library(bpp_stmt_mat STATIC statements/stmt_extended/stmt_mat.c)
target_include_directories(bpp_stmt_mat PUBLIC ../include)

add_library(bpp_stmt_randomize STATIC statements/stmt_extended/stmt_randomize.c)
target_include_directories(bpp_stmt_randomize PUBLIC ../include)

add_library(bpp_stmt_select STATIC statements/stmt_extended/stmt_select.c)
target_include_directories(bpp_stmt_select PUBLIC ../include)

add_library(bpp_stmt_struct STATIC statements/stmt_extended/stmt_struct.c)
target_include_directories(bpp_stmt_struct PUBLIC ../include)

add_library(bpp_stmt_sub STATIC statements/stmt_extended/stmt_sub.c)
target_include_directories(bpp_stmt_sub PUBLIC ../include)

add_library(bpp_stmt_try STATIC statements/stmt_extended/stmt_try.c)
target_include_directories(bpp_stmt_try PUBLIC ../include)

add_library(bpp_stmt_beep STATIC statements/stmt_graphics/stmt_beep.c)
target_include_directories(bpp_stmt_beep PUBLIC ../include)

add_library(bpp_stmt_bgi STATIC statements/stmt_graphics/stmt_bgi.c)
target_include_directories(bpp_stmt_bgi PUBLIC ../include device/bgi/aalib/include)

add_library(bpp_stmt_compat STATIC statements/stmt_graphics/stmt_compat.c)
target_include_directories(bpp_stmt_compat PUBLIC ../include)

add_library(bpp_stmt_defseg STATIC statements/stmt_io/stmt_defseg.c)
target_include_directories(bpp_stmt_defseg PUBLIC ../include)

add_library(bpp_stmt_file STATIC statements/stmt_io/stmt_file.c)
target_include_directories(bpp_stmt_file PUBLIC ../include)

add_library(bpp_stmt_filemgmt STATIC statements/stmt_io/stmt_filemgmt.c)
target_include_directories(bpp_stmt_filemgmt PUBLIC ../include)

add_library(bpp_stmt_io_devices STATIC statements/stmt_io/stmt_io_devices.c)
target_include_directories(bpp_stmt_io_devices PUBLIC ../include)

add_library(bpp_stmt_lset STATIC statements/stmt_io/stmt_lset.c)
target_include_directories(bpp_stmt_lset PUBLIC ../include)

add_library(bpp_stmt_bios STATIC statements/stmt_system/stmt_bios.c)
target_include_directories(bpp_stmt_bios PUBLIC ../include)

add_library(bpp_stmt_debug STATIC statements/stmt_system/stmt_debug.c)
target_include_directories(bpp_stmt_debug PUBLIC ../include)

add_library(bpp_stmt_infra STATIC statements/stmt_system/stmt_infra.c)
target_include_directories(bpp_stmt_infra PUBLIC ../include)

target_link_libraries(libbasicpp PUBLIC
    bpp_plat_console
    bpp_plat_fs
    bpp_plat_sys
    bpp_plat_time
    bpp_plat_thread
    bpp_plat_dl
    bpp_plat_net
    bpp_plat_regex
    bpp_plat_clipboard
    bpp_vm_context
    bpp_vm_exec
    bpp_vm_data
    bpp_vm_error
    bpp_vm_control
    bpp_vm_events
    bpp_vm_math
    bpp_stmt_print
    bpp_stmt_let
    bpp_stmt_lset
    bpp_stmt_input
    bpp_stmt_randomize
    bpp_stmt_introspection
    bpp_stmt_beep
    bpp_stmt_flow
    bpp_stmt_program
    bpp_stmt_loop
    bpp_stmt_dim
    bpp_stmt_def
    bpp_stmt_data
    bpp_stmt_error
    bpp_stmt_try
    bpp_stmt_struct
    bpp_stmt_selftest
    bpp_stmt_file
    bpp_stmt_filemgmt
    bpp_stmt_select
    bpp_stmt_sub
    bpp_stmt_security
    bpp_stmt_module
    bpp_stmt_task
    bpp_stmt_mat
    bpp_stmt_arrayext
    bpp_stmt_help
    bpp_stmt_infra
    bpp_stmt_defseg
    bpp_stmt_compat
    bpp_stmt_remove
    bpp_stmt_dialect
    bpp_stmt_txn
    bpp_stmt_event
    bpp_stmt_bios
    bpp_stmt_debug
    bpp_stmt_io_devices
    bpp_stmt_bgi
    bpp_editor_buffer
    bpp_editor_render
    bpp_editor_selection
    bpp_editor_term
)

target_link_libraries(libbasicpp_lite PUBLIC
    bpp_plat_console
    bpp_plat_fs
    bpp_plat_sys
    bpp_plat_time
    bpp_plat_thread
    bpp_plat_dl
    bpp_plat_net
    bpp_plat_regex
    bpp_plat_clipboard
    bpp_vm_context
    bpp_vm_exec
    bpp_vm_data
    bpp_vm_error
    bpp_vm_control
    bpp_vm_events
    bpp_vm_math
    bpp_stmt_print
    bpp_stmt_let
    bpp_stmt_lset
    bpp_stmt_input
    bpp_stmt_randomize
    bpp_stmt_beep
    bpp_stmt_flow
    bpp_stmt_program
    bpp_stmt_loop
    bpp_stmt_dim
    bpp_stmt_def
    bpp_stmt_data
    bpp_stmt_error
    bpp_stmt_try
    bpp_stmt_selftest
    bpp_stmt_file
    bpp_stmt_filemgmt
    bpp_stmt_select
    bpp_stmt_sub
    bpp_stmt_module
    bpp_stmt_task
    bpp_stmt_help
    bpp_stmt_infra
    bpp_stmt_compat
    bpp_stmt_dialect
    bpp_stmt_txn
    bpp_stmt_event
    bpp_stmt_debug
    bpp_stmt_io_devices
    bpp_stmt_bgi
)

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
    vm/vm.c
    vm/vm_stack.c
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
    runtime/task.c
    runtime/metadata.c
    runtime/funcreg.c
    runtime/map.c
    runtime/map_serialize.c
    runtime/crypto.c
    runtime/string_ext.c
    runtime/array_sort.c
    module/mod_mathext.c
    statements/stmt_print.c
    statements/stmt_let.c
    statements/stmt_lset.c
    statements/stmt_input.c
    statements/stmt_beep.c
    statements/stmt_flow.c
    statements/stmt_program.c
    statements/stmt_loop.c
    statements/stmt_dim.c
    statements/stmt_def.c
    statements/stmt_data.c
    statements/stmt_error.c
    statements/stmt_try.c
    statements/stmt_struct.c
    statements/stmt_selftest.c
    statements/stmt_file.c
    statements/stmt_filemgmt.c
    statements/stmt_select.c
    statements/stmt_sub.c
    statements/stmt_security.c
    statements/stmt_module.c
    statements/stmt_task.c
    statements/stmt_mat.c
    statements/stmt_arrayext.c
    module/mod_arrayext.c
    statements/stmt_help.c
    statements/stmt_infra.c
    statements/stmt_defseg.c
    statements/stmt_compat.c
    statements/stmt_dialect.c
    statements/stmt_txn.c
    statements/stmt_event.c
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
    statements/stmt_bios.c
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
    statements/stmt_debug.c
    statements/stmt_io_devices.c
    runtime/state.c
    platform/platform.c
    device/vdev_fujinet.c
    runtime/gemini.c
    device/bgi/bgi_core.c
    device/bgi/bgi_raster.c
    device/bgi/bgi_font.c
    device/bgi/bgi_modes.c
    statements/stmt_bgi.c
)

target_include_directories(libbasicpp PUBLIC ../include)

add_library(libbasicpp_lite STATIC
    bootstrap/boot.c
    vm/vm.c
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
    runtime/task.c
    runtime/metadata.c
    runtime/funcreg.c
    runtime/map.c
    runtime/map_serialize.c
    runtime/crypto.c
    runtime/string_ext.c
    runtime/array_sort.c
    statements/stmt_print.c
    statements/stmt_let.c
    statements/stmt_lset.c
    statements/stmt_input.c
    statements/stmt_beep.c
    statements/stmt_flow.c
    statements/stmt_program.c
    statements/stmt_loop.c
    statements/stmt_dim.c
    statements/stmt_def.c
    statements/stmt_data.c
    statements/stmt_error.c
    statements/stmt_try.c
    statements/stmt_selftest.c
    statements/stmt_file.c
    statements/stmt_filemgmt.c
    statements/stmt_select.c
    statements/stmt_sub.c
    statements/stmt_module.c
    statements/stmt_task.c
    statements/stmt_help.c
    statements/stmt_infra.c
    statements/stmt_compat.c
    statements/stmt_dialect.c
    statements/stmt_txn.c
    statements/stmt_event.c
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
    statements/stmt_debug.c
    statements/stmt_io_devices.c
    runtime/state.c
    platform/platform.c
    device/bgi/bgi_core.c
    device/bgi/bgi_raster.c
    device/bgi/bgi_font.c
    device/bgi/bgi_modes.c
    statements/stmt_bgi.c
)
target_include_directories(libbasicpp_lite PUBLIC ../include)
target_compile_definitions(libbasicpp_lite PRIVATE BPP_LITE_BUILD NO_SDL2)

if(UNIX)
    target_compile_definitions(libbasicpp PUBLIC _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_compile_definitions(libbasicpp_lite PUBLIC _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    target_link_libraries(libbasicpp PUBLIC m ncurses)
    target_link_libraries(libbasicpp_lite PUBLIC m)
endif()

if(WIN32)
    target_link_libraries(libbasicpp PUBLIC ws2_32)
    target_link_libraries(libbasicpp_lite PUBLIC ws2_32)
endif()

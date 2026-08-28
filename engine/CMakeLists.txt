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
#   Source files use #include "vm/vm.h" not #include "vm.h".
# - What to do if something breaks: Check that include_directories
#   points to engine/include/ and all subdirectories exist.
#
# =====================================================================

cmake_minimum_required(VERSION 3.16)

# Set include path relative to engine/
set(ENGINE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/iot/include)
include_directories(${ENGINE_INCLUDE_DIR})

# Also expose aalib private headers for BGI subsystem
set(AALIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/device/bgi/aalib/include)

# --- IoT & Microcontroller Subsystem ---
add_subdirectory(iot)

# =====================================================================
# Modular Static Libraries Architecture System
# =====================================================================

# --- 0. Core Freestanding Runtime & HAL Subsystem Libraries ---
add_library(libcore_runtime_ctype STATIC src/runtime/ctype/ctype.c)
target_include_directories(libcore_runtime_ctype PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(libcore_runtime_string STATIC src/runtime/string/memops.c src/runtime/string/strops.c)
target_include_directories(libcore_runtime_string PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(libcore_runtime_memory STATIC src/runtime/memory/alloc.c src/runtime/memory/arena.c)
target_include_directories(libcore_runtime_memory PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(libcore_runtime_conv STATIC src/runtime/conv/num_parse.c src/runtime/conv/float_parse.c)
target_include_directories(libcore_runtime_conv PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(libcore_runtime_math STATIC src/runtime/math/basic.c src/runtime/math/trig.c src/runtime/math/algebra.c)
target_include_directories(libcore_runtime_math PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(libcore_runtime_format STATIC src/runtime/format/snprintf.c src/runtime/format/sscanf.c)
target_include_directories(libcore_runtime_format PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(libcore_runtime_sort STATIC src/runtime/sort/qsort.c)
target_include_directories(libcore_runtime_sort PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(libcore_runtime_time STATIC src/runtime/time/calendar.c)
target_include_directories(libcore_runtime_time PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(libcore_runtime STATIC
    src/runtime/ctype/ctype.c
    src/runtime/string/memops.c
    src/runtime/string/strops.c
    src/runtime/memory/alloc.c
    src/runtime/memory/arena.c
    src/runtime/conv/num_parse.c
    src/runtime/conv/float_parse.c
    src/runtime/math/basic.c
    src/runtime/math/trig.c
    src/runtime/math/algebra.c
    src/runtime/format/snprintf.c
    src/runtime/format/sscanf.c
    src/runtime/sort/qsort.c
    src/runtime/time/calendar.c
)
target_include_directories(libcore_runtime PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(libhal STATIC
    src/hal/hal_freestanding.c
    src/hal/hal_hosted.c
    src/hal/hal_sdl2.c
)
target_include_directories(libhal PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libhal PUBLIC libcore_runtime)

# --- 1. Boot Subsystem Library: libboot ---
add_library(boot_common OBJECT src/bootstrap/common/common.c)
target_include_directories(boot_common PUBLIC ${ENGINE_INCLUDE_DIR})
target_compile_definitions(boot_common PUBLIC BASIC_CORE_BUILD)

add_library(boot_reg_stmts OBJECT src/bootstrap/common/common_reg_stmts.c)
target_include_directories(boot_reg_stmts PUBLIC ${ENGINE_INCLUDE_DIR})
target_compile_definitions(boot_reg_stmts PUBLIC BASIC_CORE_BUILD)

add_library(boot_reg_funcs OBJECT src/bootstrap/common/common_reg_funcs.c)
target_include_directories(boot_reg_funcs PUBLIC ${ENGINE_INCLUDE_DIR})
target_compile_definitions(boot_reg_funcs PUBLIC BASIC_CORE_BUILD)

add_library(boot_lifecycle OBJECT src/bootstrap/common/common_boot.c)
target_include_directories(boot_lifecycle PUBLIC ${ENGINE_INCLUDE_DIR})
target_compile_definitions(boot_lifecycle PUBLIC BASIC_CORE_BUILD)

add_library(libboot STATIC
    $<TARGET_OBJECTS:boot_common>
    $<TARGET_OBJECTS:boot_reg_stmts>
    $<TARGET_OBJECTS:boot_reg_funcs>
    $<TARGET_OBJECTS:boot_lifecycle>
)
target_include_directories(libboot PUBLIC ${ENGINE_INCLUDE_DIR})
target_compile_definitions(libboot PUBLIC BASIC_CORE_BUILD)
target_link_libraries(libboot PUBLIC libcore_runtime libhal)

# --- 2. OS Platform Abstraction Subsystem Library: libplatform ---
add_library(libplatform STATIC
    lib/platform/plat_console.c
    lib/platform/plat_fs.c
    lib/platform/plat_sys.c
    lib/platform/plat_time.c
    lib/platform/plat_thread.c
    lib/platform/plat_dl.c
    lib/platform/plat_net.c
    lib/platform/plat_regex.c
    lib/platform/plat_clipboard.c
)
target_include_directories(libplatform PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libplatform PUBLIC libboot)

# Platform Variant Targets
add_library(libplatform_win32 ALIAS libplatform)
add_library(libplatform_posix ALIAS libplatform)
add_library(libplatform_dos ALIAS libplatform)
add_library(libplatform_mcu ALIAS libplatform)

# --- 3. Kernel Core Subsystem Library: libkernel ---
add_library(libkernel STATIC
    src/bios/bios.c
    src/bios/bios_at.c
    src/bios/bios_cpu8086.c
    src/bios/bios_hal_vm.c
    src/bios/bios_int10.c
    src/bios/bios_int13.c
    src/bios/bios_int16.c
    src/bios/bios_int1a.c
    src/bios/bios_jr.c
    src/bios/bios_pc.c
    src/bios/bios_xt.c
    src/core/bpp_api.c
    src/core/dialect.c
    src/core/feature_reg.c
    src/core/struct.c
    src/debug/logger.c
    src/device/bus.c
    src/device/console.c
    src/device/mux.c
    src/device/vcon.c
    src/device/vdev.c
    src/device/vdev_esp32.c
    src/device/pdf_writer.c
    src/device/vprinter.c
    src/device/bgi_bridge.c
    src/device/bgi_text.c
    src/device/bgi/bgi_palette.c
    src/device/bgi/bgi_gfx.c
    src/device/bgi/bgi_autodetect.c
    src/lexer/lexer.c
    src/lexer/scan_keyword.c
    src/lexer/scan_number.c
    src/lexer/scan_string.c
    src/memory/mem_system.c
    src/runtime/arrays.c
    src/runtime/arrays/arr_core.c
    src/runtime/arrays/arr_access.c
    src/runtime/arrays/arr_persist.c
    src/runtime/map.c
    src/runtime/metadata.c
    src/security/security.c
    src/vm/context.c
    src/vm/control.c
    src/vm/data.c
    src/vm/error.c
    src/vm/events.c
    src/vm/events/events_alarm.c
    src/vm/events/events_poll.c
    src/vm/events/events_trap.c
    src/vm/host.c
    src/vm/math.c
    src/vm/stack.c
)
target_include_directories(libkernel PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libkernel PUBLIC libplatform libiot)

# --- 4. Language Runtime Engine Subsystem Library: libengine ---
add_library(libengine STATIC
    src/eval/ast.c
    src/eval/ast/ast_create.c
    src/eval/ast/ast_eval_expr.c
    src/eval/ast/ast_eval_stmt.c
    src/eval/ast/ast_parser.c
    src/eval/ast/parser/ast_parse_block.c
    src/eval/ast/parser/ast_parse_expr.c
    src/eval/ast/parser/ast_parse_stmt.c
    src/eval/dispatch.c
    src/eval/dispatch/dispatch_call.c
    src/eval/dispatch/dispatch_check.c
    src/eval/dispatch/dispatch_special.c
    src/eval/eval.c
    src/eval/expression/eval_ident.c
    src/eval/expression/eval_array.c
    src/eval/expression/eval_op.c
    src/eval/expression/eval_new.c
    src/eval/eval_builtins.c
    src/eval/builtins/math_fn.c
    src/eval/builtins/string_fn.c
    src/eval/builtins/sys_fn.c
    src/eval/builtins/conversion_fn.c
    src/eval/helpers.c
    src/eval/microplex.c
    src/eval/ops.c
    src/eval/rpn.c
    src/eval/stack.c
    src/eval/type.c
    src/module/arrayext.c
    src/module/mathext.c
    src/module/module.c
    src/parser/parser.c
    src/runtime/array_sort.c
    src/runtime/funcreg.c
    src/runtime/keyword_props.c
    src/runtime/map_serialize.c
    src/runtime/micro_lib_metadata.c
    src/runtime/mux.c
    src/runtime/num_format.c
    src/runtime/override.c
    src/eval/functions/datetime/pds_datetime.c
    src/eval/functions/system/environment/pds_sys.c
    src/eval/functions/system/environment/fre.c
    src/runtime/print_using.c
    src/runtime/spec.c
    src/runtime/state.c
    src/runtime/string_ext.c
    src/runtime/strings.c
    src/runtime/using_engine.c
    src/runtime/variables.c
    src/runtime/variables/var_lookup.c
    src/runtime/variables/var_magic.c
    src/runtime/variables/var_scope.c
    src/scope/scope.c
    src/vm/exec.c
    src/vm/exec/exec_dispatch.c
    src/vm/exec/exec_control.c
    src/vm/exec/control/exec_line.c
    src/vm/exec/control/exec_postfix.c
    src/vm/exec/control/exec_stmt.c
    src/vm/exec/exec_interrupt.c
)
target_include_directories(libengine PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libengine PUBLIC libkernel)

# --- 5. Hardware Subsystem Library: libhardware ---
add_library(libhardware STATIC
    src/device/bgi/bgi_autodetect.c
    src/device/bgi/bgi_core.c
    src/device/bgi/bgi_font.c
    src/device/bgi/bgi_modes.c
    src/device/bgi/bgi_palette.c
    src/device/bgi/bgi_raster.c
    src/device/fujinet.c
    src/memory/segmented_mem.c
    src/runtime/microplex.c
)
target_include_directories(libhardware PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libhardware PUBLIC libengine)

# --- 6. Cloud Server & Networking Subsystem Library: libserver ---
add_library(libserver STATIC
    src/runtime/crypto.c
    src/runtime/gemini.c
    src/runtime/gopher.c
    src/runtime/tnfs.c
    src/runtime/task.c
    src/runtime/vfs.c
    src/runtime/vnet.c
    src/module/regex.c
)
target_include_directories(libserver PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libserver PUBLIC libhardware)

# --- 7. Batch Script Runner Subsystem Library: libscript ---
add_library(libscript STATIC
    src/runtime/file.c
    src/runtime/file/file_channel.c
    src/runtime/file/file_io.c
    src/runtime/file/file_record.c
    src/runtime/file/file_txn.c
)
target_include_directories(libscript PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libscript PUBLIC libserver)

# --- 8. Core REPL & Introspection Subsystem Library: libcore ---
add_library(libcore STATIC
    src/docgen/docgen.c
    src/statements/dialect/meta/category.c
    src/statements/dialect/meta/help.c
    src/statements/dialect/meta/introspection.c
    src/statements/dialect/meta/selftest.c
    src/eval/functions/system/environment/category.c
)

target_include_directories(libcore PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libcore PUBLIC libscript)

# --- 9. Dynamic Metaprogramming & Flex Subsystem Library: libflex ---
add_library(libflex STATIC
    src/statements/dialect/meta/alias.c
    src/statements/dialect/meta/keyword.c
    src/statements/dialect/meta/override.c
    src/statements/dialect/meta/remove.c
    src/statements/dialect/meta/scope.c
)
target_include_directories(libflex PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libflex PUBLIC libcore)

# --- 10. Standard TUI Workstation & Editor Subsystem Library: libstandard ---
add_library(libstandard STATIC
    src/debug/dap_server.c
    src/editor/edit.c
    src/editor/edit/edit_term.c
    src/editor/edit/edit_buf.c
    src/editor/edit/edit_render.c
    src/editor/edit/edit_menu.c
    src/editor/edit/edit_cmd.c
    src/editor/editor.c
    src/editor/editor_manager.c
    src/editor/edlin.c
    src/editor/edlin/edlin_buf.c
    src/editor/edlin/edlin_cmd.c
    src/editor/edlin/edlin_exec.c
    src/editor/tui_multiplexer.c
    src/editor/vi.c
    src/editor/vi/vi_term.c
    src/editor/vi/vi_buf.c
    src/editor/vi/vi_render.c
    src/editor/vi/vi_cmd.c
    src/editor/ws.c
    src/editor/ws/ws_term.c
    src/editor/ws/ws_buf.c
    src/editor/ws/ws_render.c
    src/editor/ws/ws_cmd.c
)
target_include_directories(libstandard PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libstandard PUBLIC libflex)

# --- 11. Desktop Visual & Multimedia Subsystem Library: libadvanced ---
add_library(libadvanced STATIC
    src/device/bgi/aalib/aalib.c
    src/device/bgi/bgi_core.c
    src/device/bgi/bgi_font.c
    src/device/bgi/bgi_modes.c
    src/device/bgi/bgi_raster.c
    src/device/gfx.c
    src/device/gfx/gfx_palette.c
    src/device/gfx/gfx_tui.c
    src/device/gfx/gfx_audio.c
    src/device/gfx/gfx_primitives.c
    src/device/gfx/primitives/gfx_draw_core.c
    src/device/gfx/primitives/gfx_stmt_shapes.c
    src/device/gfx/primitives/gfx_stmt_retro.c
)
target_include_directories(libadvanced PUBLIC ${ENGINE_INCLUDE_DIR} ${AALIB_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})
target_compile_definitions(libadvanced PUBLIC BASIC_STANDARD_BUILD BGI_HAVE_AALIB)
target_link_libraries(libadvanced PUBLIC libstandard)

# Parallel Peer Variant Aliases
add_library(libadvanced_sdl2 ALIAS libadvanced)
add_library(libadvanced_bgi ALIAS libadvanced)

# --- 12. Open-Ended Extension Library Template: libext ---
add_library(libext STATIC
    src/runtime/spec.c
)
target_include_directories(libext PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libext PUBLIC libadvanced)

# =====================================================================
# OS & System Library Definitions for Modular Libraries
# =====================================================================
if(UNIX)
    set(ALL_MODULAR_LIBS libboot libplatform libkernel libengine libhardware libserver libscript libcore libflex libstandard libadvanced libext)
    foreach(tgt ${ALL_MODULAR_LIBS})
        target_compile_definitions(${tgt} PUBLIC _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE)
    endforeach()
    target_link_libraries(libplatform PUBLIC m)
    target_link_libraries(libstandard PUBLIC ncurses)
endif()

if(WIN32)
    target_link_libraries(libplatform PUBLIC ws2_32)
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
add_library(vm_context STATIC src/vm/context.c)
target_include_directories(vm_context PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_exec STATIC
    src/vm/exec.c
    src/vm/exec/exec_dispatch.c
    src/vm/exec/exec_control.c
    src/vm/exec/control/exec_line.c
    src/vm/exec/control/exec_postfix.c
    src/vm/exec/control/exec_stmt.c
    src/vm/exec/exec_interrupt.c
)
target_include_directories(vm_exec PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_data STATIC src/vm/data.c)
target_include_directories(vm_data PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_error STATIC src/vm/error.c)
target_include_directories(vm_error PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_control STATIC src/vm/control.c)
target_include_directories(vm_control PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_events STATIC
    src/vm/events.c
    src/vm/events/events_alarm.c
    src/vm/events/events_poll.c
    src/vm/events/events_trap.c
)
target_include_directories(vm_events PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_math STATIC src/vm/math.c)
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
###############################################################################
# All Dynamic Micro-Libraries (Funcs & Statements)
###############################################################################

add_library(bitcount OBJECT src/eval/functions/bits/manipulation/bitcount.c)
target_include_directories(bitcount PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(readbit OBJECT src/eval/functions/bits/manipulation/readbit.c)
target_include_directories(readbit PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(resetbit OBJECT src/eval/functions/bits/manipulation/resetbit.c)
target_include_directories(resetbit PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(setbit OBJECT src/eval/functions/bits/manipulation/setbit.c)
target_include_directories(setbit PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(shl OBJECT src/eval/functions/bits/shift/shl.c)
target_include_directories(shl PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(shr OBJECT src/eval/functions/bits/shift/shr.c)
target_include_directories(shr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(togglebit OBJECT src/eval/functions/bits/manipulation/togglebit.c)
target_include_directories(togglebit PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_and OBJECT src/eval/functions/bits/logic/and.c)
target_include_directories(func_and PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_or OBJECT src/eval/functions/bits/logic/or.c)
target_include_directories(func_or PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_xor OBJECT src/eval/functions/bits/logic/xor.c)
target_include_directories(func_xor PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_imp OBJECT src/eval/functions/bits/logic/imp.c)
target_include_directories(func_imp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_eqv OBJECT src/eval/functions/bits/logic/eqv.c)
target_include_directories(func_eqv PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_not OBJECT src/eval/functions/bits/logic/not.c)
target_include_directories(func_not PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(abs OBJECT src/eval/functions/math/algebra/abs.c)
target_include_directories(abs PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(acos OBJECT src/eval/functions/math/trig/acos.c)
target_include_directories(acos PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(asin OBJECT src/eval/functions/math/trig/asin.c)
target_include_directories(asin PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(atan2 OBJECT src/eval/functions/math/trig/atan2.c)
target_include_directories(atan2 PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(atn OBJECT src/eval/functions/math/trig/atn.c)
target_include_directories(atn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(ceil OBJECT src/eval/functions/math/algebra/ceil.c)
target_include_directories(ceil PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(clamp OBJECT src/eval/functions/math/algebra/clamp.c)
target_include_directories(clamp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(cos OBJECT src/eval/functions/math/trig/cos.c)
target_include_directories(cos PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(exp OBJECT src/eval/functions/math/algebra/exp.c)
target_include_directories(exp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(fix OBJECT src/eval/functions/math/algebra/fix.c)
target_include_directories(fix PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(floor OBJECT src/eval/functions/math/algebra/floor.c)
target_include_directories(floor PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(int OBJECT src/eval/functions/math/algebra/int.c)
target_include_directories(int PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(lerp OBJECT src/eval/functions/math/algebra/lerp.c)
target_include_directories(lerp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(log OBJECT src/eval/functions/math/algebra/log.c)
target_include_directories(log PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(pi OBJECT src/eval/functions/math/trig/pi.c)
target_include_directories(pi PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(rnd OBJECT src/eval/functions/math/random/rnd.c)
target_include_directories(rnd PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(round OBJECT src/eval/functions/math/algebra/round.c)
target_include_directories(round PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(sgn OBJECT src/eval/functions/math/algebra/sgn.c)
target_include_directories(sgn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(sin OBJECT src/eval/functions/math/trig/sin.c)
target_include_directories(sin PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(sqr OBJECT src/eval/functions/math/algebra/sqr.c)
target_include_directories(sqr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(tan OBJECT src/eval/functions/math/trig/tan.c)
target_include_directories(tan PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(eps OBJECT src/eval/functions/math/algebra/eps.c)
target_include_directories(eps PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(inf OBJECT src/eval/functions/math/algebra/inf.c)
target_include_directories(inf PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(maxnum OBJECT src/eval/functions/math/algebra/maxnum.c)
target_include_directories(maxnum PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(degrees OBJECT src/eval/functions/math/trig/degrees.c)
target_include_directories(degrees PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(radians OBJECT src/eval/functions/math/trig/radians.c)
target_include_directories(radians PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(remainder OBJECT src/eval/functions/math/algebra/remainder.c)
target_include_directories(remainder PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(angle OBJECT src/eval/functions/math/trig/angle.c)
target_include_directories(angle PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(truncate OBJECT src/eval/functions/math/algebra/truncate.c)
target_include_directories(truncate PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_min OBJECT src/eval/functions/math/algebra/min.c)
target_include_directories(func_min PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_max OBJECT src/eval/functions/math/algebra/max.c)
target_include_directories(func_max PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_hypot OBJECT src/eval/functions/math/trig/hypot.c)
target_include_directories(func_hypot PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_mod OBJECT src/eval/functions/math/algebra/mod.c)
target_include_directories(func_mod PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_sinh OBJECT src/eval/functions/math/trig/sinh.c)
target_include_directories(func_sinh PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_cosh OBJECT src/eval/functions/math/trig/cosh.c)
target_include_directories(func_cosh PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_tanh OBJECT src/eval/functions/math/trig/tanh.c)
target_include_directories(func_tanh PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_log10 OBJECT src/eval/functions/math/algebra/log10.c)
target_include_directories(func_log10 PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_log2 OBJECT src/eval/functions/math/algebra/log2.c)
target_include_directories(func_log2 PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_cot OBJECT src/eval/functions/math/trig/cot.c)
target_include_directories(func_cot PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_sec OBJECT src/eval/functions/math/trig/sec.c)
target_include_directories(func_sec PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_csc OBJECT src/eval/functions/math/trig/csc.c)
target_include_directories(func_csc PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_comp OBJECT src/eval/functions/math/linear_algebra/comp.c)
target_include_directories(func_comp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_pdif OBJECT src/eval/functions/math/algebra/pdif.c)
target_include_directories(func_pdif PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(bin OBJECT src/eval/functions/string/conversion/bin.c)
target_include_directories(bin PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(chr OBJECT src/eval/functions/string/conversion/chr.c)
target_include_directories(chr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(hex OBJECT src/eval/functions/string/conversion/hex.c)
target_include_directories(hex PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(instr OBJECT src/eval/functions/string/search/instr.c)
target_include_directories(instr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(lcase OBJECT src/eval/functions/string/manipulation/lcase.c)
target_include_directories(lcase PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(left OBJECT src/eval/functions/string/manipulation/left.c)
target_include_directories(left PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(len OBJECT src/eval/functions/string/format/len.c)
target_include_directories(len PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(ltrim OBJECT src/eval/functions/string/manipulation/ltrim.c)
target_include_directories(ltrim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(mid OBJECT src/eval/functions/string/manipulation/mid.c)
target_include_directories(mid PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(oct OBJECT src/eval/functions/string/conversion/oct.c)
target_include_directories(oct PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(pack OBJECT src/eval/functions/string/manipulation/pack.c)
target_include_directories(pack PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(right OBJECT src/eval/functions/string/manipulation/right.c)
target_include_directories(right PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(rtrim OBJECT src/eval/functions/string/manipulation/rtrim.c)
target_include_directories(rtrim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(space OBJECT src/eval/functions/string/manipulation/space.c)
target_include_directories(space PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(str OBJECT src/eval/functions/string/conversion/str.c)
target_include_directories(str PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(string OBJECT src/eval/functions/string/manipulation/string.c)
target_include_directories(string PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(trim OBJECT src/eval/functions/string/manipulation/trim.c)
target_include_directories(trim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(ucase OBJECT src/eval/functions/string/manipulation/ucase.c)
target_include_directories(ucase PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(shuffle OBJECT src/eval/functions/string/manipulation/shuffle.c)
target_include_directories(shuffle PUBLIC ${ENGINE_INCLUDE_DIR})


add_library(unpack OBJECT src/eval/functions/string/manipulation/unpack.c)
target_include_directories(unpack PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(val OBJECT src/eval/functions/string/conversion/val.c)
target_include_directories(val PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(clock_num OBJECT src/eval/functions/system/time/clock_num.c)
target_include_directories(clock_num PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(clock_str OBJECT src/eval/functions/system/time/clock_str.c)
target_include_directories(clock_str PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(date OBJECT src/eval/functions/system/time/date.c)
target_include_directories(date PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(environ OBJECT src/eval/functions/system/environment/environ.c)
target_include_directories(environ PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(fre OBJECT src/eval/functions/system/environment/fre.c)
target_include_directories(fre PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(inkey OBJECT src/eval/functions/system/terminal/inkey.c)
target_include_directories(inkey PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(inp OBJECT src/eval/functions/system/hardware/inp.c)
target_include_directories(inp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(peek OBJECT src/eval/functions/system/hardware/peek.c)
target_include_directories(peek PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(ticks OBJECT src/eval/functions/system/time/ticks.c)
target_include_directories(ticks PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(time OBJECT src/eval/functions/system/time/time.c)
target_include_directories(time PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(timer OBJECT src/eval/functions/system/time/timer.c)
target_include_directories(timer PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_dread OBJECT src/eval/functions/system/hardware/func_dread.c)
target_include_directories(func_dread PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_aread OBJECT src/eval/functions/system/hardware/func_aread.c)
target_include_directories(func_aread PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_touch OBJECT src/eval/functions/system/hardware/func_touch.c)
target_include_directories(func_touch PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_hall OBJECT src/eval/functions/system/hardware/func_hall.c)
target_include_directories(func_hall PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_ticks_iot OBJECT src/eval/functions/system/hardware/func_ticks.c)
target_include_directories(func_ticks_iot PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_mem OBJECT src/eval/functions/system/func_mem.c)
target_include_directories(func_mem PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_http OBJECT src/eval/functions/system/hardware/func_http.c)
target_include_directories(func_http PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_gemini OBJECT src/eval/functions/system/hardware/func_gemini.c)
target_include_directories(func_gemini PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_gopher OBJECT src/eval/functions/system/hardware/func_gopher.c)
target_include_directories(func_gopher PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_python OBJECT src/eval/functions/system/func_python.c)
target_include_directories(func_python PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_end OBJECT src/statements/core/program/end.c)
target_include_directories(stmt_end PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_gosub OBJECT src/statements/core/program/gosub.c)
target_include_directories(stmt_gosub PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_goto OBJECT src/statements/core/program/goto.c)
target_include_directories(stmt_goto PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_if OBJECT src/statements/core/program/if.c)
target_include_directories(stmt_if PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_input OBJECT src/statements/core/io/input.c)
target_include_directories(stmt_input PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_print OBJECT src/statements/core/io/print.c)
target_include_directories(stmt_print PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_randomize OBJECT src/statements/core/program/randomize.c)
target_include_directories(stmt_randomize PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_rem OBJECT src/statements/core/program/rem.c)
target_include_directories(stmt_rem PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_return OBJECT src/statements/core/program/return.c)
target_include_directories(stmt_return PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_select OBJECT src/statements/core/program/select.c)
target_include_directories(stmt_select PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_stop OBJECT src/statements/core/program/stop.c)
target_include_directories(stmt_stop PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_do OBJECT src/statements/loops/conditional/do.c)
target_include_directories(stmt_do PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_exit_loop OBJECT src/statements/loops/conditional/exit_loop.c)
target_include_directories(stmt_exit_loop PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_for OBJECT src/statements/loops/counter/for.c)
target_include_directories(stmt_for PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_loop OBJECT src/statements/loops/conditional/loop.c)
target_include_directories(stmt_loop PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_next OBJECT src/statements/loops/counter/next.c)
target_include_directories(stmt_next PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_wend OBJECT src/statements/loops/conditional/wend.c)
target_include_directories(stmt_wend PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_while OBJECT src/statements/loops/conditional/while.c)
target_include_directories(stmt_while PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_chain OBJECT src/statements/program/chain.c)
target_include_directories(stmt_chain PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_clear OBJECT src/statements/program/clear.c)
target_include_directories(stmt_clear PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_cont OBJECT src/statements/program/cont.c)
target_include_directories(stmt_cont PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_list OBJECT src/statements/program/list.c)
target_include_directories(stmt_list PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_llist OBJECT src/statements/program/llist.c)
target_include_directories(stmt_llist PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_reformat OBJECT
    src/statements/program/reformat.c
    src/statements/program/reformat_engine.c
    src/statements/program/reformat/reformat_analyze.c
    src/statements/program/reformat/reformat_indent.c
    src/statements/program/reformat/reformat_report.c
)
target_include_directories(stmt_reformat PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(debug_analyzer OBJECT src/debug/analyzer.c)
target_include_directories(debug_analyzer PUBLIC ${ENGINE_INCLUDE_DIR})
target_compile_definitions(debug_analyzer PRIVATE $<$<BOOL:${UNIX}>: _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE>)

add_library(stmt_check OBJECT src/statements/debug/diagnostics/check.c)
target_include_directories(stmt_check PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_verify OBJECT src/statements/debug/diagnostics/verify.c)
target_include_directories(stmt_verify PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_test OBJECT src/statements/debug/diagnostics/test.c)
target_include_directories(stmt_test PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_version OBJECT src/statements/system/environment/version.c)
target_include_directories(stmt_version PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_varptr OBJECT src/functions/varptr.c)
target_include_directories(func_varptr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_load OBJECT src/statements/program/load.c)
target_include_directories(stmt_load PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_new OBJECT src/statements/program/new.c)
target_include_directories(stmt_new PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_run OBJECT src/statements/program/run.c)
target_include_directories(stmt_run PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_save OBJECT src/statements/program/save.c)
target_include_directories(stmt_save PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_merge OBJECT src/statements/program/merge.c)
target_include_directories(stmt_merge PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_delete OBJECT src/statements/program/delete.c)
target_include_directories(stmt_delete PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_renum OBJECT src/statements/program/renum.c)
target_include_directories(stmt_renum PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_auto OBJECT src/statements/program/auto.c)
target_include_directories(stmt_auto PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_array_ext OBJECT src/statements/variables/assignment/array_ext.c)
target_include_directories(stmt_array_ext PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_data OBJECT src/statements/variables/data/data.c)
target_include_directories(stmt_data PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_def OBJECT src/statements/variables/declaration/def.c)
target_include_directories(stmt_def PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_dim OBJECT src/statements/variables/declaration/dim.c)
target_include_directories(stmt_dim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_let OBJECT src/statements/variables/assignment/let.c)
target_include_directories(stmt_let PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_lset OBJECT src/statements/variables/assignment/lset.c)
target_include_directories(stmt_lset PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mid_stmt OBJECT src/statements/variables/assignment/mid_stmt.c)
target_include_directories(stmt_mid_stmt PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mux OBJECT src/statements/variables/options/mux.c)
target_include_directories(stmt_mux PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_option OBJECT src/statements/variables/options/option.c)
target_include_directories(stmt_option PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_exchange OBJECT src/statements/variables/assignment/exchange.c)
target_include_directories(stmt_exchange PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_perform OBJECT src/statements/system/environment/perform.c)
target_include_directories(stmt_perform PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_invoke OBJECT src/statements/system/environment/invoke.c)
target_include_directories(stmt_invoke PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_prefix OBJECT src/statements/filesystem/binary_ops/prefix.c)
target_include_directories(stmt_prefix PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_repeat OBJECT src/statements/loops/conditional/repeat.c)
target_include_directories(stmt_repeat PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_until OBJECT src/statements/loops/conditional/until.c)
target_include_directories(stmt_until PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_endloop OBJECT src/statements/loops/conditional/endloop.c)
target_include_directories(stmt_endloop PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_destroy OBJECT src/statements/filesystem/binary_ops/destroy.c)
target_include_directories(stmt_destroy PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_create OBJECT src/statements/filesystem/binary_ops/create.c)
target_include_directories(stmt_create PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_param OBJECT src/statements/oop/structure/param.c)
target_include_directories(stmt_param PUBLIC ${ENGINE_INCLUDE_DIR})



add_library(stmt_bload OBJECT src/statements/filesystem/binary_ops/bload.c)
target_include_directories(stmt_bload PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_close OBJECT src/statements/filesystem/file_ops/close.c)
target_include_directories(stmt_close PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_field OBJECT src/statements/filesystem/file_ops/field.c)
target_include_directories(stmt_field PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_files OBJECT src/statements/filesystem/dir_ops/files.c)
target_include_directories(stmt_files PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_get OBJECT src/statements/filesystem/file_ops/get.c)
target_include_directories(stmt_get PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_input_file OBJECT src/statements/filesystem/file_ops/input_file.c)
target_include_directories(stmt_input_file PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_kill OBJECT src/statements/filesystem/dir_ops/kill.c)
target_include_directories(stmt_kill PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_lock OBJECT src/statements/filesystem/file_ops/lock.c)
target_include_directories(stmt_lock PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mkdir OBJECT src/statements/filesystem/dir_ops/mkdir.c)
target_include_directories(stmt_mkdir PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_name OBJECT src/statements/filesystem/dir_ops/name.c)
target_include_directories(stmt_name PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_open OBJECT src/statements/filesystem/file_ops/open.c)
target_include_directories(stmt_open PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_print_file OBJECT src/statements/filesystem/file_ops/print_file.c)
target_include_directories(stmt_print_file PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_put OBJECT src/statements/filesystem/file_ops/put.c)
target_include_directories(stmt_put PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_seek OBJECT src/statements/filesystem/file_ops/seek.c)
target_include_directories(stmt_seek PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_write_file OBJECT src/statements/filesystem/file_ops/write_file.c)
target_include_directories(stmt_write_file PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_vdim OBJECT src/statements/filesystem/binary_ops/vdim.c)
target_include_directories(stmt_vdim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_whenever OBJECT src/statements/event/trapping/whenever.c)
target_include_directories(stmt_whenever PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_beep OBJECT src/statements/sound/synthesis/beep.c)
target_include_directories(stmt_beep PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_play OBJECT src/statements/sound/synthesis/play.c)
target_include_directories(stmt_play PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_sound OBJECT src/statements/sound/synthesis/sound.c)
target_include_directories(stmt_sound PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_voice OBJECT src/statements/sound/synthesis/voice.c)
target_include_directories(stmt_voice PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_bgi OBJECT src/statements/graphics/bgi.c)
target_include_directories(stmt_bgi PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_circle OBJECT src/statements/graphics/draw/circle.c)
target_include_directories(stmt_circle PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_cls OBJECT src/statements/graphics/screen/cls.c)
target_include_directories(stmt_cls PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_home OBJECT src/statements/graphics/stmt_home.c)
target_include_directories(stmt_home PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_color OBJECT src/statements/graphics/screen/color.c)
target_include_directories(stmt_color PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_compat OBJECT src/statements/graphics/compat.c)
target_include_directories(stmt_compat PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_draw OBJECT src/statements/graphics/draw/draw.c)
target_include_directories(stmt_draw PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_line OBJECT src/statements/graphics/draw/line.c)
target_include_directories(stmt_line PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_paint OBJECT src/statements/graphics/draw/paint.c)
target_include_directories(stmt_paint PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_palette OBJECT src/statements/graphics/screen/palette.c)
target_include_directories(stmt_palette PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_preset OBJECT src/statements/graphics/draw/preset.c)
target_include_directories(stmt_preset PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_pset OBJECT src/statements/graphics/draw/pset.c)
target_include_directories(stmt_pset PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_screen OBJECT src/statements/graphics/screen/screen.c)
target_include_directories(stmt_screen PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_view OBJECT src/statements/graphics/screen/view.c)
target_include_directories(stmt_view PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_window OBJECT src/statements/graphics/screen/window.c)
target_include_directories(stmt_window PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_plot OBJECT src/statements/graphics/draw/stmt_plot.c)
target_include_directories(stmt_plot PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_call OBJECT src/statements/oop/call.c)
target_include_directories(stmt_call PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_class OBJECT src/statements/oop/structure/class.c)
target_include_directories(stmt_class PUBLIC ${ENGINE_INCLUDE_DIR})


add_library(stmt_enum OBJECT src/statements/oop/structure/enum.c)
target_include_directories(stmt_enum PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_function OBJECT src/statements/oop/structure/function.c)
target_include_directories(stmt_function PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_module OBJECT src/statements/oop/module.c)
target_include_directories(stmt_module PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_shared OBJECT src/statements/oop/shared.c)
target_include_directories(stmt_shared PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_share OBJECT src/statements/variables/declaration/share.c)
target_include_directories(stmt_share PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_public OBJECT src/statements/variables/declaration/public.c)
target_include_directories(stmt_public PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_global OBJECT src/statements/variables/declaration/global.c)
target_include_directories(stmt_global PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_doevents OBJECT src/statements/control/flow/doevents.c)
target_include_directories(stmt_doevents PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_msgbox OBJECT src/statements/ui/widgets/msgbox.c)
target_include_directories(stmt_msgbox PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_command OBJECT src/eval/functions/system/environment/command_fn.c)
target_include_directories(func_command PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_vbdos OBJECT src/eval/functions/ui/dialogs/vbdos_fn.c)
target_include_directories(func_vbdos PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_declare OBJECT src/statements/control/external/declare.c)
target_include_directories(stmt_declare PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_external OBJECT src/statements/control/external/external.c)
target_include_directories(stmt_external PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_when OBJECT src/statements/control/branch/when.c)
target_include_directories(stmt_when PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_handler OBJECT src/statements/control/branch/handler.c)
target_include_directories(stmt_handler PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_cause OBJECT src/statements/control/branch/cause.c)
target_include_directories(stmt_cause PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_retry OBJECT src/statements/control/flow/retry.c)
target_include_directories(stmt_retry PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_continue OBJECT src/statements/control/flow/continue.c)
target_include_directories(stmt_continue PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_ask OBJECT src/statements/io/ask.c)
target_include_directories(stmt_ask PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_viewport OBJECT src/statements/bgi/viewport.c)
target_include_directories(stmt_viewport PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_picture OBJECT src/statements/bgi/picture.c)
target_include_directories(stmt_picture PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_det OBJECT src/eval/functions/math/linear_algebra/det.c)
target_include_directories(func_det PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_dot OBJECT src/eval/functions/math/linear_algebra/dot.c)
target_include_directories(func_dot PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_cross OBJECT src/eval/functions/math/linear_algebra/cross.c)
target_include_directories(func_cross PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_sub OBJECT
    src/statements/oop/sub.c
    src/statements/oop/sub/sub_lookup.c
    src/statements/oop/sub/sub_exec.c
    src/statements/oop/sub/sub_invoke.c
)
target_include_directories(stmt_sub PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_type OBJECT src/statements/oop/structure/type.c)
target_include_directories(stmt_type PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_with OBJECT src/statements/oop/structure/with.c)
target_include_directories(stmt_with PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat_input OBJECT src/statements/matrices/io/mat_input.c)
target_include_directories(stmt_mat_input PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat_ops OBJECT
    src/statements/matrices/ops/mat_ops.c
    src/statements/matrices/mat_ops/mat_arith.c
    src/statements/matrices/mat_ops/mat_special.c
    src/statements/matrices/mat_ops/mat_transform.c
)
target_include_directories(stmt_mat_ops PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat_print OBJECT src/statements/matrices/io/mat_print.c)
target_include_directories(stmt_mat_print PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat_read OBJECT src/statements/matrices/io/mat_read.c)
target_include_directories(stmt_mat_read PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat_write OBJECT src/statements/matrices/io/mat_write.c)
target_include_directories(stmt_mat_write PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_on_com OBJECT src/statements/event/trapping/on_com.c)
target_include_directories(stmt_on_com PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_on_error OBJECT src/statements/event/trapping/on_error.c)
target_include_directories(stmt_on_error PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_on_key OBJECT src/statements/event/trapping/on_key.c)
target_include_directories(stmt_on_key PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_on_timer OBJECT src/statements/event/trapping/on_timer.c)
target_include_directories(stmt_on_timer PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_resume OBJECT src/statements/event/resume.c)
target_include_directories(stmt_resume PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_try OBJECT src/statements/event/try.c)
target_include_directories(stmt_try PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_bios OBJECT src/statements/system/bios.c)
target_include_directories(stmt_bios PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_debug OBJECT src/statements/system/debug.c)
target_include_directories(stmt_debug PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_defseg OBJECT src/statements/system/hardware/defseg.c)
target_include_directories(stmt_defseg PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_joystick OBJECT src/statements/system/joystick.c)
target_include_directories(stmt_joystick PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mouse OBJECT src/statements/system/mouse.c)
target_include_directories(stmt_mouse PUBLIC ${ENGINE_INCLUDE_DIR})


add_library(stmt_pen OBJECT src/statements/system/pen.c)
target_include_directories(stmt_pen PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_poke OBJECT src/statements/system/hardware/poke.c)
target_include_directories(stmt_poke PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_out OBJECT src/statements/system/hardware/out.c)
target_include_directories(stmt_out PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_security OBJECT src/statements/system/security.c)
target_include_directories(stmt_security PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_system OBJECT src/statements/system/system.c)
target_include_directories(stmt_system PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_goodbye OBJECT src/statements/system/goodbye.c)
target_include_directories(stmt_goodbye PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_task OBJECT src/statements/system/task.c)
target_include_directories(stmt_task PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_txn OBJECT src/statements/system/txn.c)
target_include_directories(stmt_txn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_pinmode OBJECT src/statements/system/hardware/stmt_pinmode.c)
target_include_directories(stmt_pinmode PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_dwrite OBJECT src/statements/system/hardware/stmt_dwrite.c)
target_include_directories(stmt_dwrite PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_dac OBJECT src/statements/system/hardware/stmt_dac.c)
target_include_directories(stmt_dac PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_pwm OBJECT src/statements/system/hardware/stmt_pwm.c)
target_include_directories(stmt_pwm PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_servo OBJECT src/statements/system/hardware/stmt_servo.c)
target_include_directories(stmt_servo PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_i2c OBJECT src/statements/system/hardware/stmt_i2c.c)
target_include_directories(stmt_i2c PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_spi OBJECT src/statements/system/hardware/stmt_spi.c)
target_include_directories(stmt_spi PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_neopixel OBJECT src/statements/system/hardware/stmt_neopixel.c)
target_include_directories(stmt_neopixel PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_dht OBJECT src/statements/system/hardware/stmt_dht.c)
target_include_directories(stmt_dht PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_delay OBJECT src/statements/system/hardware/stmt_delay.c)
target_include_directories(stmt_delay PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_every OBJECT src/statements/system/hardware/stmt_every.c)
target_include_directories(stmt_every PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_deepsleep OBJECT src/statements/system/stmt_deepsleep.c)
target_include_directories(stmt_deepsleep PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_reboot OBJECT src/statements/system/stmt_reboot.c)
target_include_directories(stmt_reboot PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_freq OBJECT src/statements/system/stmt_freq.c)
target_include_directories(stmt_freq PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_wifi OBJECT src/statements/system/hardware/stmt_wifi.c)
target_include_directories(stmt_wifi PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_mqtt OBJECT src/statements/system/hardware/stmt_mqtt.c)
target_include_directories(stmt_mqtt PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_webrepl OBJECT src/statements/system/hardware/stmt_webrepl.c)
target_include_directories(stmt_webrepl PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_python OBJECT src/statements/system/stmt_python.c)
target_include_directories(stmt_python PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_espnow OBJECT src/statements/system/wireless/stmt_espnow.c)
target_include_directories(stmt_espnow PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_bluetooth OBJECT src/statements/system/wireless/stmt_bluetooth.c)
target_include_directories(stmt_bluetooth PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_nfc OBJECT src/statements/system/hardware/stmt_nfc.c)
target_include_directories(stmt_nfc PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_gemini OBJECT src/statements/network/stmt_gemini.c)
target_include_directories(stmt_gemini PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_gopher OBJECT src/statements/network/stmt_gopher.c)
target_include_directories(stmt_gopher PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_tnfs OBJECT src/statements/network/stmt_tnfs.c)
target_include_directories(stmt_tnfs PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_gemini_browse OBJECT src/statements/network/stmt_gemini_browse.c)
target_include_directories(stmt_gemini_browse PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_tnfs OBJECT src/eval/functions/system/hardware/func_tnfs.c)
target_include_directories(func_tnfs PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_gemini_meta OBJECT src/eval/functions/system/hardware/func_gemini_meta.c)
target_include_directories(func_gemini_meta PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(func_fujinet OBJECT src/eval/functions/system/hardware/func_fujinet.c)
target_include_directories(func_fujinet PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(nil_compress OBJECT src/runtime/nil_compress.c)
target_include_directories(nil_compress PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(nil_bead OBJECT src/runtime/nil_bead.c)
target_include_directories(nil_bead PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(nil_transport OBJECT src/runtime/nil_transport.c)
target_include_directories(nil_transport PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(vdev_remote OBJECT src/device/vdev_remote.c)
target_include_directories(vdev_remote PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_remote OBJECT src/statements/network/stmt_remote.c)
target_include_directories(stmt_remote PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(stmt_nil_bead OBJECT src/statements/network/stmt_nil_bead.c)
target_include_directories(stmt_nil_bead PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_nil_compress OBJECT src/eval/functions/string/manipulation/func_nil_compress.c)
target_include_directories(func_nil_compress PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_nil_bead OBJECT src/eval/functions/system/hardware/func_nil_bead.c)
target_include_directories(func_nil_bead PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_remote OBJECT src/eval/functions/system/hardware/func_remote.c)
target_include_directories(func_remote PUBLIC ${ENGINE_INCLUDE_DIR} iot/include)

add_library(peer OBJECT src/runtime/peer.c)
target_include_directories(peer PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(sock_engine OBJECT src/runtime/sock_engine.c)
target_include_directories(sock_engine PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(packet_sniff OBJECT src/runtime/packet_sniff.c)
target_include_directories(packet_sniff PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(crypto_engine OBJECT src/runtime/crypto_engine.c)
target_include_directories(crypto_engine PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(events_net OBJECT src/vm/events/events_net.c)
target_include_directories(events_net PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_peer OBJECT src/statements/network/stmt_peer.c)
target_include_directories(stmt_peer PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_sock OBJECT src/statements/network/stmt_sock.c)
target_include_directories(stmt_sock PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_port_trigger OBJECT src/statements/network/stmt_port_trigger.c)
target_include_directories(stmt_port_trigger PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_sniff OBJECT src/statements/network/stmt_sniff.c)
target_include_directories(stmt_sniff PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_net_config OBJECT src/statements/network/stmt_net_config.c)
target_include_directories(stmt_net_config PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_sock OBJECT src/eval/functions/system/hardware/func_sock.c)
target_include_directories(func_sock PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_packet OBJECT src/eval/functions/system/hardware/func_packet.c)
target_include_directories(func_packet PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_crypto OBJECT src/eval/functions/system/security/func_crypto.c)
target_include_directories(func_crypto PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_help OBJECT src/statements/dialect/meta/help.c)
target_include_directories(stmt_help PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_category OBJECT src/statements/dialect/meta/category.c)
target_include_directories(stmt_category PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_category OBJECT src/eval/functions/system/environment/category.c)
target_include_directories(func_category PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_introspection OBJECT src/statements/dialect/meta/introspection.c)

target_include_directories(stmt_introspection PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_alias OBJECT src/statements/dialect/meta/alias.c)
target_include_directories(stmt_alias PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_scope OBJECT src/statements/dialect/meta/scope.c)
target_include_directories(stmt_scope PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_keyword OBJECT src/statements/dialect/meta/keyword.c)
target_include_directories(stmt_keyword PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_override OBJECT src/statements/dialect/meta/override.c)
target_include_directories(stmt_override PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_remove OBJECT src/statements/dialect/meta/remove.c)
target_include_directories(stmt_remove PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_selftest OBJECT src/statements/dialect/meta/selftest.c)
target_include_directories(stmt_selftest PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(mod_module OBJECT src/module/module.c)
target_include_directories(mod_module PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(rt_task OBJECT src/runtime/task.c)
target_include_directories(rt_task PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(rt_gemini OBJECT src/runtime/gemini.c)
target_include_directories(rt_gemini PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(microplex OBJECT src/eval/microplex.c)
target_include_directories(microplex PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_change OBJECT src/statements/variables/assignment/change.c)
target_include_directories(stmt_change PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_margin OBJECT src/statements/io/margin.c)
target_include_directories(stmt_margin PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_zone OBJECT src/statements/io/zone.c)
target_include_directories(stmt_zone PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_append OBJECT src/statements/filesystem/file_ops/append.c)
target_include_directories(stmt_append PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_modify OBJECT src/statements/filesystem/binary_ops/modify.c)
target_include_directories(stmt_modify PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_text OBJECT src/statements/filesystem/binary_ops/text.c)
target_include_directories(stmt_text PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_linput OBJECT src/statements/io/linput.c)
target_include_directories(stmt_linput PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_image OBJECT src/statements/io/image.c)
target_include_directories(stmt_image PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_form OBJECT src/statements/io/form.c)
target_include_directories(stmt_form PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_assign OBJECT src/statements/filesystem/binary_ops/assign.c)
target_include_directories(stmt_assign PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_record OBJECT src/statements/filesystem/file_ops/record.c)
target_include_directories(stmt_record PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_find OBJECT src/statements/filesystem/binary_ops/find.c)
target_include_directories(stmt_find PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_map OBJECT src/statements/core/program/map.c)
target_include_directories(stmt_map PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_incr OBJECT src/statements/variables/assignment/incr.c)
target_include_directories(stmt_incr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_clr OBJECT src/statements/variables/data/clr.c)
target_include_directories(stmt_clr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_void OBJECT src/statements/control/flow/void.c)
target_include_directories(stmt_void PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_arrayfill OBJECT src/statements/variables/assignment/arrayfill.c)
target_include_directories(stmt_arrayfill PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_sleep OBJECT src/statements/control/flow/sleep.c)
target_include_directories(stmt_sleep PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_suspend OBJECT src/statements/control/flow/suspend.c)
target_include_directories(stmt_suspend PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_key OBJECT src/statements/event/trapping/key.c)
target_include_directories(stmt_key PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_wait OBJECT src/statements/control/flow/wait.c)
target_include_directories(stmt_wait PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_echo OBJECT src/statements/io/echo.c)
target_include_directories(stmt_echo PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_extend OBJECT src/statements/control/external/extend.c)
target_include_directories(stmt_extend PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_scale OBJECT src/statements/control/external/scale.c)
target_include_directories(stmt_scale PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_enter OBJECT src/statements/io/enter.c)
target_include_directories(stmt_enter PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_cvt OBJECT src/eval/functions/string/conversion/cvt.c)
target_include_directories(func_cvt PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_rad OBJECT src/eval/functions/string/conversion/rad.c)
target_include_directories(func_rad PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_num OBJECT src/eval/functions/string/conversion/num.c)
target_include_directories(func_num PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_xlate OBJECT src/eval/functions/string/conversion/xlate.c)
target_include_directories(func_xlate PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_mag OBJECT src/eval/functions/math/algebra/mag.c)
target_include_directories(func_mag PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_sys OBJECT src/eval/functions/system/environment/sys_fn.c)
target_include_directories(func_sys PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_time_fn OBJECT src/eval/functions/system/time/time_fn.c)
target_include_directories(func_time_fn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_seg OBJECT src/eval/functions/string/manipulation/seg.c)
target_include_directories(func_seg PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_ert OBJECT src/eval/functions/string/format/ert.c)
target_include_directories(func_ert PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_ascii_fn OBJECT src/eval/functions/string/conversion/ascii_fn.c)
target_include_directories(func_ascii_fn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_ups OBJECT src/eval/functions/string/conversion/ups.c)
target_include_directories(func_ups PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_index_fn OBJECT src/eval/functions/string/search/index_fn.c)
target_include_directories(func_index_fn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_str_math OBJECT src/eval/functions/string/manipulation/str_math.c)
target_include_directories(func_str_math PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_spec_fn OBJECT src/eval/functions/system/environment/spec_fn.c)
target_include_directories(func_spec_fn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_magtape OBJECT src/eval/functions/system/hardware/magtape.c)
target_include_directories(func_magtape PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(core_complex_num OBJECT src/core/complex_num.c)
target_include_directories(core_complex_num PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_complex OBJECT src/eval/functions/math/linear_algebra/complex_fn.c)
target_include_directories(func_complex PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_complex OBJECT src/statements/variables/declaration/complex.c)
target_include_directories(stmt_complex PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_verify OBJECT src/eval/functions/string/search/verify_fn.c)
target_include_directories(func_verify PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(rt_session OBJECT src/runtime/session.c)
target_include_directories(rt_session PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_session OBJECT src/statements/system/environment/session_stmts.c)
target_include_directories(stmt_session PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_page OBJECT src/statements/io/page.c)
target_include_directories(stmt_page PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_rewind OBJECT src/statements/filesystem/binary_ops/rewind.c)
target_include_directories(stmt_rewind PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_backspace OBJECT src/statements/filesystem/binary_ops/backspace.c)
target_include_directories(stmt_backspace PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_unless OBJECT src/statements/control/branch/unless.c)
target_include_directories(stmt_unless PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_pick OBJECT src/eval/functions/string/manipulation/pick.c)
target_include_directories(func_pick PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_fid OBJECT src/eval/functions/filesystem/descriptors/fid.c)
target_include_directories(func_fid PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_ath OBJECT src/eval/functions/string/conversion/ath.c)
target_include_directories(func_ath PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_tek OBJECT src/eval/functions/string/format/tek.c)
target_include_directories(func_tek PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_fpt OBJECT src/eval/functions/math/algebra/fpt.c)
target_include_directories(func_fpt PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_mbf OBJECT src/eval/functions/string/conversion/mbf.c)
target_include_directories(func_mbf PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_financial OBJECT src/eval/functions/math/financial/financial.c)
target_include_directories(func_financial PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_vbdos_filebox OBJECT src/eval/functions/ui/dialogs/vbdos_filebox.c)
target_include_directories(func_vbdos_filebox PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_typ OBJECT src/eval/functions/filesystem/descriptors/typ.c)
target_include_directories(func_typ PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_moddir OBJECT src/eval/functions/system/environment/moddir.c)
target_include_directories(func_moddir PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_isam OBJECT src/statements/db/isam/isam.c)
target_include_directories(stmt_isam PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_vbdos_widgets OBJECT src/statements/ui/widgets/vbdos_widgets.c)
target_include_directories(stmt_vbdos_widgets PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_pds_datetime OBJECT src/eval/functions/datetime/pds_datetime.c)
target_include_directories(func_pds_datetime PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_interrupt OBJECT src/statements/system/hardware/interrupt.c)
target_include_directories(stmt_interrupt PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_vbdos_controls OBJECT src/statements/ui/widgets/vbdos_controls.c)
target_include_directories(stmt_vbdos_controls PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_const OBJECT src/statements/variables/declaration/const.c)
target_include_directories(stmt_const PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_read OBJECT src/statements/variables/data/read.c)
target_include_directories(stmt_read PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_restore OBJECT src/statements/variables/data/restore.c)
target_include_directories(stmt_restore PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_defint OBJECT src/statements/variables/declaration/defint.c)
target_include_directories(stmt_defint PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_defsng OBJECT src/statements/variables/declaration/defsng.c)
target_include_directories(stmt_defsng PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_defdbl OBJECT src/statements/variables/declaration/defdbl.c)
target_include_directories(stmt_defdbl PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_defstr OBJECT src/statements/variables/declaration/defstr.c)
target_include_directories(stmt_defstr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_deflng OBJECT src/statements/variables/declaration/deflng.c)
target_include_directories(stmt_deflng PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_common OBJECT src/statements/variables/declaration/common.c)
target_include_directories(stmt_common PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_redim OBJECT src/statements/variables/declaration/redim.c)
target_include_directories(stmt_redim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_erase OBJECT src/statements/variables/data/erase.c)
target_include_directories(stmt_erase PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_swap OBJECT src/statements/variables/assignment/swap.c)
target_include_directories(stmt_swap PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_rset OBJECT src/statements/variables/assignment/rset.c)
target_include_directories(stmt_rset PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_def_seg OBJECT src/statements/system/hardware/def_seg.c)
target_include_directories(stmt_def_seg PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_def_usr OBJECT src/statements/system/hardware/def_usr.c)
target_include_directories(stmt_def_usr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_shell OBJECT src/statements/system/environment/shell.c)
target_include_directories(stmt_shell PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_pause OBJECT src/statements/system/environment/pause.c)
target_include_directories(stmt_pause PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_sys OBJECT src/statements/system/environment/sys.c)
target_include_directories(stmt_sys PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_bsave OBJECT src/statements/filesystem/binary_ops/bsave.c)
target_include_directories(stmt_bsave PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_brun OBJECT src/statements/filesystem/binary_ops/brun.c)
target_include_directories(stmt_brun PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_chdir OBJECT src/statements/filesystem/dir_ops/chdir.c)
target_include_directories(stmt_chdir PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_rmdir OBJECT src/statements/filesystem/dir_ops/rmdir.c)
target_include_directories(stmt_rmdir PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_lprint OBJECT src/statements/io/lprint.c)
target_include_directories(stmt_lprint PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_line_input OBJECT src/statements/core/io/line_input.c)
target_include_directories(stmt_line_input PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_tab OBJECT src/eval/functions/string/format/tab.c)
target_include_directories(func_tab PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_spc OBJECT src/eval/functions/string/format/spc.c)
target_include_directories(func_spc PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_pos OBJECT src/eval/functions/system/terminal/pos.c)
target_include_directories(func_pos PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_csrlin OBJECT src/eval/functions/system/terminal/csrlin.c)
target_include_directories(func_csrlin PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_lpos OBJECT src/eval/functions/system/terminal/lpos.c)
target_include_directories(func_lpos PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_erl OBJECT src/eval/functions/system/error/erl.c)
target_include_directories(func_erl PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_err_fn OBJECT src/eval/functions/system/error/err_fn.c)
target_include_directories(func_err_fn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_eof_fn OBJECT src/eval/functions/filesystem/status/eof_fn.c)
target_include_directories(func_eof_fn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_lof OBJECT src/eval/functions/filesystem/status/lof.c)
target_include_directories(func_lof PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_loc_fn OBJECT src/eval/functions/filesystem/status/loc_fn.c)
target_include_directories(func_loc_fn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_freefile OBJECT src/eval/functions/filesystem/descriptors/freefile.c)
target_include_directories(func_freefile PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_point_fn OBJECT src/eval/functions/ui/graphics/point_fn.c)
target_include_directories(func_point_fn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_lbound OBJECT src/eval/functions/math/linear_algebra/lbound.c)
target_include_directories(func_lbound PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(func_ubound OBJECT src/eval/functions/math/linear_algebra/ubound.c)
target_include_directories(func_ubound PUBLIC ${ENGINE_INCLUDE_DIR})

# Link Micro-Libraries into Core Libraries
# =====================================================================
set(ALL_PLAT_LIBS plat_console plat_fs plat_sys plat_time
    plat_thread plat_dl plat_net plat_regex plat_clipboard)

set(ALL_VM_LIBS vm_context vm_exec vm_data vm_error vm_control vm_events vm_math)

set(ALL_EDITOR_LIBS editor_buffer editor_render editor_selection editor_term)

target_link_libraries(libcore PUBLIC
    ${ALL_PLAT_LIBS} ${ALL_VM_LIBS})
target_link_libraries(libstandard PUBLIC
    ${ALL_EDITOR_LIBS})

if(UNIX)
    target_link_libraries(libcore PUBLIC m)
    target_link_libraries(libstandard PUBLIC ncurses)
endif()

if(WIN32)
    target_link_libraries(libcore PUBLIC ws2_32)
endif()

# NOTE: Executable targets (baspp, blite, bscript, bppc, detok) are
# defined in the root CMakeLists.txt where SDL2 and platform-specific
# configurations are available.







# Category Library Variable Definitions
set(SET_HEADLESS_LIBS
    abs
    acos
    asin
    atan2
    atn
    bin
    bitcount
    ceil
    chr
    clamp
    clock_num
    clock_str
    cos
    date
    degrees
    eps
    inf
    maxnum
    radians
    remainder
    angle
    truncate
    environ
    exp
    fix
    floor
    fre
    hex
    inkey
    inp
    instr
    int
    lcase
    left
    len
    lerp
    log
    ltrim
    mid
    oct
    pack
    peek
    pi
    readbit
    resetbit
    right
    rnd
    round
    rtrim
    setbit
    sgn
    shl
    shr
    sin
    space
    sqr
    str
    string
    tan
    ticks
    time
    timer
    togglebit
    func_and
    func_or
    func_xor
    func_imp
    func_eqv
    func_not
    func_min
    func_max
    func_hypot
    func_mod
    func_sinh
    func_cosh
    func_tanh
    func_log10
    func_log2
    func_cot
    func_sec
    func_csc
    func_comp
    func_pdif
    trim
    ucase
    shuffle
    unpack
    val
    func_tab
    func_spc
    func_pos
    func_csrlin
    func_lpos
    func_erl
    func_err_fn
    func_eof_fn
    func_lof
    func_loc_fn
    func_freefile
    func_point_fn
    func_lbound
    func_ubound

    stmt_array_ext
    stmt_auto
    stmt_delete
    stmt_merge
    stmt_renum
    stmt_read
    stmt_restore
    stmt_defint
    stmt_defsng
    stmt_defdbl
    stmt_defstr
    stmt_deflng
    stmt_common
    stmt_redim
    stmt_erase
    stmt_swap
    stmt_rset
    stmt_def_seg
    stmt_def_usr
    stmt_shell
    stmt_pause
    stmt_sys
    stmt_bsave
    stmt_brun
    stmt_chdir
    stmt_rmdir
    stmt_lprint
    stmt_line_input
    stmt_beep
    stmt_bgi
    stmt_bios
    stmt_bload
    stmt_call
    stmt_chain
    stmt_circle
    stmt_class
    stmt_clear
    stmt_close
    stmt_cls
    stmt_home
    stmt_color
    stmt_compat
    stmt_const
    stmt_cont
    stmt_data
    stmt_debug
    stmt_def
    stmt_defseg
    stmt_dim
    stmt_do
    stmt_draw
    stmt_end
    stmt_enum
    stmt_exit_loop
    stmt_field
    stmt_files
    stmt_for
    stmt_function
    stmt_get
    stmt_gosub
    stmt_goto
    stmt_help
    stmt_category
    func_category
    stmt_if

    stmt_input
    stmt_input_file
    stmt_introspection
    stmt_joystick
    stmt_kill
    stmt_let
    stmt_line
    stmt_list
    stmt_llist
    stmt_reformat
    debug_analyzer
    stmt_check
    stmt_verify
    stmt_test
    stmt_version
    func_varptr
    stmt_load
    stmt_lock
    stmt_loop
    stmt_lset
    stmt_mat_input
    stmt_mat_ops
    stmt_mat_print
    stmt_mat_read
    stmt_mat_write
    stmt_mid_stmt
    stmt_mkdir
    stmt_module
    stmt_mouse
    stmt_mux
    stmt_name
    stmt_new
    stmt_next
    stmt_on_com
    stmt_on_error
    stmt_on_key
    stmt_on_timer
    stmt_open
    stmt_option
    stmt_paint
    stmt_palette
    stmt_pen
    stmt_play
    stmt_plot
    stmt_poke
    stmt_preset
    stmt_print
    stmt_print_file
    stmt_pset
    stmt_put
    stmt_out
    stmt_randomize
    stmt_alias
    stmt_scope
    stmt_keyword
    stmt_override
    stmt_rem
    stmt_remove
    stmt_resume
    stmt_return
    stmt_run
    stmt_save
    stmt_screen
    stmt_security
    stmt_seek
    stmt_select
    stmt_selftest
    stmt_shared
    stmt_share
    stmt_public
    stmt_global
    stmt_declare
    stmt_external
    stmt_when
    stmt_handler
    stmt_cause
    stmt_retry
    stmt_continue
    stmt_ask
    stmt_viewport
    stmt_picture
    func_det
    func_dot
    func_cross
    stmt_sound
    stmt_stop
    stmt_sub
    stmt_system
    stmt_goodbye
    stmt_task
    stmt_try
    stmt_txn
    stmt_type
    stmt_view
    stmt_voice
    stmt_wend
    stmt_while
    stmt_window
    stmt_with
    stmt_write_file
    stmt_vdim
    stmt_whenever
    stmt_change
    stmt_margin
    stmt_zone
    stmt_append
    stmt_modify
    stmt_text
    stmt_linput
    stmt_image
    stmt_form
    stmt_assign
    stmt_record
    stmt_find
    stmt_map
    stmt_incr
    stmt_clr
    stmt_void
    stmt_arrayfill
    stmt_sleep
    stmt_suspend
    stmt_key
    stmt_wait
    stmt_echo
    stmt_extend
    stmt_scale
    stmt_enter
    func_cvt
    func_rad
    func_num
    func_xlate
    func_seg
    func_ert
    func_ascii_fn
    func_ups
    func_index_fn
    func_str_math
    func_spec_fn
    func_magtape
    func_mag
    func_sys
    func_time_fn
    mod_module
    rt_task
    rt_gemini
    microplex
    core_complex_num
    func_complex
    stmt_complex
    func_verify
    rt_session
    stmt_session
    stmt_page
    stmt_rewind
    stmt_backspace
    stmt_unless
    func_pick
    func_fid
    func_ath
    func_tek
    func_fpt
    stmt_doevents
    stmt_msgbox
    func_command
    func_vbdos
    func_mbf
    func_financial
    func_vbdos_filebox
    stmt_isam
    stmt_vbdos_widgets
    func_pds_datetime
    stmt_interrupt
    stmt_vbdos_controls
    stmt_exchange
    stmt_perform
    stmt_invoke
    stmt_prefix
    stmt_repeat
    stmt_until
    stmt_endloop
    stmt_destroy
    stmt_create
    stmt_param
    func_typ
    func_moddir
    func_dread
    func_aread
    func_touch
    func_hall
    func_ticks_iot
    func_mem
    func_http
    func_gemini
    func_gopher
    func_python
    stmt_pinmode
    stmt_dwrite
    stmt_dac
    stmt_pwm
    stmt_servo
    stmt_i2c
    stmt_spi
    stmt_neopixel
    stmt_dht
    stmt_delay
    stmt_every
    stmt_deepsleep
    stmt_reboot
    stmt_freq
    stmt_wifi
    stmt_mqtt
    stmt_webrepl
    stmt_python
    stmt_espnow
    stmt_bluetooth
    stmt_nfc
    stmt_gemini
    stmt_gopher
    stmt_tnfs
    stmt_gemini_browse
    func_tnfs
    func_gemini_meta
    func_fujinet
    nil_compress
    nil_bead
    nil_transport
    vdev_remote
    stmt_remote
    stmt_nil_bead
    func_nil_compress
    func_nil_bead
    func_remote
    peer
    sock_engine
    packet_sniff
    crypto_engine
    events_net
    stmt_peer
    stmt_sock
    stmt_port_trigger
    stmt_sniff
    stmt_net_config
    func_sock
    func_packet
    func_crypto
    PARENT_SCOPE
)

set(SET_FULL_STANDARD_LIBS
    abs
    acos
    asin
    atan2
    atn
    bin
    bitcount
    ceil
    chr
    clamp
    clock_num
    clock_str
    cos
    date
    degrees
    eps
    inf
    maxnum
    radians
    remainder
    angle
    truncate
    environ
    exp
    fix
    floor
    fre
    hex
    inkey
    inp
    instr
    int
    lcase
    left
    len
    lerp
    log
    ltrim
    mid
    oct
    pack
    peek
    pi
    readbit
    resetbit
    right
    rnd
    round
    rtrim
    setbit
    sgn
    shl
    shr
    sin
    space
    sqr
    str
    string
    tan
    ticks
    time
    timer
    togglebit
    func_and
    func_or
    func_xor
    func_imp
    func_eqv
    func_not
    func_min
    func_max
    func_hypot
    func_mod
    func_sinh
    func_cosh
    func_tanh
    func_log10
    func_log2
    func_cot
    func_sec
    func_csc
    func_comp
    func_pdif
    trim
    ucase
    shuffle
    unpack
    val
    func_tab
    func_spc
    func_pos
    func_csrlin
    func_lpos
    func_erl
    func_err_fn
    func_eof_fn
    func_lof
    func_loc_fn
    func_freefile
    func_point_fn
    func_lbound
    func_ubound

    stmt_array_ext
    stmt_auto
    stmt_delete
    stmt_merge
    stmt_renum
    stmt_read
    stmt_restore
    stmt_defint
    stmt_defsng
    stmt_defdbl
    stmt_defstr
    stmt_deflng
    stmt_common
    stmt_redim
    stmt_erase
    stmt_swap
    stmt_rset
    stmt_def_seg
    stmt_def_usr
    stmt_shell
    stmt_pause
    stmt_sys
    stmt_bsave
    stmt_brun
    stmt_chdir
    stmt_rmdir
    stmt_lprint
    stmt_line_input
    stmt_beep
    stmt_bgi
    stmt_bios
    stmt_bload
    stmt_call
    stmt_chain
    stmt_circle
    stmt_class
    stmt_clear
    stmt_close
    stmt_cls
    stmt_home
    stmt_color
    stmt_compat
    stmt_const
    stmt_cont
    stmt_data
    stmt_debug
    stmt_def
    stmt_defseg
    stmt_dim
    stmt_do
    stmt_draw
    stmt_end
    stmt_enum
    stmt_exit_loop
    stmt_field
    stmt_files
    stmt_for
    stmt_function
    stmt_get
    stmt_gosub
    stmt_goto
    stmt_help
    stmt_category
    func_category
    stmt_if

    stmt_input
    stmt_input_file
    stmt_introspection
    stmt_joystick
    stmt_kill
    stmt_let
    stmt_line
    stmt_list
    stmt_llist
    stmt_reformat
    debug_analyzer
    stmt_check
    stmt_verify
    stmt_test
    stmt_version
    func_varptr
    stmt_load
    stmt_lock
    stmt_loop
    stmt_lset
    stmt_mat_input
    stmt_mat_ops
    stmt_mat_print
    stmt_mat_read
    stmt_mat_write
    stmt_mid_stmt
    stmt_mkdir
    stmt_module
    stmt_mouse
    stmt_mux
    stmt_name
    stmt_new
    stmt_next
    stmt_on_com
    stmt_on_error
    stmt_on_key
    stmt_on_timer
    stmt_open
    stmt_option
    stmt_paint
    stmt_palette
    stmt_pen
    stmt_play
    stmt_plot
    stmt_poke
    stmt_preset
    stmt_print
    stmt_print_file
    stmt_pset
    stmt_put
    stmt_out
    stmt_randomize
    stmt_alias
    stmt_scope
    stmt_keyword
    stmt_override
    stmt_rem
    stmt_remove
    stmt_resume
    stmt_return
    stmt_run
    stmt_save
    stmt_screen
    stmt_security
    stmt_seek
    stmt_select
    stmt_selftest
    stmt_shared
    stmt_share
    stmt_public
    stmt_global
    stmt_declare
    stmt_external
    stmt_when
    stmt_handler
    stmt_cause
    stmt_retry
    stmt_continue
    stmt_ask
    stmt_viewport
    stmt_picture
    func_det
    func_dot
    func_cross
    stmt_sound
    stmt_stop
    stmt_sub
    stmt_system
    stmt_goodbye
    stmt_task
    stmt_try
    stmt_txn
    stmt_type
    stmt_view
    stmt_voice
    stmt_exchange
    stmt_perform
    stmt_invoke
    stmt_prefix
    stmt_repeat
    stmt_until
    stmt_endloop
    stmt_destroy
    stmt_create
    stmt_param
    func_typ
    func_moddir
    stmt_wend
    stmt_while
    stmt_window
    stmt_with
    stmt_write_file
    stmt_vdim
    stmt_whenever
    stmt_change
    stmt_margin
    stmt_zone
    stmt_append
    stmt_modify
    stmt_text
    stmt_linput
    stmt_image
    stmt_form
    stmt_assign
    stmt_record
    stmt_find
    stmt_map
    stmt_incr
    stmt_clr
    stmt_void
    stmt_arrayfill
    stmt_sleep
    stmt_suspend
    stmt_key
    stmt_wait
    stmt_echo
    stmt_extend
    stmt_scale
    stmt_enter
    func_cvt
    func_rad
    func_num
    func_xlate
    func_seg
    func_ert
    func_ascii_fn
    func_ups
    func_index_fn
    func_str_math
    func_spec_fn
    func_magtape
    func_mag
    func_sys
    func_time_fn
    mod_module
    rt_task
    rt_gemini
    microplex
    core_complex_num
    func_complex
    stmt_complex
    func_verify
    rt_session
    stmt_session
    stmt_page
    stmt_rewind
    stmt_backspace
    stmt_unless
    func_pick
    func_fid
    func_ath
    func_tek
    func_fpt
    stmt_doevents
    stmt_msgbox
    func_command
    func_vbdos
    func_mbf
    func_financial
    func_vbdos_filebox
    stmt_isam
    stmt_vbdos_widgets
    func_pds_datetime
    stmt_interrupt
    stmt_vbdos_controls
    func_dread
    func_aread
    func_touch
    func_hall
    func_ticks_iot
    func_mem
    func_http
    func_gemini
    func_gopher
    func_python
    stmt_pinmode
    stmt_dwrite
    stmt_dac
    stmt_pwm
    stmt_servo
    stmt_i2c
    stmt_spi
    stmt_neopixel
    stmt_dht
    stmt_delay
    stmt_every
    stmt_deepsleep
    stmt_reboot
    stmt_freq
    stmt_wifi
    stmt_mqtt
    stmt_webrepl
    stmt_python
    stmt_espnow
    stmt_bluetooth
    stmt_nfc
    stmt_gemini
    stmt_gopher
    stmt_tnfs
    stmt_gemini_browse
    func_tnfs
    func_gemini_meta
    func_fujinet
    nil_compress
    nil_bead
    nil_transport
    vdev_remote
    stmt_remote
    stmt_nil_bead
    func_nil_compress
    func_nil_bead
    func_remote
    peer
    sock_engine
    packet_sniff
    crypto_engine
    events_net
    stmt_peer
    stmt_sock
    stmt_port_trigger
    stmt_sniff
    stmt_net_config
    func_sock
    func_packet
    func_crypto
    PARENT_SCOPE
)

# =====================================================================
# Cross-Language Interoperability Subsystem
# =====================================================================

# --- libinterop_core: Interop handle table, error bridge, marshaling ---
set(INTEROP_CORE_SOURCES
    src/interop/interop_core.c
    src/interop/interop_handle.c
    src/interop/interop_error.c
    src/interop/interop_marshal.c
    src/interop/interop_ipc.c
    src/interop/interop_jsonrpc.c
)
if(WIN32)
    list(APPEND INTEROP_CORE_SOURCES
        src/interop/interop_com.c
        src/interop/interop_com_register.c
    )
endif()
add_library(libinterop_core STATIC ${INTEROP_CORE_SOURCES})
target_include_directories(libinterop_core PUBLIC ${ENGINE_INCLUDE_DIR})
target_compile_definitions(libinterop_core PRIVATE $<$<BOOL:${UNIX}>: _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE>)

# --- libcompiler_ir: Universal IR, LLVM/QBE backends ---
set(COMPILER_IR_SOURCES
    src/compiler/compiler_ir.c
)
add_library(libcompiler_ir STATIC ${COMPILER_IR_SOURCES})
target_include_directories(libcompiler_ir PUBLIC ${ENGINE_INCLUDE_DIR})
target_compile_definitions(libcompiler_ir PRIVATE $<$<BOOL:${UNIX}>: _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE>)

# --- Interop test targets ---
if (EXISTS "${CMAKE_SOURCE_DIR}/tests/interop/test_handle.c")
    add_executable(test_handle ${CMAKE_SOURCE_DIR}/tests/interop/test_handle.c)
    target_include_directories(test_handle PRIVATE ${ENGINE_INCLUDE_DIR})
    target_link_libraries(test_handle PRIVATE libinterop_core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/tests/interop/test_marshal.c")
    add_executable(test_marshal ${CMAKE_SOURCE_DIR}/tests/interop/test_marshal.c)
    target_include_directories(test_marshal PRIVATE ${ENGINE_INCLUDE_DIR})
    target_link_libraries(test_marshal PRIVATE libinterop_core)
endif()


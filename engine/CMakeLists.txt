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
set(ENGINE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${ENGINE_INCLUDE_DIR})

# Also expose aalib private headers for BGI subsystem
set(AALIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/device/bgi/aalib/include)

# =====================================================================
# Modular Static Libraries Architecture System
# =====================================================================

# --- 1. Boot Subsystem Library: libboot ---
add_library(libboot STATIC
    src/bootstrap/common/common.c
)
target_include_directories(libboot PUBLIC ${ENGINE_INCLUDE_DIR})
target_compile_definitions(libboot PUBLIC BASIC_CORE_BUILD)

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
    src/lexer/lexer.c
    src/memory/mem_system.c
    src/runtime/arrays.c
    src/runtime/map.c
    src/runtime/metadata.c
    src/security/security.c
    src/vm/context.c
    src/vm/control.c
    src/vm/data.c
    src/vm/error.c
    src/vm/events.c
    src/vm/host.c
    src/vm/math.c
    src/vm/stack.c
)
target_include_directories(libkernel PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libkernel PUBLIC libplatform)

# --- 4. Language Runtime Engine Subsystem Library: libengine ---
add_library(libengine STATIC
    src/eval/ast.c
    src/eval/dispatch.c
    src/eval/eval.c
    src/eval/eval_builtins.c
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
    src/runtime/print_using.c
    src/runtime/spec.c
    src/runtime/state.c
    src/runtime/string_ext.c
    src/runtime/strings.c
    src/runtime/using_engine.c
    src/runtime/variables.c
    src/scope/scope.c
    src/vm/exec.c
)
target_include_directories(libengine PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libengine PUBLIC libkernel)

# --- 5. Hardware Subsystem Library: libhardware ---
add_library(libhardware STATIC
    src/device/bgi/bgi_stub.c
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
)
target_include_directories(libscript PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libscript PUBLIC libserver)

# --- 8. Core REPL & Introspection Subsystem Library: libcore ---
add_library(libcore STATIC
    src/docgen/docgen.c
    src/statements/dialect/help.c
    src/statements/dialect/introspection.c
    src/statements/dialect/selftest.c
)
target_include_directories(libcore PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libcore PUBLIC libscript)

# --- 9. Dynamic Metaprogramming & Flex Subsystem Library: libflex ---
add_library(libflex STATIC
    src/statements/dialect/alias.c
    src/statements/dialect/keyword.c
    src/statements/dialect/module_stmt.c
    src/statements/dialect/override.c
    src/statements/dialect/remove.c
    src/statements/dialect/scope.c
)
target_include_directories(libflex PUBLIC ${ENGINE_INCLUDE_DIR})
target_link_libraries(libflex PUBLIC libcore)

# --- 10. Standard TUI Workstation & Editor Subsystem Library: libstandard ---
add_library(libstandard STATIC
    src/debug/dap_server.c
    src/editor/edit.c
    src/editor/editor.c
    src/editor/editor_manager.c
    src/editor/edlin.c
    src/editor/tui_multiplexer.c
    src/editor/vi.c
    src/editor/ws.c
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

add_library(vm_exec STATIC src/vm/exec.c)
target_include_directories(vm_exec PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_data STATIC src/vm/data.c)
target_include_directories(vm_data PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_error STATIC src/vm/error.c)
target_include_directories(vm_error PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_control STATIC src/vm/control.c)
target_include_directories(vm_control PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(vm_events STATIC src/vm/events.c)
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

add_library(bitcount OBJECT src/eval/functions/bits/bitcount.c)
target_include_directories(bitcount PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(readbit OBJECT src/eval/functions/bits/readbit.c)
target_include_directories(readbit PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(resetbit OBJECT src/eval/functions/bits/resetbit.c)
target_include_directories(resetbit PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(setbit OBJECT src/eval/functions/bits/setbit.c)
target_include_directories(setbit PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(shl OBJECT src/eval/functions/bits/shl.c)
target_include_directories(shl PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(shr OBJECT src/eval/functions/bits/shr.c)
target_include_directories(shr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(togglebit OBJECT src/eval/functions/bits/togglebit.c)
target_include_directories(togglebit PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(abs OBJECT src/eval/functions/math/abs.c)
target_include_directories(abs PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(acos OBJECT src/eval/functions/math/acos.c)
target_include_directories(acos PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(asin OBJECT src/eval/functions/math/asin.c)
target_include_directories(asin PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(atan2 OBJECT src/eval/functions/math/atan2.c)
target_include_directories(atan2 PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(atn OBJECT src/eval/functions/math/atn.c)
target_include_directories(atn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(ceil OBJECT src/eval/functions/math/ceil.c)
target_include_directories(ceil PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(clamp OBJECT src/eval/functions/math/clamp.c)
target_include_directories(clamp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(cos OBJECT src/eval/functions/math/cos.c)
target_include_directories(cos PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(exp OBJECT src/eval/functions/math/exp.c)
target_include_directories(exp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(fix OBJECT src/eval/functions/math/fix.c)
target_include_directories(fix PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(floor OBJECT src/eval/functions/math/floor.c)
target_include_directories(floor PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(int OBJECT src/eval/functions/math/int.c)
target_include_directories(int PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(lerp OBJECT src/eval/functions/math/lerp.c)
target_include_directories(lerp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(log OBJECT src/eval/functions/math/log.c)
target_include_directories(log PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(pi OBJECT src/eval/functions/math/pi.c)
target_include_directories(pi PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(rnd OBJECT src/eval/functions/math/rnd.c)
target_include_directories(rnd PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(round OBJECT src/eval/functions/math/round.c)
target_include_directories(round PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(sgn OBJECT src/eval/functions/math/sgn.c)
target_include_directories(sgn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(sin OBJECT src/eval/functions/math/sin.c)
target_include_directories(sin PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(sqr OBJECT src/eval/functions/math/sqr.c)
target_include_directories(sqr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(tan OBJECT src/eval/functions/math/tan.c)
target_include_directories(tan PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(bin OBJECT src/eval/functions/string/bin.c)
target_include_directories(bin PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(chr OBJECT src/eval/functions/string/chr.c)
target_include_directories(chr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(hex OBJECT src/eval/functions/string/hex.c)
target_include_directories(hex PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(instr OBJECT src/eval/functions/string/instr.c)
target_include_directories(instr PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(lcase OBJECT src/eval/functions/string/lcase.c)
target_include_directories(lcase PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(left OBJECT src/eval/functions/string/left.c)
target_include_directories(left PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(len OBJECT src/eval/functions/string/len.c)
target_include_directories(len PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(ltrim OBJECT src/eval/functions/string/ltrim.c)
target_include_directories(ltrim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(mid OBJECT src/eval/functions/string/mid.c)
target_include_directories(mid PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(oct OBJECT src/eval/functions/string/oct.c)
target_include_directories(oct PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(pack OBJECT src/eval/functions/string/pack.c)
target_include_directories(pack PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(right OBJECT src/eval/functions/string/right.c)
target_include_directories(right PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(rtrim OBJECT src/eval/functions/string/rtrim.c)
target_include_directories(rtrim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(space OBJECT src/eval/functions/string/space.c)
target_include_directories(space PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(str OBJECT src/eval/functions/string/str.c)
target_include_directories(str PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(string OBJECT src/eval/functions/string/string.c)
target_include_directories(string PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(trim OBJECT src/eval/functions/string/trim.c)
target_include_directories(trim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(ucase OBJECT src/eval/functions/string/ucase.c)
target_include_directories(ucase PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(unpack OBJECT src/eval/functions/string/unpack.c)
target_include_directories(unpack PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(val OBJECT src/eval/functions/string/val.c)
target_include_directories(val PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(clock_num OBJECT src/eval/functions/system/clock_num.c)
target_include_directories(clock_num PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(clock_str OBJECT src/eval/functions/system/clock_str.c)
target_include_directories(clock_str PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(date OBJECT src/eval/functions/system/date.c)
target_include_directories(date PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(environ OBJECT src/eval/functions/system/environ.c)
target_include_directories(environ PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(fre OBJECT src/eval/functions/system/fre.c)
target_include_directories(fre PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(inkey OBJECT src/eval/functions/system/inkey.c)
target_include_directories(inkey PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(inp OBJECT src/eval/functions/system/inp.c)
target_include_directories(inp PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(peek OBJECT src/eval/functions/system/peek.c)
target_include_directories(peek PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(ticks OBJECT src/eval/functions/system/ticks.c)
target_include_directories(ticks PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(time OBJECT src/eval/functions/system/time.c)
target_include_directories(time PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(timer OBJECT src/eval/functions/system/timer.c)
target_include_directories(timer PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_end OBJECT src/statements/core/end.c)
target_include_directories(stmt_end PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_gosub OBJECT src/statements/core/gosub.c)
target_include_directories(stmt_gosub PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_goto OBJECT src/statements/core/goto.c)
target_include_directories(stmt_goto PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_if OBJECT src/statements/core/if.c)
target_include_directories(stmt_if PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_input OBJECT src/statements/core/input.c)
target_include_directories(stmt_input PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_print OBJECT src/statements/core/print.c)
target_include_directories(stmt_print PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_randomize OBJECT src/statements/core/randomize.c)
target_include_directories(stmt_randomize PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_rem OBJECT src/statements/core/rem.c)
target_include_directories(stmt_rem PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_return OBJECT src/statements/core/return.c)
target_include_directories(stmt_return PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_select OBJECT src/statements/core/select.c)
target_include_directories(stmt_select PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_stop OBJECT src/statements/core/stop.c)
target_include_directories(stmt_stop PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_do OBJECT src/statements/loops/do.c)
target_include_directories(stmt_do PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_exit_loop OBJECT src/statements/loops/exit_loop.c)
target_include_directories(stmt_exit_loop PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_for OBJECT src/statements/loops/for.c)
target_include_directories(stmt_for PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_loop OBJECT src/statements/loops/loop.c)
target_include_directories(stmt_loop PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_next OBJECT src/statements/loops/next.c)
target_include_directories(stmt_next PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_wend OBJECT src/statements/loops/wend.c)
target_include_directories(stmt_wend PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_while OBJECT src/statements/loops/while.c)
target_include_directories(stmt_while PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_chain OBJECT src/statements/program/chain.c)
target_include_directories(stmt_chain PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_clear OBJECT src/statements/program/clear.c)
target_include_directories(stmt_clear PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_cont OBJECT src/statements/program/cont.c)
target_include_directories(stmt_cont PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_list OBJECT src/statements/program/list.c)
target_include_directories(stmt_list PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_reformat OBJECT src/statements/program/reformat.c src/statements/program/reformat_engine.c)
target_include_directories(stmt_reformat PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(debug_analyzer OBJECT src/debug/analyzer.c)
target_include_directories(debug_analyzer PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_check OBJECT src/statements/debug/check.c)
target_include_directories(stmt_check PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_verify OBJECT src/statements/debug/verify.c)
target_include_directories(stmt_verify PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_test OBJECT src/statements/debug/test.c)
target_include_directories(stmt_test PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_version OBJECT src/statements/system/version.c)
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

add_library(stmt_array_ext OBJECT src/statements/variables/array_ext.c)
target_include_directories(stmt_array_ext PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_data OBJECT src/statements/variables/data.c)
target_include_directories(stmt_data PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_def OBJECT src/statements/variables/def.c)
target_include_directories(stmt_def PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_dim OBJECT src/statements/variables/dim.c)
target_include_directories(stmt_dim PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_let OBJECT src/statements/variables/let.c)
target_include_directories(stmt_let PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_lset OBJECT src/statements/variables/lset.c)
target_include_directories(stmt_lset PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mid_stmt OBJECT src/statements/variables/mid_stmt.c)
target_include_directories(stmt_mid_stmt PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mux OBJECT src/statements/variables/mux.c)
target_include_directories(stmt_mux PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_option OBJECT src/statements/variables/option.c)
target_include_directories(stmt_option PUBLIC ${ENGINE_INCLUDE_DIR})


add_library(stmt_bload OBJECT src/statements/filesystem/bload.c)
target_include_directories(stmt_bload PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_close OBJECT src/statements/filesystem/close.c)
target_include_directories(stmt_close PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_field OBJECT src/statements/filesystem/field.c)
target_include_directories(stmt_field PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_files OBJECT src/statements/filesystem/files.c)
target_include_directories(stmt_files PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_get OBJECT src/statements/filesystem/get.c)
target_include_directories(stmt_get PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_input_file OBJECT src/statements/filesystem/input_file.c)
target_include_directories(stmt_input_file PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_kill OBJECT src/statements/filesystem/kill.c)
target_include_directories(stmt_kill PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_lock OBJECT src/statements/filesystem/lock.c)
target_include_directories(stmt_lock PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mkdir OBJECT src/statements/filesystem/mkdir.c)
target_include_directories(stmt_mkdir PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_name OBJECT src/statements/filesystem/name.c)
target_include_directories(stmt_name PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_open OBJECT src/statements/filesystem/open.c)
target_include_directories(stmt_open PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_print_file OBJECT src/statements/filesystem/print_file.c)
target_include_directories(stmt_print_file PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_put OBJECT src/statements/filesystem/put.c)
target_include_directories(stmt_put PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_seek OBJECT src/statements/filesystem/seek.c)
target_include_directories(stmt_seek PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_write_file OBJECT src/statements/filesystem/write_file.c)
target_include_directories(stmt_write_file PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_beep OBJECT src/statements/sound/beep.c)
target_include_directories(stmt_beep PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_play OBJECT src/statements/sound/play.c)
target_include_directories(stmt_play PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_sound OBJECT src/statements/sound/sound.c)
target_include_directories(stmt_sound PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_voice OBJECT src/statements/sound/voice.c)
target_include_directories(stmt_voice PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_bgi OBJECT src/statements/graphics/bgi.c)
target_include_directories(stmt_bgi PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_circle OBJECT src/statements/graphics/circle.c)
target_include_directories(stmt_circle PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_cls OBJECT src/statements/graphics/cls.c)
target_include_directories(stmt_cls PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_home OBJECT src/statements/graphics/stmt_home.c)
target_include_directories(stmt_home PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_color OBJECT src/statements/graphics/color.c)
target_include_directories(stmt_color PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_compat OBJECT src/statements/graphics/compat.c)
target_include_directories(stmt_compat PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_draw OBJECT src/statements/graphics/draw.c)
target_include_directories(stmt_draw PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_line OBJECT src/statements/graphics/line.c)
target_include_directories(stmt_line PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_paint OBJECT src/statements/graphics/paint.c)
target_include_directories(stmt_paint PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_palette OBJECT src/statements/graphics/palette.c)
target_include_directories(stmt_palette PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_preset OBJECT src/statements/graphics/preset.c)
target_include_directories(stmt_preset PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_pset OBJECT src/statements/graphics/pset.c)
target_include_directories(stmt_pset PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_screen OBJECT src/statements/graphics/screen.c)
target_include_directories(stmt_screen PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_view OBJECT src/statements/graphics/view.c)
target_include_directories(stmt_view PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_window OBJECT src/statements/graphics/window.c)
target_include_directories(stmt_window PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_call OBJECT src/statements/oop/call.c)
target_include_directories(stmt_call PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_class OBJECT src/statements/oop/class.c)
target_include_directories(stmt_class PUBLIC ${ENGINE_INCLUDE_DIR})


add_library(stmt_enum OBJECT src/statements/oop/enum.c)
target_include_directories(stmt_enum PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_function OBJECT src/statements/oop/function.c)
target_include_directories(stmt_function PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_module OBJECT src/statements/oop/module.c)
target_include_directories(stmt_module PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_shared OBJECT src/statements/oop/shared.c)
target_include_directories(stmt_shared PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_sub OBJECT src/statements/oop/sub.c)
target_include_directories(stmt_sub PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_type OBJECT src/statements/oop/type.c)
target_include_directories(stmt_type PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_with OBJECT src/statements/oop/with.c)
target_include_directories(stmt_with PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat_input OBJECT src/statements/matrices/mat_input.c)
target_include_directories(stmt_mat_input PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat_ops OBJECT src/statements/matrices/mat_ops.c)
target_include_directories(stmt_mat_ops PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat_print OBJECT src/statements/matrices/mat_print.c)
target_include_directories(stmt_mat_print PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mat_read OBJECT src/statements/matrices/mat_read.c)
target_include_directories(stmt_mat_read PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_on_com OBJECT src/statements/event/on_com.c)
target_include_directories(stmt_on_com PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_on_error OBJECT src/statements/event/on_error.c)
target_include_directories(stmt_on_error PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_on_key OBJECT src/statements/event/on_key.c)
target_include_directories(stmt_on_key PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_on_timer OBJECT src/statements/event/on_timer.c)
target_include_directories(stmt_on_timer PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_resume OBJECT src/statements/event/resume.c)
target_include_directories(stmt_resume PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_try OBJECT src/statements/event/try.c)
target_include_directories(stmt_try PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_bios OBJECT src/statements/system/bios.c)
target_include_directories(stmt_bios PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_debug OBJECT src/statements/system/debug.c)
target_include_directories(stmt_debug PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_defseg OBJECT src/statements/system/defseg.c)
target_include_directories(stmt_defseg PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_joystick OBJECT src/statements/system/joystick.c)
target_include_directories(stmt_joystick PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_mouse OBJECT src/statements/system/mouse.c)
target_include_directories(stmt_mouse PUBLIC ${ENGINE_INCLUDE_DIR})


add_library(stmt_pen OBJECT src/statements/system/pen.c)
target_include_directories(stmt_pen PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_poke OBJECT src/statements/system/poke.c)
target_include_directories(stmt_poke PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_security OBJECT src/statements/system/security.c)
target_include_directories(stmt_security PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_system OBJECT src/statements/system/system.c)
target_include_directories(stmt_system PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_task OBJECT src/statements/system/task.c)
target_include_directories(stmt_task PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_txn OBJECT src/statements/system/txn.c)
target_include_directories(stmt_txn PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_help OBJECT src/statements/dialect/help.c)
target_include_directories(stmt_help PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_introspection OBJECT src/statements/dialect/introspection.c)
target_include_directories(stmt_introspection PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_alias OBJECT src/statements/dialect/alias.c)
target_include_directories(stmt_alias PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_scope OBJECT src/statements/dialect/scope.c)
target_include_directories(stmt_scope PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_keyword OBJECT src/statements/dialect/keyword.c)
target_include_directories(stmt_keyword PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_override OBJECT src/statements/dialect/override.c)
target_include_directories(stmt_override PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_remove OBJECT src/statements/dialect/remove.c)
target_include_directories(stmt_remove PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_selftest OBJECT src/statements/dialect/selftest.c)
target_include_directories(stmt_selftest PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(mod_module OBJECT src/module/module.c)
target_include_directories(mod_module PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(rt_task OBJECT src/runtime/task.c)
target_include_directories(rt_task PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(rt_gemini OBJECT src/runtime/gemini.c)
target_include_directories(rt_gemini PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(stmt_dialect_stubs OBJECT src/statements/dialect/dialect_stubs.c)
target_include_directories(stmt_dialect_stubs PUBLIC ${ENGINE_INCLUDE_DIR})

add_library(microplex OBJECT src/eval/microplex.c)
target_include_directories(microplex PUBLIC ${ENGINE_INCLUDE_DIR})

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
    trim
    ucase
    unpack
    val
    stmt_array_ext
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
    stmt_cont
    stmt_data
    stmt_debug
    stmt_def
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
    stmt_if
    stmt_input
    stmt_input_file
    stmt_introspection
    stmt_joystick
    stmt_kill
    stmt_let
    stmt_line
    stmt_list
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
    stmt_mid_stmt
    stmt_mkdir
    stmt_module
    stmt_mouse
    stmt_mux
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
    stmt_poke
    stmt_preset
    stmt_print
    stmt_print_file
    stmt_pset
    stmt_put
    stmt_randomize
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
    stmt_sound
    stmt_stop
    stmt_sub
    stmt_system
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
    mod_module
    rt_task
    rt_gemini
    microplex
    stmt_dialect_stubs
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
    trim
    ucase
    unpack
    val
    stmt_array_ext
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
    stmt_cont
    stmt_data
    stmt_debug
    stmt_def
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
    stmt_if
    stmt_input
    stmt_input_file
    stmt_introspection
    stmt_joystick
    stmt_kill
    stmt_let
    stmt_line
    stmt_list
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
    stmt_mid_stmt
    stmt_mkdir
    stmt_module
    stmt_mouse
    stmt_mux
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
    stmt_poke
    stmt_preset
    stmt_print
    stmt_print_file
    stmt_pset
    stmt_put
    stmt_randomize
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
    stmt_sound
    stmt_stop
    stmt_sub
    stmt_system
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
    mod_module
    rt_task
    rt_gemini
    microplex
    PARENT_SCOPE
)

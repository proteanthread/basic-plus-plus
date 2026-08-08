/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file exec.c
 * @brief VM master fetch-decode-execute loop, line step controller, and statement handler dispatcher for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `vm_exec()`, `vm_run()`, `vm_step()`, `vm_exec_line()`, statement handler registration, and master bytecode execution loop.
 *
 * 2. WHY IT EXISTS:
 * Serves as the central execution engine of BASIC++, processing source lines sequentially or jumping dynamically across label targets.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Ephemerally tokenizes current line via `lexer_tokenize()`, identifies statement keyword, dispatches to dedicated keyword statement handler function, polls async events, and checks for break/stop signals.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "vm/vm.h", "vm_internal.h",
 * "lexer/lexer.h", "stmt/stmt.h", "eval/eval.h", "device/vdev.h", "debug/logger.h", "platform/platform.h", <stdio.h>, <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Register new statement handlers in `vm_register_statement_handlers()` or adjust execution cycle polling hooks.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Strictly non-recursive VM execution loop invariant (host stack recursion strictly forbidden).
 *
 * 8. WHAT TO EXPECT:
 * `vm_exec()` executes until program completion (`END`/`STOP`), unhandled error, or interrupt, returning execution status code.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify statement handler dispatch tables, trace `vm->pc` line pointer movement, and check `vm->last_error`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid `VMContext` created with initialized statement handler registry.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Host stack non-recursive design ensures safety on constrained targets (MCUs, embedded).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/context.c
 * - engine/src/vm/control.c
 * - engine/src/lexer/lexer.c
 * - engine/src/eval/eval.c
 * Prerequisite Header Files:
 * - engine/include/vm/vm.h
 * - engine/src/vm/vm_internal.h
 * - engine/include/lexer/lexer.h
 * - engine/include/stmt/stmt.h
 * - engine/include/eval/eval.h
 */
#include "vm/vm.h"
#include "vm_internal.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "types/config.h"
#include "runtime/metadata.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "device/usb.h"
#include "runtime/file.h"
#include "device/vcon.h"
#include "device/bus.h"
#include "runtime/spec.h"
#include "security/security.h"
#include "eval/eval.h"
#include "debug/logger.h"
#include "runtime/collections.h"
#include "runtime/variables.h"
#include "platform/platform.h"

#include "core/struct.h"
#include "statements/dialect/alias.h"
#include "statements/dialect/scope.h"
#include "statements/dialect/keyword.h"
#include "statements/dialect/override.h"
#include "statements/dialect/module_stmt.h"
#include "scope/scope.h"
#include "module/module.h"
#include "runtime/keyword_props.h"
#include "runtime/override.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


extern void console_hide_mouse_cursor(void);
extern void console_draw_mouse_cursor(void);
extern void platform_mouse_get_position(int *col, int *row);
extern int platform_mouse_get_button(int btn_idx);

#define MAX_PARAMS 8

void        vm_register_alias(VMContext *vm, const char *name, const char *expansion);
const char *vm_lookup_alias(VMContext *vm, const char *name);
void        vm_trigger_try_catch_handler(VMContext *vm, int code, const char *msg);
BValue      eval_expression(VMContext *vm, LexerContext *lex, BppError *out_err);
BValue      invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err);

BppError stmt_print_handler(VMContext *vm, LexerContext *lex);
BppError stmt_display_handler(VMContext *vm, LexerContext *lex);
BppError stmt_lprint_handler(VMContext *vm, LexerContext *lex);
BppError stmt_let_handler(VMContext *vm, LexerContext *lex);
BppError stmt_lset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_input_handler(VMContext *vm, LexerContext *lex);
BppError stmt_randomize_handler(VMContext *vm, LexerContext *lex);
BppError stmt_goto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_gosub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_return_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_handler(VMContext *vm, LexerContext *lex);
BppError stmt_system_handler(VMContext *vm, LexerContext *lex);
BppError stmt_stop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rem_handler(VMContext *vm, LexerContext *lex);
BppError stmt_if_handler(VMContext *vm, LexerContext *lex);
BppError stmt_list_handler(VMContext *vm, LexerContext *lex);
BppError stmt_reformat_handler(VMContext *vm, LexerContext *lex);
BppError stmt_run_handler(VMContext *vm, LexerContext *lex);
BppError stmt_new_handler(VMContext *vm, LexerContext *lex);
BppError stmt_for_handler(VMContext *vm, LexerContext *lex);
BppError stmt_next_handler(VMContext *vm, LexerContext *lex);
BppError stmt_while_handler(VMContext *vm, LexerContext *lex);
BppError stmt_wend_handler(VMContext *vm, LexerContext *lex);
BppError stmt_do_handler(VMContext *vm, LexerContext *lex);
BppError stmt_loop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dim_handler(VMContext *vm, LexerContext *lex);
BppError stmt_erase_handler(VMContext *vm, LexerContext *lex);
BppError stmt_option_handler(VMContext *vm, LexerContext *lex);
BppError stmt_data_handler(VMContext *vm, LexerContext *lex);
BppError stmt_read_handler(VMContext *vm, LexerContext *lex);
BppError stmt_restore_handler(VMContext *vm, LexerContext *lex);
BppError stmt_on_handler(VMContext *vm, LexerContext *lex);
BppError stmt_timer_handler(VMContext *vm, LexerContext *lex);
BppError stmt_alarm_handler(VMContext *vm, LexerContext *lex);
BppError stmt_alarm_str_handler(VMContext *vm, LexerContext *lex);
BppError stmt_set_handler(VMContext *vm, LexerContext *lex);
BppError stmt_key_handler(VMContext *vm, LexerContext *lex);
BppError stmt_com_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pen_handler(VMContext *vm, LexerContext *lex);
BppError stmt_strig_handler(VMContext *vm, LexerContext *lex);
BppError stmt_try_handler(VMContext *vm, LexerContext *lex);
BppError stmt_with_handler(VMContext *vm, LexerContext *lex);
BppError stmt_catch_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_try_handler(VMContext *vm, LexerContext *lex);
BppError stmt_throw_handler(VMContext *vm, LexerContext *lex);
BppError stmt_alias_handler(VMContext *vm, LexerContext *lex);

BppError stmt_metadata_handler(VMContext *vm, LexerContext *lex);
BppError stmt_resume_handler(VMContext *vm, LexerContext *lex);
BppError stmt_load_handler(VMContext *vm, LexerContext *lex);
BppError stmt_save_handler(VMContext *vm, LexerContext *lex);
BppError stmt_merge_handler(VMContext *vm, LexerContext *lex);
BppError stmt_common_handler(VMContext *vm, LexerContext *lex);
BppError stmt_chain_handler(VMContext *vm, LexerContext *lex);
BppError stmt_selftest_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defint_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defsng_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bye_handler(VMContext *vm, LexerContext *lex);
BppError stmt_exit_handler(VMContext *vm, LexerContext *lex);
BppError stmt_shell_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defdbl_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defstr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defusr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unsave_handler(VMContext *vm, LexerContext *lex);
#if SUPPORT_OOP
BppError stmt_type_handler(VMContext *vm, LexerContext *lex);
BppError stmt_class_handler(VMContext *vm, LexerContext *lex);
BppError stmt_enum_handler(VMContext *vm, LexerContext *lex);
#endif

/* Phase 3 & 4 statement handlers */
BppError stmt_open_handler(VMContext *vm, LexerContext *lex);
BppError stmt_field_handler(VMContext *vm, LexerContext *lex);
BppError stmt_close_handler(VMContext *vm, LexerContext *lex);
BppError stmt_reset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_get_handler(VMContext *vm, LexerContext *lex);
BppError stmt_put_handler(VMContext *vm, LexerContext *lex);
BppError stmt_seek_handler(VMContext *vm, LexerContext *lex);
BppError stmt_files_handler(VMContext *vm, LexerContext *lex);
BppError stmt_kill_handler(VMContext *vm, LexerContext *lex);
BppError stmt_scratch_handler(VMContext *vm, LexerContext *lex);
BppError stmt_chdir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pwd_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hostname_handler(VMContext *vm, LexerContext *lex);
BppError stmt_username_handler(VMContext *vm, LexerContext *lex);
BppError stmt_path_handler(VMContext *vm, LexerContext *lex);
BppError stmt_error_statement_handler(VMContext *vm, LexerContext *lex);

/* Phase 11b and 11c statement handlers */
BppError stmt_noise_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndplay_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndloop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndstop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndpause_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndvol_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mouseinput_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mousehide_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mouseshow_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mouse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hmouse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vmouse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_trig_handler(VMContext *vm, LexerContext *lex);
BppError stmt_title_handler(VMContext *vm, LexerContext *lex);
BppError stmt_screenmove_handler(VMContext *vm, LexerContext *lex);
BppError stmt_fullscreen_handler(VMContext *vm, LexerContext *lex);
BppError stmt_resize_handler(VMContext *vm, LexerContext *lex);
BppError stmt_icon_handler(VMContext *vm, LexerContext *lex);
BppError stmt_nwrite_handler(VMContext *vm, LexerContext *lex);
BppError stmt_freeimage_handler(VMContext *vm, LexerContext *lex);
BppError stmt_putimage_handler(VMContext *vm, LexerContext *lex);
BppError stmt_statesave_handler(VMContext *vm, LexerContext *lex);
BppError stmt_stateload_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mkdir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rmdir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_name_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_setattr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_environ_handler(VMContext *vm, LexerContext *lex);
BppError stmt_lock_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unlock_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ioctl_handler(VMContext *vm, LexerContext *lex);
BppError stmt_devices_handler(VMContext *vm, LexerContext *lex);
BppError stmt_select_handler(VMContext *vm, LexerContext *lex);
BppError stmt_case_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_procedure_handler(VMContext *vm, LexerContext *lex);
BppError stmt_function_handler(VMContext *vm, LexerContext *lex);
BppError stmt_call_handler(VMContext *vm, LexerContext *lex);
BppError stmt_declare_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_sub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_function_handler(VMContext *vm, LexerContext *lex);
BppError stmt_screen_handler(VMContext *vm, LexerContext *lex);
BppError stmt_color_handler(VMContext *vm, LexerContext *lex);
BppError stmt_cls_handler(VMContext *vm, LexerContext *lex);
BppError stmt_home_handler(VMContext *vm, LexerContext *lex);
BppError stmt_auto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_fcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_clear_handler(VMContext *vm, LexerContext *lex);
BppError stmt_cursor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_locate_handler(VMContext *vm, LexerContext *lex);
BppError stmt_shared_handler(VMContext *vm, LexerContext *lex);
BppError stmt_beep_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bload_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bsave_handler(VMContext *vm, LexerContext *lex);
BppError stmt_brun_handler(VMContext *vm, LexerContext *lex);
BppError stmt_line_handler(VMContext *vm, LexerContext *lex);
#ifndef BASIC_LITE_BUILD
BppError stmt_circle_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_preset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_paint_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sound_handler(VMContext *vm, LexerContext *lex);
BppError stmt_play_handler(VMContext *vm, LexerContext *lex);
#endif
#if SUPPORT_EDITOR && !defined(BASIC_LITE_BUILD)
BppError stmt_edit_handler(VMContext *vm, LexerContext *lex);
#endif
#ifndef BASIC_LITE_BUILD
BppError stmt_security_handler(VMContext *vm, LexerContext *lex);
#endif
BppError stmt_module_handler(VMContext *vm, LexerContext *lex);
BppError stmt_task_handler(VMContext *vm, LexerContext *lex);
BppError stmt_help_handler(VMContext *vm, LexerContext *lex);
BppError stmt_remove_handler(VMContext *vm, LexerContext *lex);
BppError stmt_remove_str_handler(VMContext *vm, LexerContext *lex);
#if SUPPORT_MAT
BppError stmt_mat_handler(VMContext *vm, LexerContext *lex);
#ifndef BASIC_LITE_BUILD
BppError stmt_arrayext_handler(VMContext *vm, LexerContext *lex);
#endif
#endif
BppError stmt_mux_handler(VMContext *vm, LexerContext *lex);
BppError stmt_demux_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unpack_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bitmux_handler(VMContext *vm, LexerContext *lex);
#ifndef BASIC_LITE_BUILD
BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex);
#endif
#if SUPPORT_EDITOR
BppError stmt_renum_handler(VMContext *vm, LexerContext *lex);
BppError stmt_delete_handler(VMContext *vm, LexerContext *lex);
#endif
BppError stmt_check_handler(VMContext *vm, LexerContext *lex);
BppError stmt_verify_handler(VMContext *vm, LexerContext *lex);
BppError stmt_test_handler(VMContext *vm, LexerContext *lex);
BppError stmt_endtest_handler(VMContext *vm, LexerContext *lex);
BppError stmt_trace_handler(VMContext *vm, LexerContext *lex);
BppError stmt_debug_handler(VMContext *vm, LexerContext *lex);
BppError stmt_cont_handler(VMContext *vm, LexerContext *lex);
BppError stmt_backtrace_handler(VMContext *vm, LexerContext *lex);
BppError stmt_info_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dump_handler(VMContext *vm, LexerContext *lex);
BppError stmt_version_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ver_handler(VMContext *vm, LexerContext *lex);
BppError stmt_catalog_handler(VMContext *vm, LexerContext *lex);
#if SUPPORT_NET
BppError stmt_mount_handler(VMContext *vm, LexerContext *lex);
BppError stmt_umount_handler(VMContext *vm, LexerContext *lex);
#endif
BppError stmt_chvt_handler(VMContext *vm, LexerContext *lex);
#if SUPPORT_NET
BppError stmt_net_handler(VMContext *vm, LexerContext *lex);
#endif
BppError stmt_out_handler(VMContext *vm, LexerContext *lex);
BppError stmt_poke_handler(VMContext *vm, LexerContext *lex);
#if SUPPORT_BIOS
BppError stmt_bios_handler(VMContext *vm, LexerContext *lex);
#endif
#if SUPPORT_GEMINI
BppError stmt_gemini_handler(VMContext *vm, LexerContext *lex);
#endif

#ifndef BASIC_LITE_BUILD
BppError stmt_gr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hgr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hgr2_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_plot_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hlin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vlin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hplot_handler(VMContext *vm, LexerContext *lex);

/* Transaction statement handlers */
BppError stmt_txn_handler(VMContext *vm, LexerContext *lex);
BppError stmt_atomic_handler(VMContext *vm, LexerContext *lex);
BppError stmt_commit_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rollback_handler(VMContext *vm, LexerContext *lex);
BppError stmt_graphics_handler(VMContext *vm, LexerContext *lex);
BppError stmt_drawto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_border_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ink_handler(VMContext *vm, LexerContext *lex);
BppError stmt_paper_handler(VMContext *vm, LexerContext *lex);

/* BGI (BASIC++ Graphics Interface) statement handlers */
BppError stmt_initgraph_handler(VMContext *vm, LexerContext *lex);
BppError stmt_closegraph_handler(VMContext *vm, LexerContext *lex);
BppError stmt_putpixel_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bar_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ellipse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rectangle_handler(VMContext *vm, LexerContext *lex);
BppError stmt_outtextxy_handler(VMContext *vm, LexerContext *lex);
BppError stmt_palette_handler(VMContext *vm, LexerContext *lex);
#endif

BppError stmt_pause_handler(VMContext *vm, LexerContext *lex);
BppError stmt_get_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sys_handler(VMContext *vm, LexerContext *lex);
BppError stmt_onerr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_assert_handler(VMContext *vm, LexerContext *lex);
BppError stmt_tron_handler(VMContext *vm, LexerContext *lex);
BppError stmt_troff_handler(VMContext *vm, LexerContext *lex);
BppError stmt_break_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vars_handler(VMContext *vm, LexerContext *lex);

/* Register all core statements */
extern BppError stmt_swap_handler(VMContext *vm, LexerContext *lex);
extern BppError stmt_local_handler(VMContext *vm, LexerContext *lex);
extern BppError stmt_static_handler(VMContext *vm, LexerContext *lex);
extern BppError stmt_redim_handler(VMContext *vm, LexerContext *lex);

void register_core_statements(VMContext *vm) {
    stmt_register(vm->stmt_reg, KW_PRINT,  stmt_print_handler,  "PRINT",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DISPLAY,stmt_display_handler,"DISPLAY",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LPRINT, stmt_lprint_handler, "LPRINT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LET,    stmt_let_handler,    "LET",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LSET,   stmt_lset_handler,   "LSET",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RSET,   stmt_rset_handler,   "RSET",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SWAP,   stmt_swap_handler,   "SWAP",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INPUT,  stmt_input_handler,  "INPUT",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RANDOMIZE, stmt_randomize_handler, "RANDOMIZE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEMAND, stmt_input_handler,  "DEMAND", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GOTO,   stmt_goto_handler,   "GOTO",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_GOSUB,  stmt_gosub_handler,  "GOSUB",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_RETURN, stmt_return_handler, "RETURN", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_END,    stmt_end_handler,    "END",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SYSTEM, stmt_system_handler, "SYSTEM", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SHELL,  stmt_shell_handler,  "SHELL",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ENVIRON, stmt_environ_handler, "ENVIRON", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BYE,    stmt_bye_handler,    "BYE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_STOP,   stmt_stop_handler,   "STOP",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_EXIT,   stmt_exit_handler,    "EXIT",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REM,    stmt_rem_handler,    "REM",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_IF,     stmt_if_handler,     "IF",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LIST,   stmt_list_handler,   "LIST",   STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_RUN,    stmt_run_handler,    "RUN",    STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_NEW,    stmt_new_handler,    "NEW",    STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_FOR,    stmt_for_handler,    "FOR",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_NEXT,   stmt_next_handler,   "NEXT",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_WHILE,  stmt_while_handler,  "WHILE",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_WEND,   stmt_wend_handler,   "WEND",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DO,     stmt_do_handler,     "DO",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_LOOP,   stmt_loop_handler,   "LOOP",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DIM,    stmt_dim_handler,    "DIM",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOCAL,  stmt_local_handler,  "LOCAL",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_STATIC, stmt_static_handler, "STATIC", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_REDIM,  stmt_redim_handler,  "REDIM",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ERASE,  stmt_erase_handler,  "ERASE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OPTION, stmt_option_handler, "OPTION", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DATA,   stmt_data_handler,   "DATA",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_READ,   stmt_read_handler,   "READ",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_RESTORE,stmt_restore_handler,"RESTORE",STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DEF,    stmt_defusr_handler, "DEF",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFINT, stmt_defint_handler, "DEFINT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFSNG, stmt_defsng_handler, "DEFSNG", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFDBL, stmt_defdbl_handler, "DEFDBL", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFSTR, stmt_defstr_handler, "DEFSTR", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_USR,    stmt_defusr_handler, "USR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ON,     stmt_on_handler,     "ON",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_TIMER,  stmt_timer_handler,  "TIMER",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ALARM,  stmt_alarm_handler,  "ALARM",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ALARM_STR, stmt_alarm_str_handler, "ALARM$", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SET,    stmt_set_handler,    "SET",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ERROR,  stmt_error_statement_handler, "ERROR", STMT_FLAG_BOTH);
#if SUPPORT_TRY
    stmt_register(vm->stmt_reg, KW_TRY,      stmt_try_handler,      "TRY",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CATCH,    stmt_catch_handler,    "CATCH",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_THROW,    stmt_throw_handler,    "THROW",    STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_ALIAS,    stmt_alias_handler,    "ALIAS",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCOPE,    stmt_scope_handler,    "SCOPE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_KEYWORD,  stmt_keyword_handler,  "KEYWORD",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OVERRIDE, stmt_override_handler, "OVERRIDE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MODULE,   stmt_module_handler,   "MODULE",   STMT_FLAG_BOTH);

    stmt_register(vm->stmt_reg, KW_METADATA, stmt_metadata_handler, "METADATA", STMT_FLAG_BOTH);
#if SUPPORT_TRY
    stmt_register(vm->stmt_reg, KW_RESUME, stmt_resume_handler, "RESUME", STMT_FLAG_PROGRAM);
#endif
#if SUPPORT_OOP
    stmt_register(vm->stmt_reg, KW_TYPE,   stmt_type_handler,   "TYPE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CLASS,  stmt_class_handler,  "CLASS",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_ENUM,   stmt_enum_handler,   "ENUM",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_WITH,   stmt_with_handler,   "WITH",   STMT_FLAG_PROGRAM);
#endif
    stmt_register(vm->stmt_reg, KW_LOAD,   stmt_load_handler,   "LOAD",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SAVE,   stmt_save_handler,   "SAVE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BLOAD,  stmt_bload_handler,  "BLOAD",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BSAVE,  stmt_bsave_handler,  "BSAVE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BRUN,   stmt_brun_handler,   "BRUN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MERGE,  stmt_merge_handler,  "MERGE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_COMMON, stmt_common_handler, "COMMON", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CHAIN,  stmt_chain_handler,  "CHAIN",  STMT_FLAG_BOTH);
#if SUPPORT_HELP
    stmt_register(vm->stmt_reg, KW_SELFTEST,stmt_selftest_handler,"SELFTEST",STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_UNSAVE, stmt_unsave_handler, "UNSAVE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ASSERT, stmt_assert_handler, "ASSERT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TRON,   stmt_tron_handler,   "TRON",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TROFF,  stmt_troff_handler,  "TROFF",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BREAK,  stmt_break_handler,  "BREAK",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VARS,      stmt_vars_handler,      "VARS",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CHECK,     stmt_check_handler,     "CHECK",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VERIFY,    stmt_verify_handler,    "VERIFY",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TEST,      stmt_test_handler,      "TEST",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ENDTEST,   stmt_endtest_handler,   "ENDTEST",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TRACE,     stmt_trace_handler,     "TRACE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEBUG,     stmt_debug_handler,     "DEBUG",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CONT,      stmt_cont_handler,      "CONT",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BACKTRACE, stmt_backtrace_handler, "BACKTRACE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INFO,      stmt_info_handler,      "INFO",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DUMP,      stmt_dump_handler,      "DUMP",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VERSION,   stmt_version_handler,   "VERSION",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VER,       stmt_ver_handler,       "VER",       STMT_FLAG_BOTH);

    /* Phase 3 & 4 Registrations */
#if SUPPORT_FILES
    stmt_register(vm->stmt_reg, KW_OPEN,   stmt_open_handler,   "OPEN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FIELD,  stmt_field_handler,  "FIELD",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLOSE,  stmt_close_handler,  "CLOSE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RESET,  stmt_reset_handler,  "RESET",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GET,    stmt_get_handler,    "GET",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_BGET,   stmt_get_handler,    "BGET",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PUT,    stmt_put_handler,    "PUT",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_BPUT,   stmt_put_handler,    "BPUT",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SEEK,   stmt_seek_handler,   "SEEK",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FILES,  stmt_files_handler,  "FILES",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_KILL,   stmt_kill_handler,   "KILL",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCRATCH,stmt_scratch_handler, "SCRATCH",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CHDIR,  stmt_chdir_handler,  "CHDIR",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MKDIR,  stmt_mkdir_handler,  "MKDIR",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RMDIR,  stmt_rmdir_handler,  "RMDIR",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NAME,   stmt_name_handler,   "NAME",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEVICES,stmt_devices_handler,"DEVICES",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DIR,    stmt_dir_handler,    "DIR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SETATTR,stmt_setattr_handler,"SETATTR",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PWD,      stmt_pwd_handler,      "PWD",      STMT_FLAG_BOTH);
#if SUPPORT_HELP
    stmt_register(vm->stmt_reg, KW_HOSTNAME, stmt_hostname_handler, "HOSTNAME", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_USERNAME, stmt_username_handler, "USERNAME", STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_PATH,     stmt_path_handler,     "PATH",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOCK,   stmt_lock_handler,   "LOCK",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_UNLOCK, stmt_unlock_handler, "UNLOCK", STMT_FLAG_PROGRAM);
#endif
    stmt_register(vm->stmt_reg, KW_SELECT, stmt_select_handler, "SELECT", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CASE,   stmt_case_handler,   "CASE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SUB,      stmt_sub_handler,      "SUB",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PROCEDURE, stmt_procedure_handler, "PROCEDURE", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_FUNCTION, stmt_function_handler, "FUNCTION", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CALL,     stmt_call_handler,     "CALL",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DECLARE,  stmt_declare_handler,  "DECLARE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ENDFUNC,  stmt_end_function_handler, "ENDFUNC", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_ENDPROC,  stmt_end_sub_handler,      "ENDPROC", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_AUTO,    stmt_auto_handler,     "AUTO",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BCOLOR,  stmt_bcolor_handler,   "BCOLOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FCOLOR,  stmt_fcolor_handler,   "FCOLOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLEAR,   stmt_clear_handler,    "CLEAR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CURSOR,  stmt_cursor_handler,   "CURSOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOCATE,  stmt_locate_handler,   "LOCATE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SHARED,  stmt_shared_handler,   "SHARED",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCREEN,  stmt_screen_handler,   "SCREEN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_COLOR,   stmt_color_handler,    "COLOR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLS,     stmt_cls_handler,      "CLS",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HOME,    stmt_home_handler,     "HOME",     STMT_FLAG_BOTH);
#if SUPPORT_GRAPHICS
    stmt_register(vm->stmt_reg, KW_BEEP,    stmt_beep_handler,     "BEEP",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LINE,    stmt_line_handler,     "LINE",     STMT_FLAG_PROGRAM);
#ifndef BASIC_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_CIRCLE,  stmt_circle_handler,   "CIRCLE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PSET,    stmt_pset_handler,     "PSET",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PRESET,  stmt_preset_handler,   "PRESET",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PAINT,   stmt_paint_handler,    "PAINT",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SOUND,   stmt_sound_handler,    "SOUND",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PLAY,    stmt_play_handler,     "PLAY",     STMT_FLAG_PROGRAM);
#endif
#endif
#if SUPPORT_EDITOR && !defined(BASIC_LITE_BUILD)
    stmt_register(vm->stmt_reg, KW_EDIT,    stmt_edit_handler,     "EDIT",     STMT_FLAG_BOTH);
#endif
#if SUPPORT_MAT
    stmt_register(vm->stmt_reg, KW_MAT,      stmt_mat_handler,      "MAT",      STMT_FLAG_BOTH);
#ifndef BASIC_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_ARRAY,    stmt_arrayext_handler, "ARRAY",    STMT_FLAG_BOTH);
#endif
#endif
    stmt_register(vm->stmt_reg, KW_MUX,      stmt_mux_handler,      "MUX",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEMUX,    stmt_demux_handler,    "DEMUX",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_UNPACK,   stmt_unpack_handler,   "UNPACK",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BITMUX,   stmt_bitmux_handler,   "BITMUX",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RENUM,    stmt_renum_handler,    "RENUM",    STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_REFORMAT, stmt_reformat_handler, "REFORMAT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DELETE,   stmt_delete_handler,   "DELETE",   STMT_FLAG_IMMEDIATE);
#if SUPPORT_HELP
    stmt_register(vm->stmt_reg, KW_HELP,     stmt_help_handler,     "HELP",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CATALOG,  stmt_catalog_handler,  "CATALOG",  STMT_FLAG_BOTH);
#endif
#if SUPPORT_HELP
    stmt_register(vm->stmt_reg, KW_REMOVE,     stmt_remove_handler,     "REMOVE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REMOVE_STR, stmt_remove_str_handler, "REMOVE$",    STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_POKE,     stmt_poke_handler,     "POKE",     STMT_FLAG_BOTH);


    /* Phase 11b and 11c statements */
    stmt_register(vm->stmt_reg, KW_NOISE,      stmt_noise_handler,      "NOISE",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SNDPLAY,    stmt_sndplay_handler,    "_SNDPLAY",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDLOOP,    stmt_sndloop_handler,    "_SNDLOOP",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDSTOP,    stmt_sndstop_handler,    "_SNDSTOP",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDPAUSE,   stmt_sndpause_handler,   "_SNDPAUSE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDVOL,     stmt_sndvol_handler,     "_SNDVOL",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSEINPUT, stmt_mouseinput_handler, "_MOUSEINPUT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSEHIDE,  stmt_mousehide_handler,  "_MOUSEHIDE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSESHOW,  stmt_mouseshow_handler,  "_MOUSESHOW",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSE,      stmt_mouse_handler,      "MOUSE",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HMOUSE,     stmt_hmouse_handler,     "HMOUSE",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VMOUSE,     stmt_vmouse_handler,     "VMOUSE",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TRIG,       stmt_trig_handler,       "TRIG",        STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TITLE,      stmt_title_handler,      "_TITLE",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCREENMOVE, stmt_screenmove_handler, "_SCREENMOVE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FULLSCREEN, stmt_fullscreen_handler, "_FULLSCREEN", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RESIZE,     stmt_resize_handler,     "_RESIZE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ICON,       stmt_icon_handler,       "_ICON",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NWRITE,     stmt_nwrite_handler,     "NWRITE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FREEIMAGE,  stmt_freeimage_handler,  "_FREEIMAGE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PUTIMAGE,   stmt_putimage_handler,   "_PUTIMAGE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_STATESAVE,  stmt_statesave_handler,  "_STATESAVE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_STATELOAD,  stmt_stateload_handler,  "_STATELOAD",  STMT_FLAG_BOTH);

    /* Legacy compatibility statements */
#if SUPPORT_GRAPHICS
#ifndef BASIC_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_GR,       stmt_gr_handler,       "GR",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HGR,      stmt_hgr_handler,      "HGR",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HGR2,     stmt_hgr2_handler,     "HGR2",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HCOLOR,   stmt_hcolor_handler,   "HCOLOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PLOT,     stmt_plot_handler,     "PLOT",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HLIN,     stmt_hlin_handler,     "HLIN",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VLIN,     stmt_vlin_handler,     "VLIN",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HPLOT,    stmt_hplot_handler,    "HPLOT",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GRAPHICS, stmt_graphics_handler, "GRAPHICS", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DRAWTO,   stmt_drawto_handler,   "DRAWTO",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BORDER,   stmt_border_handler,   "BORDER",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INK,      stmt_ink_handler,      "INK",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PAPER,    stmt_paper_handler,    "PAPER",    STMT_FLAG_BOTH);

    /* BGI (BASIC++ Graphics Interface) statements */
    stmt_register(vm->stmt_reg, KW_INITGRAPH,  stmt_initgraph_handler,  "INITGRAPH",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLOSEGRAPH, stmt_closegraph_handler, "CLOSEGRAPH", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PUTPIXEL,   stmt_putpixel_handler,   "PUTPIXEL",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BAR,        stmt_bar_handler,        "BAR",        STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ELLIPSE,    stmt_ellipse_handler,    "ELLIPSE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RECTANGLE,  stmt_rectangle_handler,  "RECTANGLE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OUTTEXTXY,  stmt_outtextxy_handler,  "OUTTEXTXY",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PALETTE,    stmt_palette_handler,    "PALETTE",    STMT_FLAG_BOTH);
#endif
#endif

    stmt_register(vm->stmt_reg, KW_PAUSE,    stmt_pause_handler,    "PAUSE",    STMT_FLAG_BOTH);
#if SUPPORT_FILES
    stmt_register(vm->stmt_reg, KW_GET,      stmt_get_handler,      "GET",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BGET,     stmt_get_handler,      "BGET",     STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_SYS,      stmt_sys_handler,      "SYS",      STMT_FLAG_BOTH);
#if SUPPORT_TRY
    stmt_register(vm->stmt_reg, KW_ONERR,    stmt_onerr_handler,    "ONERR",    STMT_FLAG_BOTH);
#endif
}

static BppError execute_directive(VMContext *vm, LexerContext *lex, BppToken dir_tok) {
    BppError err;
    memset(&err, 0, sizeof(err));

    char dir_name[64];
    int len = (int)(dir_tok.length < sizeof(dir_name) - 1 ? dir_tok.length : sizeof(dir_name) - 1);
    memcpy(dir_name, dir_tok.as.string, len);
    dir_name[len] = '\0';

    if (strcasecmp(dir_name, "OPTION") == 0) {
        BppToken opt_tok = lex_next(lex);
        if (opt_tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected option name in ::OPTION";
            return err;
        }
        char opt_name[64];
        int opt_len = (int)(opt_tok.length < sizeof(opt_name) - 1 ? opt_tok.length : sizeof(opt_name) - 1);
        memcpy(opt_name, opt_tok.start, opt_len);
        opt_name[opt_len] = '\0';

        BppMetadataRegistry *reg = vm_get_metadata(vm);
        if (strcasecmp(opt_name, "STRICT") == 0 || strcasecmp(opt_name, "EXPLICIT") == 0) {
            if (reg) reg->option_strict = true;
        } else {
            err.code = 2; err.message = "Unsupported option in ::OPTION";
            return err;
        }
    }
    else if (strcasecmp(dir_name, "INCLUDE") == 0) {
        /* Already statically loaded at parse/pre-scan time; consume string argument as NOP */
        BppToken val_tok = lex_next(lex);
        if (val_tok.type != TOK_STRING) {
            err.code = 2; err.message = "Expected string argument for ::INCLUDE";
            return err;
        }
    }
    else {
        err.code = 2; err.message = "Unsupported compiler directive";
    }

    return err;
}

static bool is_block_end_marker(const char *text, const char *block_type, const char *block_target, MemoryContext *mem) {
    LexerContext *check_lex = lex_init(mem, text);
    if (!check_lex) return false;

    BppToken tok = lex_next(check_lex);
    
    /* 1. Check for 'END {block_type}' */
    if ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 3 && strncasecmp(tok.start, "END", 3) == 0) {
        BppToken next_tok = lex_next(check_lex);
        if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
            char next_name[64];
            size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
            memcpy(next_name, next_tok.start, nlen);
            next_name[nlen] = '\0';
            if (strcasecmp(next_name, block_type) == 0) {
                lex_shutdown(check_lex);
                return true;
            }
        }
    }

    /* 2. Check for target-qualified endings: '{target} {block_type}::' */
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_STRING) {
        char target_buf[64];
        size_t tlen = (tok.length < 63) ? tok.length : 63;
        memcpy(target_buf, tok.start, tlen);
        target_buf[tlen] = '\0';

        if (tok.start + tok.length < text + strlen(text) && *(tok.start + tok.length) == ':') {
            if (tlen + 1 < 63) {
                target_buf[tlen] = ':';
                target_buf[tlen + 1] = '\0';
            }
        }

        if (strcasecmp(target_buf, block_target) == 0) {
            BppToken next_tok = lex_next(check_lex);
            if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
                char next_name[64];
                size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
                memcpy(next_name, next_tok.start, nlen);
                next_name[nlen] = '\0';

                if (strcasecmp(next_name, block_type) == 0 && lex_peek(check_lex).type == TOK_DOUBLE_COLON) {
                    lex_shutdown(check_lex);
                    return true;
                }
            }
        }
    }

    /* 3. Check for standard block ending: '{block_type}::' */
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char name_buf[64];
        size_t len = (tok.length < 63) ? tok.length : 63;
        memcpy(name_buf, tok.start, len);
        name_buf[len] = '\0';

        if (strcasecmp(name_buf, block_type) == 0 && lex_peek(check_lex).type == TOK_DOUBLE_COLON) {
            lex_shutdown(check_lex);
            return true;
        }
    }

    lex_shutdown(check_lex);
    return false;
}

static BppError skip_metadata_block(VMContext *vm, LexerContext *lex, const char *block_type) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Parse target name from the current line for qualified ending checks */
    char block_target[64] = "";
    BppToken target_tok = lex_peek(lex);
    if (target_tok.type == TOK_IDENT || target_tok.type == TOK_KEYWORD || target_tok.type == TOK_STRING) {
        size_t tlen = (target_tok.length < 63) ? target_tok.length : 63;
        memcpy(block_target, target_tok.start, tlen);
        block_target[tlen] = '\0';
        
        if (target_tok.start + target_tok.length < target_tok.start + 100 && *(target_tok.start + target_tok.length) == ':') {
            if (tlen + 1 < 63) {
                block_target[tlen] = ':';
                block_target[tlen + 1] = '\0';
            }
        }
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    BppLineNumber curr_line = vm_get_current_line(vm);

    size_t start_idx = 0;
    bool found_start = false;
    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number == curr_line) {
            start_idx = i;
            found_start = true;
            break;
        }
    }
    if (!found_start) return err;

    /* Scan forward to see if a closing marker exists */
    bool has_closing = false;
    for (size_t i = start_idx; i < count; i++) {
        if (is_block_end_marker(lines[i].text, block_type, block_target, vm_get_mem(vm))) {
            has_closing = true;
            break;
        }
    }

    if (!has_closing) {
        /* Single-line block, consume the rest of the current line so it isn't executed as code */
        BppToken skip_tok = lex_next(lex);
        while (skip_tok.type != TOK_EOF && skip_tok.type != TOK_EOL) {
            skip_tok = lex_next(lex);
        }
        return err;
    }

    /* Jump to the line containing the matching block end marker */
    for (size_t i = start_idx; i < count; i++) {
        if (is_block_end_marker(lines[i].text, block_type, block_target, vm_get_mem(vm))) {
            vm_jump(vm, lines[i].line_number + 0.0001, NULL); /* Jump past this line (fractional safe) */
            return err;
        }
    }

    err.code = 2; err.message = "Block defined without closing block marker";
    return err;
}

/* Execute custom dynamically-registered keywords */
static BppError execute_custom_keyword_statement(VMContext *vm, LexerContext *lex, BppKeywordId kw) {
    BppError err;
    memset(&err, 0, sizeof(err));

    SpecObject *spec = spec_find_by_kw_id(kw);
    if (!spec) {
        err.code = 2;
        err.message = "Unknown dynamic keyword";
        return err;
    }

    /* Check required security level */
    int req_lvl_val = security_find_level_by_name(spec->required_level);
    BppSecLevel req_lvl = (req_lvl_val >= 0) ? (BppSecLevel)req_lvl_val : SEC_STANDARD;
    if (security_check_pinned_level(req_lvl) == 0) {
        err.code = 70;
        err.message = "Permission denied executing dynamic keyword statement (restricted via security pin)";
        return err;
    }

    /* Check if next token is a subcommand name */
    char sub_name[256] = "";
    BppToken next_tok = lex_peek(lex);
    bool has_subcommand = false;
    if (next_tok.type == TOK_IDENT) {
        char subcmd[128];
        size_t slen = (next_tok.length < sizeof(subcmd) - 1) ? next_tok.length : sizeof(subcmd) - 1;
        memcpy(subcmd, next_tok.start, slen);
        subcmd[slen] = '\0';
        
        /* Look for SUB specName.subcmd or SUB subcmd in library */
        char target1[256];
        snprintf(target1, sizeof(target1), "%s.%s", spec->name, subcmd);
        
        BppLineNumber def_line = 0.0;
        const char *def_text = NULL;
        if (find_procedure(vm, target1, KW_SUB, &def_line, &def_text)) {
            strncpy(sub_name, target1, sizeof(sub_name) - 1);
            sub_name[sizeof(sub_name) - 1] = '\0';
            has_subcommand = true;
            lex_next(lex); /* Consume subcommand identifier */
        } else if (find_procedure(vm, subcmd, KW_SUB, &def_line, &def_text)) {
            strncpy(sub_name, subcmd, sizeof(sub_name) - 1);
            sub_name[sizeof(sub_name) - 1] = '\0';
            has_subcommand = true;
            lex_next(lex); /* Consume subcommand identifier */
        }
    }

    if (!has_subcommand) {
        /* No subcommand matched. Look for a subroutine with the same name as the spec itself */
        BppLineNumber def_line = 0.0;
        const char *def_text = NULL;
        if (find_procedure(vm, spec->name, KW_SUB, &def_line, &def_text)) {
            strncpy(sub_name, spec->name, sizeof(sub_name) - 1);
            sub_name[sizeof(sub_name) - 1] = '\0';
        } else {
            err.code = 35;
            err.message = "Subprogram or subcommand not defined for dynamic feature";
            return err;
        }
    }

    /* Evaluate arguments (comma or space separated) */
    BValue args[MAX_PARAMS];
    int arg_count = 0;

    /* Read arguments until TOK_EOL, TOK_EOF, or colon (end of statement) */
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }

        if (arg_count >= MAX_PARAMS) {
            err.code = 2; err.message = "Too many arguments in custom statement call";
            for (int i = 0; i < arg_count; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
            }
            return err;
        }

        args[arg_count++] = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            for (int i = 0; i < arg_count - 1; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
            }
            return err;
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        }
    }

    const char *ret_pos = lex_get_pos(lex);
    err = vm_call_sub_procedure(vm, sub_name, args, arg_count, ret_pos);
    return err;
}

static BppError dispatch_gosub_target(VMContext *vm, LexerContext *lex, BppLineNumber line) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!mem_program_get(vm_get_mem(vm), line)) {
        err.code = 8;
        err.message = "Undefined line number in GOSUB hook/override";
        return err;
    }
    if (!vm_gosub_push(vm, vm_get_current_line(vm), lex_get_pos(lex))) {
        err.code = 12;
        err.message = "Subroutine nesting limit exceeded";
        return err;
    }
    vm_jump(vm, line, NULL);
    return err;
}

/* Execute a single statement in the current lexer stream */
BppError execute_single_statement(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    vm->current_stmt_pos = tok.start;
    BppKeywordId kw = KW_NONE;

    if (tok.type == TOK_DOCSTRING || tok.type == TOK_NAMESPACE_DECL || tok.type == TOK_GLOBAL_LABEL) {
        lex_next(lex); /* Consume the NOP token */
        return err;
    }

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char word_buf[64];
        size_t w_len = (tok.length < 63) ? tok.length : 63;
        memcpy(word_buf, tok.start, w_len);
        word_buf[w_len] = '\0';

        const char *expansion = vm_lookup_alias(vm, word_buf);
        if (expansion && vm->alias_expansion_depth < 10) {
            lex_next(lex); /* Consume alias token */
            const char *rest = lex_get_pos(lex);

            char *expanded = (char *)mem_scratch_alloc(vm_get_mem(vm), strlen(expansion) + strlen(rest) + 2);
            if (!expanded) {
                err.code = 14;
                err.message = "Scratch memory exhausted during alias expansion";
                return err;
            }
            snprintf(expanded, strlen(expansion) + strlen(rest) + 2, "%s %s", expansion, rest);

            while (tok.type != TOK_EOF && tok.type != TOK_EOL) {
                lex_next(lex);
                tok = lex_peek(lex);
            }

            vm->alias_expansion_depth++;
            err = vm_execute_line(vm, expanded);
            vm->alias_expansion_depth--;
            return err;
        }
    }

    if (tok.type == TOK_DIRECTIVE) {
        lex_next(lex); /* Consume directive token */

        char dir_name[64];
        size_t len = (tok.length < 63) ? tok.length : 63;
        memcpy(dir_name, tok.as.string, len);
        dir_name[len] = '\0';

        if (strcasecmp(dir_name, "KEYWORD") == 0 || strcasecmp(dir_name, "SCOPE") == 0 || strcasecmp(dir_name, "ALIAS") == 0 || strcasecmp(dir_name, "OPTION") == 0) {
            err = skip_metadata_block(vm, lex, dir_name);
            return err;
        }

        err = execute_directive(vm, lex, tok);
        return err;
    }

    if (tok.type == TOK_PERIOD && vm_with_stack_peek(vm) != NULL) {
        /* Peek past the dot and identifier(s) to see if it is a method call or assignment */
        bool is_method_call = false;
        LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        if (temp_lex) {

            lex_next(temp_lex); /* Consume '.' */
            BppToken sub = lex_next(temp_lex); /* Consume identifier */
            (void)sub;
            while (lex_peek(temp_lex).type == TOK_PERIOD) {
                lex_next(temp_lex);
                lex_next(temp_lex);
            }
            if (lex_peek(temp_lex).type == TOK_LPAREN) {
                is_method_call = true;
            }
            lex_shutdown(temp_lex);
        }
        
        if (is_method_call) {
            BValue res = eval_expression(vm, lex, &err);
            if (err.code == 0) {
                if (res.type == VAL_STRING && res.as.string) {
                    str_release(vm_get_str(vm), res.as.string);
                } else if (res.type == VAL_MAP && res.as.map) {
                    map_release(vm_get_str(vm), res.as.map);
                }
            }
            return err;
        }
        
        kw = KW_LET;
    } else if (tok.type == TOK_KEYWORD) {
        kw = tok.as.keyword;
        if (kw >= 1000 || kw == KW_MOUSE || kw == KW_HMOUSE || kw == KW_VMOUSE || kw == KW_TRIG) {
            /* Peek next token to see if it is '=' */
            LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            if (temp_lex) {

                lex_next(temp_lex); /* Consume keyword first */
            }
            BppToken next_tok = lex_next(temp_lex);
            lex_shutdown(temp_lex);
            if (next_tok.type == TOK_EQ) {
                kw = KW_LET; /* Treat as implicit LET assignment */
            } else {
                lex_next(lex); /* Consume keyword */
            }
        } else {
            lex_next(lex); /* Consume keyword */
        }
    } else if (tok.type == TOK_IDENT) {
        /* Check if this is a method call statement: e.g. obj.method(...) */
        bool is_method_call = false;
        if (memchr(tok.start, '.', tok.length) != NULL) {
            /* Clone lexer to scan ahead and see if the token is followed by '(' */
            LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            if (temp_lex) {

                lex_next(temp_lex); /* Consume current identifier */
                BppToken next_tok = lex_next(temp_lex);
                if (next_tok.type == TOK_LPAREN) {
                    is_method_call = true;
                }
                lex_shutdown(temp_lex);
            }
        }
        
        if (is_method_call) {
            /* Evaluate the method call expression and discard its return value */
            BValue res = eval_expression(vm, lex, &err);
            if (err.code == 0) {
                if (res.type == VAL_STRING && res.as.string) {
                    str_release(vm_get_str(vm), res.as.string);
                } else if (res.type == VAL_MAP && res.as.map) {
                    map_release(vm_get_str(vm), res.as.map);
                }
            }
            return err;
        }
        
        /* Check if this is a call-less subroutine/procedure call */
        char sub_name[256];
        size_t slen = (tok.length < sizeof(sub_name) - 1) ? tok.length : sizeof(sub_name) - 1;
        memcpy(sub_name, tok.start, slen);
        sub_name[slen] = '\0';
        if (find_procedure(vm, sub_name, KW_SUB, NULL, NULL)) {
            kw = KW_CALL;
        } else {
            /* Implicit LET */
            kw = KW_LET;
        }
    } else if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        /* NOP */
        return err;
    } else {
        err.code = 2; /* Syntax error */
        err.message = "Expected statement keyword or assignment";
        return err;
    }

    if (security_is_keyword_restricted((int)kw)) {
        err.code = 70;
        err.message = "Keyword restricted via SECURITY RESTRICT";
        return err;
    }

    if (kw >= 1000) {
        err = execute_custom_keyword_statement(vm, lex, kw);
        return err;
    }

    /* Scan ahead in a cloned lexer to find all trailing unnested IF or UNLESS */
    const char *postfix_ptrs[16];
    BppKeywordId postfix_kws[16];
    int postfix_count = 0;
    
    if (kw != KW_IF && kw != KW_UNLESS && kw != KW_FOR && kw != KW_WHILE &&
        kw != KW_DO && kw != KW_SELECT && kw != KW_SUB && kw != KW_FUNCTION && kw != KW_DECLARE) {
        
        int open_parens = 0;
        LexerContext *scan_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        if (scan_lex) {

            BppToken scan_tok = lex_next(scan_lex);
            while (scan_tok.type != TOK_EOF && scan_tok.type != TOK_EOL) {
                if (scan_tok.type == TOK_LPAREN) open_parens++;
                else if (scan_tok.type == TOK_RPAREN) open_parens--;
                else if (open_parens == 0 && scan_tok.type == TOK_KEYWORD && 
                         (scan_tok.as.keyword == KW_IF || scan_tok.as.keyword == KW_UNLESS)) {
                    if (postfix_count < 16) {
                        postfix_ptrs[postfix_count] = scan_tok.start;
                        postfix_kws[postfix_count] = scan_tok.as.keyword;
                        postfix_count++;
                    }
                }
                scan_tok = lex_next(scan_lex);
            }
            lex_shutdown(scan_lex);
        }
    }

    bool condition_met = true;
    const char *postfix_end_pos = NULL;
    
    for (int i = postfix_count - 1; i >= 0; i--) {
        const char *p_pos = postfix_ptrs[i];
        BppKeywordId p_kw = postfix_kws[i];
        
        char saved_char = '\0';
        char *mutable_next = NULL;
        if (i + 1 < postfix_count) {
            mutable_next = (char *)postfix_ptrs[i + 1];
            saved_char = *mutable_next;
            *mutable_next = '\0';
        }
        
        LexerContext *cond_lex = lex_init(vm_get_mem(vm), p_pos);
        if (cond_lex) {

            lex_next(cond_lex); /* Consume IF/UNLESS keyword */
            BValue cond_val = eval_expression(vm, cond_lex, &err);
            if (err.code == 0) {
                if (cond_val.type == VAL_STRING) {
                    err.code = 13;
                    err.message = "String expression not allowed in postfix conditional";
                } else {
                    bool truth = (cond_val.as.number != 0.0);
                    condition_met = (p_kw == KW_IF) ? truth : !truth;
                    if (i == postfix_count - 1) {
                        postfix_end_pos = lex_get_pos(cond_lex);
                    }
                }
            }
            lex_shutdown(cond_lex);
        }
        
        if (mutable_next) {
            *mutable_next = saved_char;
        }
        
        if (err.code != 0) {
            if (err.code == 2 && vm->opt_eh) {
                err.message = "Eh?";
            }
            return err;
        }
        
        if (!condition_met) {
            break;
        }
    }

    if (!condition_met) {
        BppToken skip_tok = lex_peek(lex);
        while (skip_tok.type != TOK_EOF && skip_tok.type != TOK_EOL) {
            lex_next(lex);
            skip_tok = lex_peek(lex);
        }
        return err;
    }

    BppStmtHandler handler = stmt_lookup(vm->stmt_reg, kw);
    if (!handler) {
        err.code = 2; /* Syntax error */
        err.message = "Unknown command";
        if (vm->opt_eh) err.message = "Eh?";
        return err;
    }

    char *mutable_postfix = (postfix_count > 0) ? (char *)postfix_ptrs[0] : NULL;
    char saved_char = '\0';
    if (postfix_count > 0 && mutable_postfix) {
        saved_char = *mutable_postfix;
        *mutable_postfix = '\0';
    }

    /* Execute the registered handler callback or hook/override */
    const char *kw_name = lex_keyword_name(kw);

    if (kw_name && scope_is_keyword_disabled(vm, kw_name)) {
        err.code = 13;
        err.message = "Keyword disabled by SCOPE";
        return err;
    }

    const BppScopeHook *before_hk = kw_name ? scope_lookup_hook(vm, kw_name, HOOK_BEFORE) : NULL;
    if (before_hk && before_hk->target_line > 0) {
        dispatch_gosub_target(vm, lex, (BppLineNumber)before_hk->target_line);
    }

    const BppOverrideEntry *ovr = kw_name ? override_lookup(vm, kw_name) : NULL;
    const BppScopeHook *ovr_hk = kw_name ? scope_lookup_hook(vm, kw_name, HOOK_OVERRIDE) : NULL;

    if (ovr && ovr->is_active) {
        if (ovr->target_line > 0) {
            err = dispatch_gosub_target(vm, lex, (BppLineNumber)ovr->target_line);
        } else if (ovr->replacement_sub[0] != '\0') {
            err = vm_call_sub_procedure(vm, ovr->replacement_sub, NULL, 0, NULL);
        }
    } else if (ovr_hk) {
        if (ovr_hk->target_line > 0) {
            err = dispatch_gosub_target(vm, lex, (BppLineNumber)ovr_hk->target_line);
        }
    } else {
        err = handler(vm, lex);
    }

    const BppScopeHook *after_hk = (err.code == 0 && kw_name) ? scope_lookup_hook(vm, kw_name, HOOK_AFTER) : NULL;
    if (after_hk && after_hk->target_line > 0) {
        dispatch_gosub_target(vm, lex, (BppLineNumber)after_hk->target_line);
    }

    /* Restore the original character */
    if (postfix_count > 0 && mutable_postfix) {
        *mutable_postfix = saved_char;
    }

    if (err.code == 0 && postfix_count > 0) {
        lex_set_pos(lex, postfix_end_pos);
    }
    if (err.code != 0 && vm->opt_eh) {
        err.message = "Eh?";
    }
    return err;
}

static void get_namespace_at_line(VMContext *vm, BppLineNumber target_line, char *out_ns, size_t max_len) {
    out_ns[0] = '\0';
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    char current_ns[64] = "";

    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number > target_line) {
            break;
        }
        LexerContext *scan_lex = lex_init(vm_get_mem(vm), lines[i].text);
        if (scan_lex) {
            BppToken tok = lex_next(scan_lex);
            if (tok.type == TOK_NAMESPACE_DECL) {
                int len = (int)(tok.length < sizeof(current_ns) - 1 ? tok.length : sizeof(current_ns) - 1);
                memcpy(current_ns, tok.as.string, len);
                current_ns[len] = '\0';
                if (strcasecmp(current_ns, "DEFAULT") == 0) {
                    current_ns[0] = '\0';
                }
            }
            lex_shutdown(scan_lex);
        }
    }
    size_t copy_len = strlen(current_ns);
    if (copy_len >= max_len) copy_len = max_len - 1;
    memcpy(out_ns, current_ns, copy_len);
    out_ns[copy_len] = '\0';
}

/* Execute a whole line of code, handling colons for multi-statement execution */
BppError vm_execute_line(VMContext *vm, const char *source) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !source) return err;


    /* Make a copy of the source line to prevent use-after-free if RUN/LOAD clears the program memory */
    size_t slen = strlen(source);
    char *source_copy = (char *)calloc(1, slen + 1);
    if (!source_copy) {
        err.code = 14; /* Out of memory */
        err.message = "Failed to copy statement source line";
        return err;
    }
    memcpy(source_copy, source, slen + 1);

    vm->active_line_original = source;
    vm->active_line_copy = source_copy;

    char ns[64];
    get_namespace_at_line(vm, vm->current_line, ns, sizeof(ns));
    var_set_namespace(vm->var, ns);

    const char *start_pos = source_copy;
    if (vm->current_pos && vm->current_pos >= source && vm->current_pos <= source + strlen(source)) {
        ptrdiff_t offset = vm->current_pos - source;
        /* Clamp offset if processed source size changed */
        if (offset >= 0 && (size_t)offset <= slen) {
            start_pos = source_copy + offset;
        }
    }
    vm->current_pos = NULL;

    /* Reset the expression recursion depth counter at the start of each line.
     * This prevents stale depth values from error-path early returns from
     * carrying over between statements. */
    vm->eval_depth = 0;

    LexerContext *lex = lex_init(vm->mem, start_pos);
    if (!lex) {
        free(source_copy);
        err.code = 14; /* Out of memory */
        err.message = "Failed to initialize statement parser";
        return err;
    }


    /* Save the running state at entry. If we're NOT in the program execution
     * loop (i.e. REPL immediate mode), we must not gate the loop on vm->running
     * or no statement will ever execute. We only check vm->running when the
     * caller was already running a program (to allow END/STOP to halt). */
    bool was_running = vm->running;

    BppToken tok = lex_peek(lex);
    while (tok.type != TOK_EOF && (was_running ? vm->running : true)) {
        /* Map current_pos back to the original source string */
        ptrdiff_t offset = tok.start - source_copy;
        vm->current_pos = source + offset;

        err = execute_single_statement(vm, lex);
        if (err.code != 0) {
            if (file_txn_status(vm->file) == 2) {
                file_txn_rollback(vm->file);
            }
            /* Scan forward to next statement separator (TOK_EOL) or EOF */
            BppToken skip_tok = lex_peek(lex);
            while (skip_tok.type != TOK_EOF && skip_tok.type != TOK_EOL) {
                lex_next(lex);
                skip_tok = lex_peek(lex);
            }
            if (skip_tok.type == TOK_EOL) {
                lex_next(lex); /* Consume separator or EOL */
            }
            ptrdiff_t next_offset = lex_get_pos(lex) - source_copy;
            vm->next_pos = source + next_offset;
            break;
        }

        if (vm->jump_active) {
            if (vm->next_line == vm->current_line) {
                ptrdiff_t jmp_offset = vm->next_pos - source;
                if (jmp_offset >= 0 && (size_t)jmp_offset <= slen) {
                    lex_shutdown(lex);
                    lex = lex_init(vm->mem, source_copy + jmp_offset);
                    if (lex) {

                        vm->jump_active = false;
                        tok = lex_peek(lex);
                        continue;
                    }
                }
            }
            /* Control flow jump broke execution of the rest of the line */
            break;
        }

        tok = lex_peek(lex);
        ptrdiff_t next_offset = tok.start - source_copy;
        vm->next_pos = source + next_offset;

        if (tok.type == TOK_EOL) {
            lex_next(lex); /* Consume separator or EOL */
            tok = lex_peek(lex);
        }
    }

    lex_shutdown(lex);
    vm->active_line_original = NULL;
    vm->active_line_copy = NULL;
    free(source_copy);


    return err;
}

void vm_set_debug_hook(VMContext *vm, void (*hook)(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data), void *user_data) {
    if (vm) {
        vm->debug_hook = hook;
        vm->debug_user_data = user_data;
    }
}

bool vm_get_single_step(VMContext *vm) {
    return vm ? vm->debug_single_step : false;
}

void vm_set_single_step(VMContext *vm, bool enable) {
    if (vm) {
        vm->debug_single_step = enable;
    }
}

void vm_trigger_breakpoint(VMContext *vm, const char *reason) {
    if (!vm) return;
    VDevContext *vdev = vm->vdev;

    if (vdev) {
        vdev_printf(vdev, "\n[BREAKPOINT] at line %lld: %s\n", (long long)vm->current_line, reason ? reason : "unknown");
        vdev_printf(vdev, "Commands: [s] Step, [c] Continue, [v] View Variables (VARS), [q] Quit\n");
    }
    log_warn("Breakpoint triggered at line %lld: %s", (long long)vm->current_line, reason ? reason : "unknown");

    while (true) {
        if (vdev) {
            vdev_printf(vdev, "debug> ");
        }
        char cmd_line[128] = {0};
        if (!fgets(cmd_line, sizeof(cmd_line), stdin)) {
            vm_halt(vm);
            break;
        }
        size_t len = strlen(cmd_line);
        while (len > 0 && (cmd_line[len - 1] == '\n' || cmd_line[len - 1] == '\r')) {
            cmd_line[len - 1] = '\0';
            len--;
        }

        if (strcmp(cmd_line, "s") == 0 || strcmp(cmd_line, "S") == 0 || len == 0) {
            vm->debug_single_step = true;
            break;
        } else if (strcmp(cmd_line, "c") == 0 || strcmp(cmd_line, "C") == 0) {
            vm->debug_single_step = false;
            break;
        } else if (strcmp(cmd_line, "v") == 0 || strcmp(cmd_line, "V") == 0) {
            var_print_all(vm->var, vdev);
        } else if (strcmp(cmd_line, "q") == 0 || strcmp(cmd_line, "Q") == 0) {
            vm_halt(vm);
            break;
        } else {
            if (vdev) {
                vdev_printf(vdev, "Unknown debug command. Use: s, c, v, q\n");
            }
        }
    }
}

void vm_run_program(VMContext *vm) {
    if (!vm) return;

    /* Initialize ERR and ERL variables to 0 */
    BValue *p_err = var_lookup(vm->var, "ERR", true);
    if (p_err) {
        p_err->type = VAL_NUMBER;
        p_err->as.number = 0.0;
    }
    BValue *p_erl = var_lookup(vm->var, "ERL", true);
    if (p_erl) {
        p_erl->type = VAL_NUMBER;
        p_erl->as.number = 0.0;
    }

    vm_build_data_table(vm);
    vm->running = true;
    vm->jump_active = false;
    vm_clear_error(vm);
    gosub_stack_clear(vm->gosub_stack);
    for_stack_clear(vm->for_stack);
    while_stack_clear(vm->while_stack);
    do_stack_clear(vm->do_stack);

    /* Fetch minimum line number */
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm->mem, &count);
    if (count == 0) {
        vm->running = false;
        return; /* No lines to run */
    }

    if (vm->start_line > 0.0) {
        vm->current_line = vm->start_line;
        vm->start_line = 0.0;
    } else {
        vm->current_line = lines[0].line_number;
    }
    vm->current_pos = NULL;

    while (vm->running && !vm_has_error(vm)) {
        if (vm->jump_active) {
            vm->current_line = vm->next_line;
            vm->current_pos = vm->next_pos;
            vm->jump_active = false;
        }

        lines = mem_program_get_all(vm->mem, &count);
        size_t idx = 0;
        bool found = false;
        bool is_lib = false;

        /* Fetch target line number from main program */
        for (size_t i = 0; i < count; ++i) {
            if (lines[i].line_number == vm->current_line) {
                idx = i;
                found = true;
                break;
            }
        }

        /* If not found, fetch target line number from library program */
        size_t lib_count = 0;
        BppProgramLine *lib_lines = mem_lib_program_get_all(vm->mem, &lib_count);
        if (!found && lib_lines) {
            for (size_t i = 0; i < lib_count; ++i) {
                if (lib_lines[i].line_number == vm->current_line) {
                    idx = i;
                    found = true;
                    is_lib = true;
                    break;
                }
            }
        }

        if (!found) {
            /* Find first line number >= current_line in main program */
            for (size_t i = 0; i < count; ++i) {
                if (lines[i].line_number >= vm->current_line) {
                    idx = i;
                    found = true;
                    vm->current_line = lines[i].line_number;
                    break;
                }
            }
        }

        if (!found && lib_lines) {
            /* Find first line number >= current_line in library program */
            for (size_t i = 0; i < lib_count; ++i) {
                if (lib_lines[i].line_number >= vm->current_line) {
                    idx = i;
                    found = true;
                    is_lib = true;
                    vm->current_line = lib_lines[i].line_number;
                    break;
                }
            }
        }

        if (!found) {
            /* Out of program boundaries */
            vm->running = false;
            break;
        }

        /* Reset the scratch arena before each program line runs */
        mem_scratch_reset(vm->mem);

        BppProgramLine *active_lines = is_lib ? lib_lines : lines;
        size_t active_count = is_lib ? lib_count : count;

        if (logger_is_trace()) {
            VDevContext *vdev = vm->vdev;
            if (vdev) {
                vdev_printf(vdev, "[Line %lld]\n", (long long)vm->current_line);
            }
            log_info("[Line %lld]", (long long)vm->current_line);
        }

        if (vm->debug_single_step) {
            char reason_buf[128];
            snprintf(reason_buf, sizeof(reason_buf), "Line %lld: %s", (long long)vm->current_line, active_lines[idx].text);
            vm_trigger_breakpoint(vm, reason_buf);
        }

        if (vm->debug_hook) {
            vm->debug_hook(vm, "line", (int)vm->current_line, NULL, vm->debug_user_data);
        }

        BppError err = vm_execute_line(vm, active_lines[idx].text);
#ifndef BASIC_LITE_BUILD
        vdev_gfx_poll_events();
#endif
        if (err.code != 0) {
            if (try_stack_count(vm->try_stack) > 0) {
                vm_trigger_try_catch_handler(vm, err.code, err.message);
                memset(&vm->last_error, 0, sizeof(BppError));
                vm->jump_active = true;
            } else if (vm->error_trap_line > 0.0 && !vm->in_error_handler) {
                BppLineNumber err_ln = (err.line != 0.0) ? err.line : vm->current_line;
                vm_trigger_error_trap(vm, err.code, err_ln, vm->current_pos, vm->next_pos);
                memset(&vm->last_error, 0, sizeof(BppError));
                vm->jump_active = true;
            } else {
                vm->last_error = err;
                if (vm->last_error.line == 0.0) {
                    vm->last_error.line = vm->current_line;
                }
                break;
            }
        }

        if (err.code == 0) {
            vm_trigger_event_polling(vm);
        }

        if (!vm->jump_active) {
            if (idx + 1 < active_count) {
                vm->current_line = active_lines[idx + 1].line_number;
            } else {
                vm->running = false; /* Finished last line */
            }
        }

    }

    if (vm->break_triggered) {
        vdev_printf(vm->vdev, "Break at line %lld\n", (long long)vm->current_line);
        vm->break_triggered = false;
    }
}

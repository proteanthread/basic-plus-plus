/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_internal.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Statement dispatch and recursive-descent expression parsing.
 *
 * 2. WHAT TO EXPECT:
 *    Highly recursive execution structure. Expression evaluation strictly obeys operator precedence levels.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Syntax parsing details for expressions, parser diagnostic logging, and parsing warnings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Operator precedence hierarchy, recursive call stack structure, keyword parsing dispatch methods.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Increase stack allocation limit if stack overflow occurs. Ensure expression parentheses evaluate in correct recursion.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_internal.h
 // ---
 //
 // Internal header shared by all parser_*.c sub-modules.
 //
 // This header is NOT part of the public API. It provides:
 // 1. All #include directives needed by parser sub-modules.
 // 2. Forward declarations for cross-module parser functions.
 // 3. Shared helper functions used by multiple sub-modules.
 //
 // Only parser_*.c files should include this header. External
 // code should use parser.h instead.
 //
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // ---

#ifndef BASICPP_PARSER_INTERNAL_H
#define BASICPP_PARSER_INTERNAL_H

// POSIX feature test macros -- needed for popen/pclose/mkdir
 // when compiling with -std=c90 on Linux/POSIX systems. 
#if !defined(_WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
  #if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 200112L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200112L
  #endif
  #ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
  #endif
#endif

// Standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

// Platform-specific
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType WinTokenType // avoid winnt.h collision
#include <windows.h>
#undef TokenType
#include <direct.h> // _chdir, _getcwd, _mkdir
#include <conio.h> // _getch, _kbhit
#include <sys/stat.h>
#else
#include <unistd.h> // chdir, getcwd
#include <dirent.h> // opendir, readdir, closedir
#include <sys/stat.h>
#endif

// Project headers
#include "parser.h"
#include "exec.h"
#include "dialect.h"
#include "fileio.h"
#include "errors.h"
#include "compiler.h"
#include "vdev.h"
#include "funcreg.h"
#include "builtins.h"
#include "vm.h"
#include "bytecode.h"
#include "module.h"
#include "alias_lang.h"
#include "scope.h"
#include "keyword_props.h"
#include "override.h"
#include "security.h"
#include "platform.h"
#include "scope_stack.h"
#include "selftest.h"
#include "check.h"
#include "help.h"
#include "detok.h"
#include "gfxbuf.h"
#include "memmap.h"
#include "device_alias.h"
#include "mod_fujinet.h"

// -----------------------------------------------------------
 // Internal utility functions (defined in parser.c)
 // Used by multiple parser sub-modules.
 // ----------------------------------------------------------- 

 // prop_eq_ci - Case-insensitive property name comparison.
int pi_prop_eq_ci(const char *a, const char *b);

 // str_case_equal - Case-insensitive full-string comparison.
int pi_str_case_equal(const char *a, const char *b);

 // set_param_by_name - Set a named parameter variable.
 // Used by SUB/FUNCTION calls and DEF FN evaluation.
 //
 // If the name is a single letter A-Z, sets the corresponding
 // single-letter variable. Otherwise sets a named variable.
void pi_set_param_by_name(RuntimeState *rt,
    const char *name, BValue val);

 // print_margin_check - Auto-wrap at screen_width columns.
void pi_print_margin_check(RuntimeState *rt);

 // skip_to_pos - Advance lexer to a specific position.
void pi_skip_to_pos(Lexer *lex, int target_pos);

// -----------------------------------------------------------
 // Expression engine (defined in parser_expr.c)
 //
 // parse_expression() and parse_expression_bval() are public
 // (declared in parser.h). These internal variants handle
 // sub-expression parsing.
 // ----------------------------------------------------------- 

long pi_parse_factor(Lexer *lex, RuntimeState *rt,
    int line_num);
long pi_parse_power(Lexer *lex, RuntimeState *rt,
    int line_num);
long pi_parse_term(Lexer *lex, RuntimeState *rt,
    int line_num);

BValue pi_parse_factor_bval(Lexer *lex, RuntimeState *rt,
    int line_num);
BValue pi_parse_power_bval(Lexer *lex, RuntimeState *rt,
    int line_num);
BValue pi_parse_term_bval(Lexer *lex, RuntimeState *rt,
    int line_num);

// -----------------------------------------------------------
 // Statement handlers (defined in parser_*.c sub-modules)
 //
 // Each function handles a single BASIC statement type.
 // Called from parse_statement() in parser.c.
 // ----------------------------------------------------------- 

// parser_io.c - I/O statements
void pi_parse_print(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_input(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_data(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_read(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_restore(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_line_input(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_assign.c - Assignment & variable declaration
void pi_parse_let(Lexer *lex, RuntimeState *rt,
    int line_num, int is_implicit);
void pi_parse_dim(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_flow.c - Control flow
void pi_parse_if(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_block_if_skip_to_end(RuntimeState *rt,
    int line_num);
KeywordId pi_block_if_scan(RuntimeState *rt,
    int line_num);
int pi_when_exception_scan(RuntimeState *rt,
    int when_idx, int *use_idx,
    int *end_when_idx, int line_num);
void pi_parse_unless(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_goto(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_gosub(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_return(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_end(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_stop(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_rem(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_loops.c - Loop constructs
void pi_parse_for(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_next(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_while(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_wend(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_do(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_loop(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_fileio.c - File I/O
#include "io/vfs.h"
void pi_parse_open(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_close(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_set_file(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_ask_file(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_rewrite(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_erase_file(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_mount(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_umount(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_mounts(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vpath(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_deffn.c - User-defined functions
void pi_parse_def_fn(Lexer *lex, RuntimeState *rt,
    int line_num);
BValue pi_eval_user_fn(Lexer *outer_lex,
    RuntimeState *rt, int line_num);

// parser_mat.c - Matrix operations
void pi_parse_mat_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_cmds.c - System commands
void pi_parse_list_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_run_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_new_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_save_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_load_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_unload_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_merge_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_chain_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_dialect_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);
int pi_ensure_bas_ext(char *fname, int len, int maxlen);

// parser.c - Statement dispatcher (internal, called by
 // parser_execute_line) 
void pi_parse_statement(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_graphics.c - Graphics & drawing
void pi_parse_screen(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_console(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_color(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_draw(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_line(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_circle(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_paint(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_preset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_palette(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pcopy(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_view(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_window(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_sound.c - Sound & music
void pi_parse_beep(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_sound(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_play(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_display.c - Display & console attributes
void pi_parse_cls(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_home(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_locate(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_width(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_ink(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_paper(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_border(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_bright(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_flash(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_inverse(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_over(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_vars.c - Variables, constants & types
void pi_parse_const_stmt(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_swap(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_redim(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_shared(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_static(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_common(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_type(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_defint(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_defdbl(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_defsng(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_defstr(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vars(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_filemgmt.c - File management & navigation
void pi_parse_files(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_dir(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_kill(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_scratch(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_unsave(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_copy(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_move(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pwd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_chdir(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_mkdir(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_rmdir(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_name(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_rename(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_erase(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_blockio.c - Block/random I/O & records
void pi_parse_field(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_get(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_put(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_lset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_rset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_seek(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_lock(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_unlock(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_ioctl(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_streamio.c - Stream I/O & printer
void pi_parse_lprint(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_llist(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_write(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_display(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_type_cmd(Lexer *lex, RuntimeState *rt,
    int line_num);

// format_using.c - Formatted output engine
void format_using_process(FILE *fp, const char *fmt, int flen,
    Lexer *lex, RuntimeState *rt, int line_num);
int format_using_numeric(FILE *fp, const char *fmt, int flen,
    int *pos, double value);
int format_using_string_field(FILE *fp, const char *fmt, int flen,
    int *pos, const char *str, int slen);

// format_input.c - Formatted input validation engine
int input_read_protected(char *buf, int maxlen, const char *prompt);
int format_input_using(char *buf, int maxlen, const char *fmt,
    int flen, const char *prompt);

// parser_shell.c - Shell & external execution
void pi_parse_shell(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_exec(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_sys(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_system(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_environ(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_debug.c - Debugging & introspection
void pi_parse_tron(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_troff(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_break(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_cont(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_assert(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_test(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_endtest(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_selftest(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_check(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_verify(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_info(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_debug(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_dump(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_backtrace(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_trace(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_help.c - Help & catalog
void pi_parse_help(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_catalog(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_ver(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_virtual.c - Virtual infrastructure
void pi_parse_vdev(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vmem(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vnet(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vcon(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vterm(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vmach(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_devmap(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_progmgmt.c - Program editing & management
void pi_parse_renum(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_delete(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_edit(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_auto(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_compile(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_bsave(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_bload(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_struct.c - Structured programming
void pi_parse_select(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_case(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_exit(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_sub(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_function(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_declare(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_call(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_procedure(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_define(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_enddefine(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_local(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_errhand.c - Error handling & exceptions
void pi_parse_error(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_cause(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_resume(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_when(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_use(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_retry(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_continue(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_trap(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_config.c - Configuration & customization
void pi_parse_option(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_alias(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_scope(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_keyword(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_override(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_security(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_module(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_misc.c - Miscellaneous utilities
void pi_parse_randomize(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pause(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_delay(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_sleep(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_wait(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_clear(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_clr(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_reset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_poke(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_memmap(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_mid(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_key(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_repeat(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_endrepeat(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_endfor(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_out(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_com(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pen(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_strig(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_access(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_image(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_timer(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_play_event(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_flow.c - Control flow additions
void pi_parse_on(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_else(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_elseif(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_endif(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_txn.c - Transaction / ATOMIC control
void pi_parse_atomic(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_end_atomic(int line_num);
void pi_parse_txn(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_commit(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_rollback(Lexer *lex, RuntimeState *rt,
    int line_num);

#endif // BASICPP_PARSER_INTERNAL_H

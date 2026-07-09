/* =====================================================================
 * BASIC++ Interpreter — Lite Build Stubs
 * =====================================================================
 * Minimal stubs satisfying linker requirements for shared file channels
 * in parser_io.c/parser_expr.c when compiling without io/fileio.c.
 * ===================================================================== */

#ifdef BPP_LITE_BUILD
#include <stdio.h>
#include "fileio.h"
#include "errors.h"
#include "lexer.h"
#include "runtime.h"
#include "pcode.h"

int fileio_print(int chan, const char *text, int line_num) {
    (void)chan; (void)text; (void)line_num;
    return 0;
}
int fileio_print_newline(int chan, int line_num) {
    (void)chan; (void)line_num;
    return 0;
}
int fileio_input_line(int chan, char *buf, int max_len, int line_num) {
    (void)chan; (void)buf; (void)max_len; (void)line_num;
    return 0;
}
void fileio_channels_cleanup(void) {}
void fileio_channels_init(void) {}
FILE *fileio_get_fp(int chan) { (void)chan; return NULL; }
int fileio_chain(ProgramStore *store, const char *filename) { (void)store; (void)filename; return -1; }

// Stubs for parser commands / flow control not present in Lite build
#define PARSER_STUB(name) \
    void name(Lexer *lex, RuntimeState *rt, int line_num) { \
        (void)lex; (void)rt; \
        error_raise(ERR_HOW, line_num); \
    }

PARSER_STUB(pi_parse_troff)
PARSER_STUB(pi_parse_break)
PARSER_STUB(pi_parse_cont)
PARSER_STUB(pi_parse_assert)
PARSER_STUB(pi_parse_test)
PARSER_STUB(pi_parse_endtest)
PARSER_STUB(pi_parse_selftest)
PARSER_STUB(pi_parse_check)
PARSER_STUB(pi_parse_verify)
PARSER_STUB(pi_parse_info)
PARSER_STUB(pi_parse_debug)
PARSER_STUB(pi_parse_dump)
PARSER_STUB(pi_parse_backtrace)
PARSER_STUB(pi_parse_trace)
PARSER_STUB(pi_parse_catalog)
PARSER_STUB(pi_parse_vdev)
PARSER_STUB(pi_parse_vmem)
PARSER_STUB(pi_parse_vnet)
PARSER_STUB(pi_parse_vcon)
PARSER_STUB(pi_parse_vterm)
PARSER_STUB(pi_parse_vmach)

PARSER_STUB(pi_parse_reverse)
PARSER_STUB(pi_parse_drawto)
PARSER_STUB(pi_parse_at_stmt)
PARSER_STUB(pi_parse_plot)
PARSER_STUB(pi_parse_devmap)
PARSER_STUB(pi_parse_select)
PARSER_STUB(pi_parse_case)
PARSER_STUB(pi_parse_exit)
PARSER_STUB(pi_parse_sub)
PARSER_STUB(pi_parse_function)
PARSER_STUB(pi_parse_declare)
PARSER_STUB(pi_parse_call)
PARSER_STUB(pi_parse_procedure)
PARSER_STUB(pi_parse_define)
PARSER_STUB(pi_parse_enddefine)
PARSER_STUB(pi_parse_local)
PARSER_STUB(pi_parse_error)
PARSER_STUB(pi_parse_cause)
PARSER_STUB(pi_parse_resume)
PARSER_STUB(pi_parse_when)
PARSER_STUB(pi_parse_use)
PARSER_STUB(pi_parse_retry)
PARSER_STUB(pi_parse_continue)
PARSER_STUB(pi_parse_trap)
PARSER_STUB(pi_parse_option)
PARSER_STUB(pi_parse_alias)
PARSER_STUB(pi_parse_scope)
PARSER_STUB(pi_parse_keyword)
PARSER_STUB(pi_parse_override)
PARSER_STUB(pi_parse_security)
PARSER_STUB(pi_parse_module)
PARSER_STUB(pi_parse_reset)
PARSER_STUB(pi_parse_poke)
PARSER_STUB(pi_parse_atomic)
PARSER_STUB(pi_parse_txn)
PARSER_STUB(pi_parse_commit)
PARSER_STUB(pi_parse_rollback)
PARSER_STUB(pi_parse_pokeb)

PARSER_STUB(pi_parse_rewrite)
PARSER_STUB(pi_parse_mount)
PARSER_STUB(pi_parse_umount)
PARSER_STUB(pi_parse_mounts)
PARSER_STUB(pi_parse_vpath)

// Tape subsystem stubs
PARSER_STUB(pi_parse_csave)
PARSER_STUB(pi_parse_cload)
PARSER_STUB(pi_parse_crun)
PARSER_STUB(pi_parse_motor)
PARSER_STUB(pi_parse_mat_cmd)
PARSER_STUB(pi_parse_save_cmd)
PARSER_STUB(pi_parse_load_cmd)
PARSER_STUB(pi_parse_merge_cmd)
PARSER_STUB(pi_parse_chain_cmd)
PARSER_STUB(pi_parse_screen)
PARSER_STUB(pi_parse_graphics)
PARSER_STUB(pi_parse_console)
PARSER_STUB(pi_parse_color)
PARSER_STUB(pi_parse_draw)
PARSER_STUB(pi_parse_line)
PARSER_STUB(pi_parse_circle)
PARSER_STUB(pi_parse_paint)
PARSER_STUB(pi_parse_pset)
PARSER_STUB(pi_parse_preset)
PARSER_STUB(pi_parse_palette)
PARSER_STUB(pi_parse_pcopy)
PARSER_STUB(pi_parse_view)
PARSER_STUB(pi_parse_window)
PARSER_STUB(pi_parse_sound)
PARSER_STUB(pi_parse_play)
PARSER_STUB(pi_parse_cls)
PARSER_STUB(pi_parse_home)
PARSER_STUB(pi_parse_locate)
PARSER_STUB(pi_parse_width)
PARSER_STUB(pi_parse_ink)
PARSER_STUB(pi_parse_paper)
PARSER_STUB(pi_parse_border)
PARSER_STUB(pi_parse_bright)
PARSER_STUB(pi_parse_flash)
PARSER_STUB(pi_parse_inverse)
PARSER_STUB(pi_parse_over)
PARSER_STUB(pi_parse_files)
PARSER_STUB(pi_parse_dir)
PARSER_STUB(pi_parse_kill)
PARSER_STUB(pi_parse_scratch)
PARSER_STUB(pi_parse_unsave)
PARSER_STUB(pi_parse_copy)
PARSER_STUB(pi_parse_move)
PARSER_STUB(pi_parse_chdir)
PARSER_STUB(pi_parse_mkdir)
PARSER_STUB(pi_parse_rmdir)
PARSER_STUB(pi_parse_name)
PARSER_STUB(pi_parse_rename)
PARSER_STUB(pi_parse_erase)
PARSER_STUB(pi_parse_field)
PARSER_STUB(pi_parse_get)
PARSER_STUB(pi_parse_put)
PARSER_STUB(pi_parse_lset)
PARSER_STUB(pi_parse_rset)
PARSER_STUB(pi_parse_seek)
PARSER_STUB(pi_parse_lock)
PARSER_STUB(pi_parse_unlock)
PARSER_STUB(pi_parse_ioctl)
PARSER_STUB(pi_parse_lprint)
PARSER_STUB(pi_parse_llist)
PARSER_STUB(pi_parse_write)
PARSER_STUB(pi_parse_display)
PARSER_STUB(pi_parse_type_cmd)
PARSER_STUB(pi_parse_shell)
PARSER_STUB(pi_parse_exec)
PARSER_STUB(pi_parse_sys)
PARSER_STUB(pi_parse_system)
PARSER_STUB(pi_parse_environ)
PARSER_STUB(pi_parse_tron)
PARSER_STUB(pi_parse_memmap)

PARSER_STUB(pi_parse_open)
PARSER_STUB(pi_parse_close)
PARSER_STUB(pi_parse_set_file)
PARSER_STUB(pi_parse_ask_file)

void emulate_interrupt(RuntimeState *rt, int int_num, int line_num) {
    (void)rt; (void)int_num; (void)line_num;
}

int fileio_get_channel_mode(int chan) {
    (void)chan;
    return 0;
}

double gw_mbf32_to_double(float val) {
    return (double)val;
}

float gw_double_to_mbf32(double val) {
    return (float)val;
}

double gw_mbf64_to_double(double val) {
    return val;
}

double gw_double_to_mbf64(double val) {
    return val;
}

struct ParseNode;
int lib_space_try_call_func(const char *name, int name_len, void *lex_ptr, void *rt_ptr, int line_num, void *out_result) {
    (void)name; (void)name_len; (void)lex_ptr; (void)rt_ptr; (void)line_num; (void)out_result;
    return 0;
}


void pcode_free(PCodeProgram *pcode) {
    (void)pcode;
}

struct LoadedLibrary;
int bpl_load(const char *filename, struct LoadedLibrary *lib) {
    (void)filename; (void)lib;
    return -1;
}

int bpp_load(ProgramStore *prog, const char *filename, void *rt_ptr) {
    (void)prog; (void)filename; (void)rt_ptr;
    return -1;
}

int bpe_load(const char *filename, ProgramStore *prog, void *rt_ptr) {
    (void)filename; (void)prog; (void)rt_ptr;
    return -1;
}

int fileio_load(ProgramStore *store, const char *filename) {
    (void)store; (void)filename;
    return -1;
}

long bpe_find_embedded_offset(const char *exe_path) {
    (void)exe_path;
    return 0;
}

void pcode_cache_invalidate(RuntimeState *rt) {
    (void)rt;
}
int ast_interpret_stmt(RuntimeState *rt, void *stmt, int line_num) {
    (void)rt; (void)stmt; (void)line_num;
    return 0;
}
int ast_eval_expr_nonrec(RuntimeState *rt, void *root, int line_num, BValue *out_val) {
    (void)rt; (void)root; (void)line_num;
    if (out_val) *out_val = bval_int(0);
    return 0;
}
BValue ast_call_function(RuntimeState *rt, int func_id, BValue *args, int arg_count, char fn_letter, int builtin_kw, int line_num) {
    (void)rt; (void)func_id; (void)args; (void)arg_count; (void)fn_letter; (void)builtin_kw; (void)line_num;
    return bval_int(0);
}

PARSER_STUB(pi_parse_class)
PARSER_STUB(pi_parse_endclass)



int ext_lib_unload(const char *name) { (void)name; return -1; }
int ext_func_unload(const char *name) { (void)name; return -1; }
int ext_feature_unload(const char *name) { (void)name; return -1; }
int ext_plugin_unload(const char *name) { (void)name; return -1; }
int bpe_save(const char *filename, ProgramStore *prog) { (void)filename; (void)prog; return -1; }
int bpp_save(const char *filename, ProgramStore *prog) { (void)filename; (void)prog; return -1; }
int bpl_save(const char *filename, ProgramStore *prog) { (void)filename; (void)prog; return -1; }
int fileio_save(ProgramStore *store, const char *filename) { (void)store; (void)filename; return -1; }
int compiler_compile(const char *src, const char *dest) { (void)src; (void)dest; return -1; }
void *lib_space_find(const char *name) { (void)name; return NULL; }

#endif


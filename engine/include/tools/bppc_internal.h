// FILENAME: bppc_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bppc.c, bppc_bytecode.c, bppc_detok.c, bppc_main.c)
// NEEDED BY: libcore (bppc_transpile.c, bppc_expr.c, bppc_stmt.c, trans.c)
// NEEDS: libcore (ctype.h, ctype.c, string.h)
// NEEDS: libengine (string.c, version.c)
// NEEDS: libkernel (config.h, version.h)
// Implements toolchain and compiler subsystem components for bppc_internal.
//
// ---- Includes ----

#ifndef TOOLS_BPPC_INTERNAL_H
#define TOOLS_BPPC_INTERNAL_H

#include "runtime/ctype/ctype.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#include "types/config.h"
#include "types/version.h"

//
// ---- Shared Compiler Global State ----

typedef enum {
    TARGET_C17 = 0,
    TARGET_FREEDOS,
    TARGET_C89,
    TARGET_KNR,
    TARGET_FREESTANDING,
    TARGET_UEFI,
    TARGET_PASCAL
} BppcTargetDialect;

extern bool g_bppc_debug_mode;
extern BppcTargetDialect g_target_dialect;
extern bool g_no_bounds_check;

//
// ---- Shared Helper Functions ----

char *bppc_trim(char *str);
bool bppc_parse_line_number(const char *str, int *out_line, const char **out_text);
char *bppc_strdup(const char *src);
void bppc_write_le16(unsigned char *buf, uint16_t val);
char *bppc_read_file_to_string(const char *path);

void bppc_sanitize_ident(const char *in, char *out, size_t out_sz);
void bppc_convert_expression_to_c(const char *expr, char *out_c, size_t out_sz);
bool bppc_is_c_keyword_or_builtin(const char *name);
void bppc_emit_print(void *out, const char *args);
void bppc_emit_statement(void *out, const char *stmt, bool debug);

bool transpile_basic_to_c(const char *source, const char *c_outfile);
bool transpile_basic_to_c_opts(const char *source, const char *c_outfile, bool inline_runtime, bool optimize, bool debug);
bool transpile_files_to_c(const char **infiles, int num_files, const char *outfile, bool inline_runtime, bool optimize, bool debug);
char *detokenize_gw_basic(const unsigned char *data, size_t file_len);

//
// ---- Bytecode & Packaging Interfaces ----

bool compile_to_bpp(const char *source_text, const char *out_bpp_path);
bool package_standalone(const char *basic_path, const char *out_exe_path, const char *argv0, const char *stub_filename);
int bppc_main_entry(int argc, char **argv);

#endif // TOOLS_BPPC_INTERNAL_H

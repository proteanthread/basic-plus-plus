// FILENAME: ctype.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, libboot, libcore, libengine, libkernel, libstandard
// NEEDS: platform, memory
// Freestanding ASCII character classification and transformation.
//
// ---- Includes ----

// FILENAME: ctype.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (arrays_internal.h, bppc_internal.h, string.h)
// NEEDED BY: libcore (variables_internal.h)
// NEEDED BY: libcore (analyzer.c, ctype.c, error.c, feature_reg.c)
// NEEDED BY: libcore (float_parse.c, funcreg.c, map_serialize.c, metadata.c)
// NEEDED BY: libcore (micro_lib_metadata.c, num_parse.c, print_using.c)
// NEEDED BY: libcore (select.c, snprintf.c, spec.c, sscanf.c, strops.c)
// NEEDED BY: libcore (struct.c, using_engine.c, vdev_core.c)
// NEEDED BY: libengine (ast_internal.h, dispatch_internal.h)
// NEEDED BY: libengine (eval_expr_internal.h, eval_internal.h)
// NEEDED BY: libengine (events_internal.h, exec_control_internal.h)
// NEEDED BY: libengine (exec_internal.h, mat_internal.h, reformat_internal.h)
// NEEDED BY: libengine (sub_internal.h, vm_internal.h)
// NEEDED BY: libengine (append.c, array_ext.c, chain.c, change.c, context.c)
// NEEDED BY: libengine (control.c, data.c, def.c, defdbl.c, defint.c, deflng.c)
// NEEDED BY: libengine (defsng.c, defstr.c, do.c, exec_dispatch.c, exit_loop.c)
// NEEDED BY: libengine (files.c, input.c, input_file.c, lexer.c, load.c, map.c)
// NEEDED BY: libengine (mat_input.c, mat_read.c, merge.c, module.c, open.c)
// NEEDED BY: libengine (play.c, randomize.c, read.c, renum.c, scan_keyword.c)
// NEEDED BY: libengine (scan_number.c, scan_string.c, suspend.c, try.c)
// NEEDED BY: libengine (whenever.c)
// NEEDED BY: libkernel (gfx_primitives_internal.h)
// NEEDED BY: libkernel (gfx_audio.c, gfx_stmt_shapes.c, gfx_tui.c, security.c)
// NEEDED BY: libkernel (vcon.c, vdev.c, vprinter.c)
// NEEDED BY: libstandard (edit_internal.h, edlin_internal.h, vi_internal.h)
// NEEDED BY: libstandard (ws_internal.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: platform, memory
// Freestanding ASCII character classification and transformation.
//
// ---- Includes ----

#ifndef RUNTIME_CTYPE_H
#define RUNTIME_CTYPE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Character classification bitmask constants
#define RUNTIME_CTYPE_DIGIT  (1U << 0)
#define RUNTIME_CTYPE_UPPER  (1U << 1)
#define RUNTIME_CTYPE_LOWER  (1U << 2)
#define RUNTIME_CTYPE_SPACE  (1U << 3)
#define RUNTIME_CTYPE_PUNCT  (1U << 4)
#define RUNTIME_CTYPE_CNTRL  (1U << 5)
#define RUNTIME_CTYPE_XDIGIT (1U << 6)
#define RUNTIME_CTYPE_BLANK  (1U << 7)

// Composite masks
#define RUNTIME_CTYPE_ALPHA  (RUNTIME_CTYPE_UPPER | RUNTIME_CTYPE_LOWER)
#define RUNTIME_CTYPE_ALNUM  (RUNTIME_CTYPE_ALPHA | RUNTIME_CTYPE_DIGIT)
#define RUNTIME_CTYPE_GRAPH  (RUNTIME_CTYPE_ALNUM | RUNTIME_CTYPE_PUNCT)
#define RUNTIME_CTYPE_PRINT  (RUNTIME_CTYPE_GRAPH | RUNTIME_CTYPE_SPACE)

// 256-entry lookup tables
extern const uint8_t runtime_ctype_table[256];
extern const uint8_t runtime_tolower_table[256];
extern const uint8_t runtime_toupper_table[256];

// Inline fast character classification predicates
static inline bool runtime_isdigit(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_DIGIT) != 0) : false;
}

static inline bool runtime_isalpha(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_ALPHA) != 0) : false;
}

static inline bool runtime_isalnum(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_ALNUM) != 0) : false;
}

static inline bool runtime_isspace(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_SPACE) != 0) : false;
}

static inline bool runtime_isupper(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_UPPER) != 0) : false;
}

static inline bool runtime_islower(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_LOWER) != 0) : false;
}

static inline bool runtime_isprint(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_PRINT) != 0) : false;
}

static inline bool runtime_ispunct(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_PUNCT) != 0) : false;
}

static inline bool runtime_iscntrl(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_CNTRL) != 0) : false;
}

static inline bool runtime_isxdigit(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_XDIGIT) != 0) : false;
}

static inline bool runtime_isblank(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_BLANK) != 0) : false;
}

static inline bool runtime_isgraph(int c) {
    return (c >= 0 && c <= 255) ? ((runtime_ctype_table[c] & RUNTIME_CTYPE_GRAPH) != 0) : false;
}

static inline int runtime_toupper(int c) {
    return (c >= 0 && c <= 255) ? (int)runtime_toupper_table[c] : c;
}

static inline int runtime_tolower(int c) {
    return (c >= 0 && c <= 255) ? (int)runtime_tolower_table[c] : c;
}

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_CTYPE_H

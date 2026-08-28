// FILENAME: ast_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast.c, ast_create.c, ast_eval_expr.c, ast_eval_stmt.c)
// NEEDED BY: libengine (ast_parse_block.c, ast_parse_expr.c, ast_parse_stmt.c)
// NEEDED BY: libengine (ast_parser.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements AST parsing and evaluation structures for ast_internal.
//
// ---- Includes ----

#ifndef EVAL_AST_INTERNAL_H
#define EVAL_AST_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/ctype/ctype.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/conv/num_parse.h"
#include "runtime/conv/float_parse.h"
#include "runtime/format/snprintf.h"
#include "runtime/math/basic.h"
#include "runtime/math/trig.h"
#include "runtime/math/algebra.h"
#include "runtime/memory/alloc.h"
#include "hal/hal.h"

#include "bios/bios.h"
#include "device/bus.h"
#include "device/vdev.h"
#include "eval/ast.h"
#include "eval/eval.h"
#include "eval/eval_internal.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "runtime/arrays.h"
#include "runtime/file.h"
#include "runtime/num_format.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "types/types.h"
#include "vm/vm.h"

//
// ---- Internal Inline Helpers ----

static inline const char *skip_line_number_and_spaces(const char *s) {
    if (!s) return "";
    while (*s && (*s == ' ' || *s == '\t')) s++;
    while (*s && runtime_isdigit((unsigned char)*s)) s++;
    while (*s && (*s == ' ' || *s == '\t')) s++;
    return s;
}

static inline BppStringRef str_from_cstr(StringContext *sc, const char *s) {
    if (!s) s = "";
    return str_create(sc, s, runtime_strlen(s));
}

static inline bool is_tok_kw(BppToken tok, BppKeywordId kw, const char *name, size_t len) {
    if (tok.type == TOK_KEYWORD && tok.as.keyword == kw) return true;
    if (tok.type == TOK_IDENT && tok.length == len && runtime_strncasecmp(tok.start, name, len) == 0) return true;
    return false;
}

static inline double fast_int_pow(double base, int exp) {
    if (exp == 0) return 1.0;
    if (exp < 0) return 1.0 / fast_int_pow(base, -exp);
    double res = 1.0;
    double b = base;
    while (exp > 0) {
        if (exp & 1) res *= b;
        b *= b;
        exp >>= 1;
    }
    return res;
}

static inline size_t ast_format_num(char *buf, double val) {
    if ((double)(int64_t)val == val && val >= -9223372036854775807LL && val <= 9223372036854775807LL) {
        int64_t n = (int64_t)val;
        size_t pos = 0;
        if (n >= 0) {
            buf[pos++] = ' ';
        } else {
            buf[pos++] = '-';
            n = -n;
        }
        char temp[24];
        int tpos = 0;
        if (n == 0) {
            temp[tpos++] = '0';
        } else {
            while (n > 0) {
                temp[tpos++] = (char)('0' + (n % 10));
                n /= 10;
            }
        }
        while (tpos > 0) {
            buf[pos++] = temp[--tpos];
        }
        buf[pos++] = ' ';
        buf[pos] = '\0';
        return pos;
    } else {
        num_format_display(buf, 64, val, true, true);
        return runtime_strlen(buf);
    }
}


//
// ---- Prototypes Across AST Micro-Modules ----

void eval_ast_free_tree(void *node_ptr);
void eval_ast_flatten_concat(VMContext *vm, EvalAstNode *node, const char **parts, size_t *lens,
                             char num_bufs[16][64], size_t *num_buf_idx,
                             BppString **to_release, size_t *scount, size_t *rcount,
                             size_t max_parts, BppError *err);
double eval_ast_calc_binary_op(BppTokenType op, double n1, double n2);

EvalAstNode *eval_ast_compile_subroutine(VMContext *vm, const BppProgramLine *lines, size_t count, BppLineNumber target_line);
EvalAstNode *eval_ast_parse_expression(LexerContext *lex);
EvalAstNode *parse_single_statement(LexerContext *lex);
EvalAstNode *eval_ast_try_compile_multiline_if(VMContext *vm, const BppProgramLine *lines, size_t start_idx, size_t total_count, size_t *out_lines_skipped);

#endif // EVAL_AST_INTERNAL_H

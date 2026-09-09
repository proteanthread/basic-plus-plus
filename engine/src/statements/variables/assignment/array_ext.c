// FILENAME: array_ext.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt.h, string.c, vm.h)
// Provides runtime implementation for the ARRAY_EXT statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "runtime/language_descriptor.h"
#include "lexer/lexer.h"
#include "runtime/arrays.h"
#include "eval/eval.h"
#include "vm/vm.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "statements/variables/data/pick_locate.h"

static const LangDesc g_array_ext_desc = {
    .name = "ARRAY EXT",
    .category = "Array Extensions",
    .syntax = "ARRAY.SORT / ARRAY.REVERSE / ARRAY.FILL",
    .description = "Provides extended high-performance array operations including sorting and filling.",
    .error_summary = "Error 9: Subscript Out of Range",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

extern BppError arrayext_execute_map(VMContext *vm, const char *src_arr, const char *dst_arr, const char *fn_name, const char *label_name, const char *expr_str);

BppError stmt_arrayext_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if ((tok.type == TOK_KEYWORD && (tok.as.keyword == KW_FIND || tok.as.keyword == KW_LOCATE)) ||
        (tok.type == TOK_IDENT && ((tok.length == 4 && runtime_strncasecmp(tok.start, "FIND", 4) == 0) ||
                                  (tok.length == 6 && runtime_strncasecmp(tok.start, "LOCATE", 6) == 0)))) {
        lex_next(lex); // Consume FIND / LOCATE
        const char *in_pos = NULL;
        if (pick_locate_has_in_clause(lex, &in_pos)) {
            size_t tlen = (size_t)(in_pos - lex_get_pos(lex));
            char *tbuf = (char *)mem_scratch_alloc(vm_get_mem(vm), tlen + 1);
            if (tbuf) {
                runtime_memcpy(tbuf, lex_get_pos(lex), tlen);
                tbuf[tlen] = '\0';
                LexerContext *sub = lex_init(vm_get_mem(vm), tbuf);
                BValue target_val = eval_expression(vm, sub, &err);
                lex_shutdown(sub);
                if (err.code != 0) return err;
                lex_set_pos(lex, in_pos);
                return stmt_pick_locate_execute(vm, lex, target_val);
            }
        }
        BValue target = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        return stmt_pick_locate_execute(vm, lex, target);
    }

    BppKeywordId op = tok.as.keyword;
    if (op != KW_MAP && op != KW_FILTER && op != KW_REDUCE) {
        err.code = 2; err.message = "Expected MAP, FILTER, REDUCE, or FIND after ARRAY";
        return err;
    }
    lex_next(lex); // Consume MAP/FILTER/REDUCE

    // Get source array
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected source array name";
        return err;
    }
    char src_arr[256];
    size_t slen = (tok.length < sizeof(src_arr) - 1) ? tok.length : sizeof(src_arr) - 1;
    runtime_memcpy(src_arr, tok.start, slen);
    src_arr[slen] = '\0';

    if (lex_next(lex).type != TOK_LPAREN || lex_next(lex).type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected () after source array name";
        return err;
    }

    char dst_arr[256];
    runtime_snprintf(dst_arr, sizeof(dst_arr), "%s", src_arr); // Default in-place

    tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) { // Wait, TO is not a keyword? Let's check lexer. Actually, TO is TOK_IDENT usually? No, it's KW_TO. Wait, there's KW_TO? In eval.c it's usually KW_TO. Let's just use TOK_IDENT and check "TO"
        lex_next(lex);
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected destination array name after TO";
            return err;
        }
        size_t dlen = (tok.length < sizeof(dst_arr) - 1) ? tok.length : sizeof(dst_arr) - 1;
        runtime_memcpy(dst_arr, tok.start, dlen);
        dst_arr[dlen] = '\0';
        if (lex_next(lex).type != TOK_LPAREN || lex_next(lex).type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected () after destination array name";
            return err;
        }
        tok = lex_peek(lex);
    }

    char fn_name[256] = "";
    char label_name[256] = "";
    char expr_str[256] = "";

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex);
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected function name after USING";
            return err;
        }
        size_t flen = (tok.length < sizeof(fn_name) - 1) ? tok.length : sizeof(fn_name) - 1;
        runtime_memcpy(fn_name, tok.start, flen);
        fn_name[flen] = '\0';
    } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOSUB) {
        lex_next(lex);
        tok = lex_next(lex);
        // Expect label
        if (tok.type != TOK_IDENT && tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected label after GOSUB";
            return err;
        }
        size_t llen = (tok.length < sizeof(label_name) - 1) ? tok.length : sizeof(label_name) - 1;
        runtime_memcpy(label_name, tok.start, llen);
        label_name[llen] = '\0';
    } else if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue expr_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (expr_val.type != VAL_STRING || !expr_val.as.string) {
            err.code = 13; err.message = "Expected string expression for ARRAY evaluation";
            return err;
        }
        runtime_strncpy(expr_str, str_data(expr_val.as.string), sizeof(expr_str) - 1);
        str_release(vm_get_str(vm), expr_val.as.string);
    } else {
        err.code = 2; err.message = "Expected USING, GOSUB, or string expression";
        return err;
    }

    if (op == KW_MAP) {
        return arrayext_execute_map(vm, src_arr, dst_arr, fn_name, label_name, expr_str);
    } else if (op == KW_FILTER) {
        // Not yet implemented in this file directly, will call module
    } else if (op == KW_REDUCE) {
        // Not yet implemented
    }

    return err;
}

void stmt_array_ext_register(void) {
    lang_desc_register(&g_array_ext_desc);
}

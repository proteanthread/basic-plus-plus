/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file lset.c
 * @brief LSET / RSET string buffer left/right alignment assignment statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements LSET string_var = string_expr and RSET string_var = string_expr statement handlers for left-aligned or right-aligned string field packing.
 *
 * 2. WHY IT EXISTS:
 * Formats string variable fields in-place (especially FIELD record buffers for random-access disk file I/O) without changing target buffer length.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Evaluates source string expression via eval_expression(). Pads with spaces or truncates source string to match exact length of target string variable. LSET left-aligns (padding on right), RSET right-aligns (padding on left).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_lset'. Includes "stmt/stmt.h",
 * "eval/eval.h", "runtime/map.h", "runtime/strings.h", "statements/filesystem/field.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support custom fill characters (LSET string_var = expr WITH fill_char$) if expanding string alignment tools.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Target buffer length invariant: Target string variable length MUST remain strictly unchanged after LSET/RSET assignment.
 *
 * 8. WHAT TO EXPECT:
 * Overwrites target string buffer in-place with padded/truncated text and returns ERR_NONE.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect target variable reference lookup and string release calls via str_release().
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Target variable MUST be a string variable (VAL_STRING).
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Proper string reference counting per Rule #1.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * - engine/src/eval/eval.c
 * Prerequisite Header Files:
 * - engine/include/statements/filesystem/field.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "runtime/map.h"
#include "runtime/strings.h"
#include "runtime/file.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static BppError stmt_xset_handler(VMContext *vm, LexerContext *lex, bool is_rset) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; /* Syntax error */
        err.message = "Expected variable name in LSET/RSET assignment";
        return err;
    }

    char var_name[256];
    size_t clen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
    memcpy(var_name, tok.start, clen);
    var_name[clen] = '\0';
    lex_next(lex); /* Consume identifier */

    /* Must be followed by '=' */
    tok = lex_peek(lex);
    if (tok.type != TOK_EQ) {
        err.code = 2;
        err.message = "Expected '=' after variable name in LSET/RSET";
        return err;
    }
    lex_next(lex); /* Consume '=' */

    /* Evaluate expression */
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type != VAL_STRING) {
        if (val.type == VAL_MAP) map_release(vm_get_str(vm), val.as.map);
        err.code = 13; /* Type mismatch */
        err.message = "LSET/RSET requires a string expression";
        return err;
    }

    /* Look up existing variable */
    VariableContext *var_ctx = vm_get_var(vm);
    BValue *var = var_lookup(var_ctx, var_name, true);
    if (!var) {
        if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
        err.code = 13; /* Type mismatch */
        err.message = "LSET/RSET target must be a valid variable";
        return err;
    }

    if (var->type != VAL_STRING && var->type != VAL_FIELD_STRING) {
        if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
        err.code = 13; /* Type mismatch */
        err.message = "LSET/RSET target must be a string variable or FIELD string";
        return err;
    }

    /* Target length */
    size_t target_len = 0;
    if (var->type == VAL_FIELD_STRING) {
        target_len = var->as.field_str.length;
    } else if (var->as.string) {
        target_len = str_len(var->as.string);
    }

    /* Padding logic */
    const char *src_str = val.as.string ? str_data(val.as.string) : "";
    size_t src_len = val.as.string ? str_len(val.as.string) : 0;

    char *padded = (char*)calloc(1, target_len + 1);
    if (!padded) {
        if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
        err.code = 7; /* Out of memory */
        err.message = "Out of memory in LSET/RSET";
        return err;
    }
    
    memset(padded, ' ', target_len);
    padded[target_len] = '\0';

    if (target_len > 0) {
        if (is_rset) {
            /* Right justify: copy to end of buffer */
            size_t copy_len = (src_len < target_len) ? src_len : target_len;
            size_t offset = target_len - copy_len;
            memcpy(padded + offset, src_str, copy_len);
        } else {
            /* Left justify: copy to beginning of buffer */
            size_t copy_len = (src_len < target_len) ? src_len : target_len;
            memcpy(padded, src_str, copy_len);
        }
    }

    /* Assign back to variable or buffer */
    if (var->type == VAL_FIELD_STRING) {
        int ch = var->as.field_str.channel;
        unsigned char *rec_buf = file_get_record_buffer(vm_get_file(vm), ch);
        if (rec_buf) {
            memcpy(rec_buf + var->as.field_str.offset, padded, target_len);
        }
        free(padded);
    } else {
        BValue new_val;
        new_val.type = VAL_STRING;
        new_val.as.string = str_create(vm_get_str(vm), padded, target_len);
        free(padded);

        if (var->as.string) {
            str_release(vm_get_str(vm), var->as.string);
        }
        var->as.string = new_val.as.string;
    }

    /* Release expression value */
    if (val.type == VAL_STRING && val.as.string) {
        str_release(vm_get_str(vm), val.as.string);
    }

    return err;
}

BppError stmt_lset_handler(VMContext *vm, LexerContext *lex) {
    return stmt_xset_handler(vm, lex, false);
}

BppError stmt_rset_handler(VMContext *vm, LexerContext *lex) {
    return stmt_xset_handler(vm, lex, true);
}

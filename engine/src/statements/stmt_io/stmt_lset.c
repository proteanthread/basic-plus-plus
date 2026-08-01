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
        if (val.type == VAL_MAP) bpp_map_release(vm_get_str(vm), val.as.map);
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

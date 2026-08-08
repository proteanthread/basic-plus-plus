/**
 * @file lset.c
 * @brief LSET string_var = expr$ and RSET string_var = expr$ string alignment statement handlers for BASIC++.
 */
#include "statements/variables/lset.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_lset_register(void) {
    static const MicroLibMetadata meta = {
        .name = "LSET",
        .category = "Variables & Memory",
        .syntax = "LSET string_var = string_expression | RSET string_var = string_expression",
        .help_text = "Left or right justifies a string in a fixed-length string variable or FIELD buffer.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_lset_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2;
        err.message = "Syntax Error in LSET";
        return err;
    }

    char var_name[64];
    if (tok.length >= sizeof(var_name)) tok.length = sizeof(var_name) - 1;
    memcpy(var_name, tok.start, tok.length);
    var_name[tok.length] = '\0';

    tok = lex_next(lex);
    if (tok.type != TOK_EQ) {
        err.code = 2;
        err.message = "Syntax Error in LSET (expected =)";
        return err;
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type != VAL_STRING || !val.as.string) {
        err.code = 13;
        err.message = "Type Mismatch in LSET";
        return err;
    }

    VariableContext *vc = vm_get_var(vm);
    StringContext *str_ctx = vm_get_str(vm);
    BValue *target_val = var_lookup(vc, var_name, true);
    BppString *target = (target_val && target_val->type == VAL_STRING) ? target_val->as.string : NULL;

    size_t target_len = target ? str_len(target) : 16;
    size_t src_len = str_len(val.as.string);
    const char *src = str_data(val.as.string);

    char buf[1024];
    if (target_len >= sizeof(buf)) target_len = sizeof(buf) - 1;

    memset(buf, ' ', target_len);
    if (src_len > target_len) src_len = target_len;
    memcpy(buf, src, src_len);
    buf[target_len] = '\0';

    BppString *res = str_create(str_ctx, buf, target_len);
    BValue new_val;
    memset(&new_val, 0, sizeof(new_val));
    new_val.type = VAL_STRING;
    new_val.as.string = res;
    var_assign(vc, var_name, new_val);
    str_release(str_ctx, res);
    str_release(str_ctx, val.as.string);

    return err;
}

BppError stmt_rset_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2;
        err.message = "Syntax Error in RSET";
        return err;
    }

    char var_name[64];
    if (tok.length >= sizeof(var_name)) tok.length = sizeof(var_name) - 1;
    memcpy(var_name, tok.start, tok.length);
    var_name[tok.length] = '\0';

    tok = lex_next(lex);
    if (tok.type != TOK_EQ) {
        err.code = 2;
        err.message = "Syntax Error in RSET (expected =)";
        return err;
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type != VAL_STRING || !val.as.string) {
        err.code = 13;
        err.message = "Type Mismatch in RSET";
        return err;
    }

    VariableContext *vc = vm_get_var(vm);
    StringContext *str_ctx = vm_get_str(vm);
    BValue *target_val = var_lookup(vc, var_name, true);
    BppString *target = (target_val && target_val->type == VAL_STRING) ? target_val->as.string : NULL;

    size_t target_len = target ? str_len(target) : 16;
    size_t src_len = str_len(val.as.string);
    const char *src = str_data(val.as.string);

    char buf[1024];
    if (target_len >= sizeof(buf)) target_len = sizeof(buf) - 1;

    memset(buf, ' ', target_len);
    if (src_len > target_len) {
        memcpy(buf, src + (src_len - target_len), target_len);
    } else {
        memcpy(buf + (target_len - src_len), src, src_len);
    }
    buf[target_len] = '\0';

    BppString *res = str_create(str_ctx, buf, target_len);
    BValue new_val;
    memset(&new_val, 0, sizeof(new_val));
    new_val.type = VAL_STRING;
    new_val.as.string = res;
    var_assign(vc, var_name, new_val);
    str_release(str_ctx, res);
    str_release(str_ctx, val.as.string);

    return err;
}

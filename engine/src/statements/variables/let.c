/**
 * @file let.c
 * @brief LET and SWAP variable assignment statement handlers for BASIC++.
 */
#include "statements/variables/let.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_let_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SWAP",
        .category = "Variables & Memory",
        .syntax = "SWAP variable1, variable2",
        .help_text = "Exchanges the values of two variables or array elements of identical types.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_let_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_LET) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type != TOK_IDENT) {
        err.code = 2; /* Syntax Error */
        err.message = "Syntax Error in LET (expected variable name)";
        return err;
    }

    tok = lex_next(lex);
    char var_name[64];
    size_t len = (tok.length < sizeof(var_name) - 1) ? tok.length : (sizeof(var_name) - 1);
    memcpy(var_name, tok.start, len);
    var_name[len] = '\0';

    BppToken eq = lex_next(lex);
    if (eq.type != TOK_EQ) {
        err.code = 2; /* Syntax Error */
        err.message = "Syntax Error in LET (expected '=')";
        return err;
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    VariableContext *vc = vm_get_var(vm);
    if (!var_assign(vc, var_name, val)) {
        err.code = 13; /* Type mismatch */
        err.message = "Type Mismatch in variable assignment";
    }

    if (val.type == VAL_STRING && val.as.string) {
        str_release(vm_get_str(vm), val.as.string);
    }

    return err;
}

BppError stmt_swap_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok1 = lex_next(lex);
    if (tok1.type != TOK_IDENT) {
        err.code = 2;
        err.message = "Syntax Error in SWAP (expected variable)";
        return err;
    }
    char var1[64];
    if (tok1.length >= sizeof(var1)) tok1.length = sizeof(var1) - 1;
    memcpy(var1, tok1.start, tok1.length);
    var1[tok1.length] = '\0';

    BppToken comma = lex_next(lex);
    if (comma.type != TOK_COMMA) {
        err.code = 2;
        err.message = "Syntax Error in SWAP (expected comma)";
        return err;
    }

    BppToken tok2 = lex_next(lex);
    if (tok2.type != TOK_IDENT) {
        err.code = 2;
        err.message = "Syntax Error in SWAP (expected second variable)";
        return err;
    }
    char var2[64];
    if (tok2.length >= sizeof(var2)) tok2.length = sizeof(var2) - 1;
    memcpy(var2, tok2.start, tok2.length);
    var2[tok2.length] = '\0';

    VariableContext *vc = vm_get_var(vm);
    bool str1 = (var1[strlen(var1) - 1] == '$');
    bool str2 = (var2[strlen(var2) - 1] == '$');

    if (str1 != str2) {
        err.code = 13;
        err.message = "Type Mismatch in SWAP";
        return err;
    }

    BValue *v1_ptr = var_lookup(vc, var1, true);
    BValue *v2_ptr = var_lookup(vc, var2, true);
    if (v1_ptr && v2_ptr) {
        BValue tmp = *v1_ptr;
        *v1_ptr = *v2_ptr;
        *v2_ptr = tmp;
    }

    return err;
}

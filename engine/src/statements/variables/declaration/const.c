// FILENAME: const.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (const.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the CONST statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/const.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_const_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CONST",
        .category = "Variables & Memory",
        .syntax = "CONST constantname = expression [, constantname = expression...]",
        .help_text = "Declares one or more symbolic constants assigned to literal or constant expressions.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_const_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2;
            err.message = "Expected identifier in CONST statement";
            return err;
        }

        char const_name[64];
        size_t len = (tok.length < sizeof(const_name) - 1) ? tok.length : (sizeof(const_name) - 1);
        memcpy(const_name, tok.start, len);
        const_name[len] = '\0';

        BppToken eq_tok = lex_next(lex);
        if (eq_tok.type != TOK_EQ) {
            err.code = 2;
            err.message = "Expected '=' in CONST statement";
            return err;
        }

        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            return err;
        }

        VariableContext *var = vm_get_var(vm);
        var_assign(var, const_name, val);

        // Release temporary string ownership if string
        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }

        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
        } else {
            break;
        }
    }

    return err;
}

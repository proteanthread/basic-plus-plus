// FILENAME: common.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (common.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the COMMON statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/common.h"
#include "runtime/variables.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>

void stmt_common_register(void) {
    MicroLibMetadata meta = {
        .name = "COMMON",
        .category = "Variables & Memory",
        .syntax = "COMMON [SHARED] var1 [()] [, var2 [()]...]",
        .help_text = "Declares global variables to be preserved across CHAIN and module boundaries.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_common_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    while (1) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }

        if (tok.type == TOK_IDENT) {
            tok = lex_next(lex);
            char var_name[64];
            size_t copy_len = tok.length < sizeof(var_name) - 1 ? tok.length : sizeof(var_name) - 1;
            memcpy(var_name, tok.start, copy_len);
            var_name[copy_len] = '\0';

            // Check if array parentheses follow e.g. A()
            BppToken next_tok = lex_peek(lex);
            if (next_tok.type == TOK_LPAREN) {
                lex_next(lex);
                next_tok = lex_peek(lex);
                if (next_tok.type == TOK_RPAREN) {
                    lex_next(lex);
                }
            }

            var_mark_common(vm_get_var(vm), var_name);
        } else {
            lex_next(lex);
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

// FILENAME: share.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (lexer.h, lexer.c, share.h, shared.h, shared.c, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the SHARE statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/share.h"
#include "statements/oop/shared.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

#include "runtime/variables.h"

BppError stmt_share_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    VariableContext *var = vm_get_var(vm);

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BppToken tok = lex_next(lex);
        if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_WITH || tok.as.keyword == KW_TO || tok.as.keyword == KW_FOR)) {
            while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
                lex_next(lex);
            }
            break;
        }

        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected variable name in SHARE statement";
            return err;
        }

        char name[64] = {0};
        size_t len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, len);

        var_set_shared(var, name);

        BppToken next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex);
        } else if (next.type == TOK_KEYWORD && (next.as.keyword == KW_WITH || next.as.keyword == KW_TO || next.as.keyword == KW_FOR)) {
            lex_next(lex); // Consume WITH / TO / FOR
            while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
                lex_next(lex);
            }
            break;
        } else {
            break;
        }
    }

    return err;
}

void stmt_share_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SHARE",
        .category = "Variables & Memory",
        .syntax = "SHARE variable [, variable...]",
        .help_text = "ECMA-116 standard statement to share variables between module routines and subprograms.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

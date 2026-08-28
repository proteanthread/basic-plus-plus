// FILENAME: public.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (lexer.h, lexer.c, public.h, string.c, vm.h)
// Provides runtime implementation for the PUBLIC statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/public.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/variables.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

extern BppError stmt_sub_handler(VMContext *vm, LexerContext *lex);

BppError stmt_public_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    VariableContext *var = vm_get_var(vm);

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BppToken tok = lex_next(lex);

        if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_TO || tok.as.keyword == KW_FOR || tok.as.keyword == KW_WITH)) {
            while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
                lex_next(lex);
            }
            break;
        }

        // If defining a routine: PUBLIC SUB foo / PUBLIC FUNCTION foo, skip routine body during execution
        if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION)) {
            return stmt_sub_handler(vm, lex);
        }

        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected identifier or routine name after PUBLIC";
            return err;
        }

        char name[64] = {0};
        size_t len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, len);

        var_set_shared(var, name);

        BppToken next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex);
        } else if (next.type == TOK_KEYWORD && (next.as.keyword == KW_TO || next.as.keyword == KW_FOR || next.as.keyword == KW_WITH)) {
            lex_next(lex); // Consume TO / FOR / WITH
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

BppError stmt_private_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BppToken tok = lex_next(lex);

        if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION)) {
            return stmt_sub_handler(vm, lex);
        }

        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected identifier or routine name after PRIVATE";
            return err;
        }

        BppToken next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

void stmt_public_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PUBLIC",
        .category = "Variables & Memory",
        .syntax = "PUBLIC [SUB|FUNCTION] name [, name...]",
        .help_text = "Exports specified variables or procedures from the current module scope to global scope.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

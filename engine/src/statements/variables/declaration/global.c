// FILENAME: global.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (global.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the GLOBAL statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/global.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/variables.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_global_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    VariableContext *var = vm_get_var(vm);

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BppToken tok = lex_next(lex);

        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected variable name after GLOBAL";
            return err;
        }

        char name[64] = {0};
        size_t len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, len);

        // Set variable as globally shared across root and procedures
        var_set_shared(var, name);

        // Check for optional AS <type>
        BppToken next = lex_peek(lex);
        if ((next.type == TOK_KEYWORD && next.as.keyword == KW_AS) ||
            (next.type == TOK_IDENT && next.length == 2 && (next.start[0] == 'A' || next.start[0] == 'a') && (next.start[1] == 'S' || next.start[1] == 's'))) {
            lex_next(lex); // Consume AS
            if (lex_peek(lex).type == TOK_IDENT || lex_peek(lex).type == TOK_KEYWORD) {
                lex_next(lex); // Consume type
            }
        }

        next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

void stmt_global_register(void) {
    static const MicroLibMetadata meta = {
        .name = "GLOBAL",
        .category = "Variables & Memory",
        .syntax = "GLOBAL variable [AS type] [, variable...]",
        .help_text = "Declares global root-level variables accessible everywhere, or binds procedure variables to global scope.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

// FILENAME: external.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (external.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the EXTERNAL statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/external/external.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_external_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || (tok.as.keyword != KW_SUB && tok.as.keyword != KW_FUNCTION)) {
        err.code = 2; err.message = "Expected SUB or FUNCTION after EXTERNAL";
        return err;
    }

    BppToken name_tok = lex_next(lex);
    if (name_tok.type != TOK_IDENT && name_tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected procedure name in EXTERNAL statement";
        return err;
    }

    // Consume parameter list if present
    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex);
        while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_RPAREN) {
            lex_next(lex);
        }
        if (lex_peek(lex).type == TOK_RPAREN) {
            lex_next(lex);
        }
    }

    return err;
}

void stmt_external_register(void) {
    static const MicroLibMetadata meta = {
        .name = "EXTERNAL",
        .category = "Control Flow",
        .syntax = "EXTERNAL SUB|FUNCTION name [(parameter_list)]",
        .help_text = "Declares an external SUB or FUNCTION procedure in ECMA-116 standard BASIC.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

// FILENAME: param.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (lexer.h, lexer.c, param.h, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the PARAM statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/structure/param.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/variables.h"
#include "types/errors.h"
#include <string.h>

#if defined(_WIN32)
#define strncasecmp _strnicmp
#endif

void stmt_param_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PARAM",
        .category = "Procedures / OOP",
        .syntax = "PARAM var1 [: type] [, var2 [: type] ...]",
        .help_text = "Declares and binds formal parameter variables inside a BASIC09 PROCEDURE block.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_param_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    BppError err;
    memset(&err, 0, sizeof(err));

    while (1) {
        BppToken tok = lex_next(lex);
        if (tok.type == TOK_EOF || tok.type == TOK_EOL || tok.type == TOK_BACKSLASH) {
            break;
        }

        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = ERR_SYNTAX;
            err.message = "Expected parameter variable name in PARAM";
            return err;
        }

        // Check for optional type specification (e.g. : INTEGER or AS INTEGER)
        BppToken next_tok = lex_peek(lex);
        if ((next_tok.start && next_tok.start[0] == ':') ||
            (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_AS) ||
            (next_tok.type == TOK_IDENT && next_tok.length == 2 && strncasecmp(next_tok.start, "AS", 2) == 0)) {
            lex_next(lex); // Consume ':' or 'AS'
            BppToken type_tok = lex_peek(lex);
            if (type_tok.type == TOK_IDENT || type_tok.type == TOK_KEYWORD) {
                lex_next(lex); // Consume type name
            }
        }

        // Check for optional array or string length e.g. STRING[30]
        if (lex_peek(lex).type == TOK_LBRACKET) {
            lex_next(lex);
            while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
                lex_next(lex);
            }
            if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
        }

        // Check for comma separating parameters
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
        } else {
            break;
        }
    }

    return err;
}

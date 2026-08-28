// FILENAME: declare.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (declare.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the DECLARE statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/external/declare.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_declare_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || (tok.as.keyword != KW_SUB && tok.as.keyword != KW_FUNCTION)) {
        err.code = 2; err.message = "Expected SUB or FUNCTION after DECLARE";
        return err;
    }

    BppToken name_tok = lex_next(lex);
    if (name_tok.type != TOK_IDENT && name_tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected procedure name in DECLARE statement";
        return err;
    }

    // Check for CDECL or PASCAL modifiers
    BppToken next_tok = lex_peek(lex);
    if (next_tok.type == TOK_IDENT) {
        if ((next_tok.length == 5 && _strnicmp(next_tok.start, "CDECL", 5) == 0) ||
            (next_tok.length == 6 && _strnicmp(next_tok.start, "PASCAL", 6) == 0)) {
            lex_next(lex);
            next_tok = lex_peek(lex);
        }
    }

    // Check for ALIAS "symbol_name"
    if ((next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_ALIAS) ||
        (next_tok.type == TOK_IDENT && next_tok.length == 5 && _strnicmp(next_tok.start, "ALIAS", 5) == 0)) {
        lex_next(lex); // Consume ALIAS
        BppToken alias_str = lex_next(lex);
        if (alias_str.type != TOK_STRING) {
            err.code = 2; err.message = "Expected alias string after ALIAS in DECLARE";
            return err;
        }
        next_tok = lex_peek(lex);
    }

    // Consume parameter list if present (e.g. (BYVAL A AS INTEGER, SEG B AS STRING))
    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex); // Consume '('
        while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_RPAREN) {
            lex_next(lex);
        }
        if (lex_peek(lex).type == TOK_RPAREN) {
            lex_next(lex); // Consume ')'
        }
    }

    // Check for trailing AS type (e.g. AS INTEGER)
    next_tok = lex_peek(lex);
    if ((next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_AS) ||
        (next_tok.type == TOK_IDENT && next_tok.length == 2 && _strnicmp(next_tok.start, "AS", 2) == 0)) {
        lex_next(lex); // Consume AS
        if (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
            lex_next(lex); // Consume type identifier/keyword
        }
    }

    return err;
}

void stmt_declare_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DECLARE",
        .category = "Control Flow",
        .syntax = "DECLARE SUB|FUNCTION name [CDECL|PASCAL] [ALIAS \"sym\"] [( [BYVAL|SEG] param [AS type], ... )] [AS type]",
        .help_text = "Declares a SUB or FUNCTION procedure prototype in QuickBASIC and ECMA-116 standard BASIC.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

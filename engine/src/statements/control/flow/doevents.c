// FILENAME: doevents.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h)
// NEEDS: libengine (doevents.h, eval.h, eval.c, lexer.h, lexer.c, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the DOEVENTS statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/doevents.h"
#include "platform/platform.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include <string.h>

BppError stmt_doevents_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5;
        err.message = "Invalid context for DOEVENTS";
        return err;
    }

    // Optional empty parentheses: DOEVENTS()
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex); // consume '('
        tok = lex_peek(lex);
        if (tok.type == TOK_RPAREN) {
            lex_next(lex); // consume ')'
        }
    }

    // Yield timeslice to OS to process event queue
    platform_sleep_ms(0);

    return err;
}

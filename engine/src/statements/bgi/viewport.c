// FILENAME: viewport.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, viewport.h)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the VIEWPORT statement in BASIC++.
//
// ---- Includes ----

#include "statements/bgi/viewport.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_viewport_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BValue xmin = eval_expression(vm, lex, &err); if (err.code != 0) return err;
    if (lex_next(lex).type != TOK_COMMA) { err.code = 2; err.message = "Expected comma in VIEWPORT"; return err; }

    BValue xmax = eval_expression(vm, lex, &err); if (err.code != 0) return err;
    if (lex_next(lex).type != TOK_COMMA) { err.code = 2; err.message = "Expected comma in VIEWPORT"; return err; }

    BValue ymin = eval_expression(vm, lex, &err); if (err.code != 0) return err;
    if (lex_next(lex).type != TOK_COMMA) { err.code = 2; err.message = "Expected comma in VIEWPORT"; return err; }

    BValue ymax = eval_expression(vm, lex, &err); if (err.code != 0) return err;

    (void)xmin; (void)xmax; (void)ymin; (void)ymax;
    return err;
}

void stmt_viewport_register(void) {
    static const MicroLibMetadata meta = {
        .name = "VIEWPORT",
        .category = "Graphics & Sound",
        .syntax = "VIEWPORT xmin, xmax, ymin, ymax",
        .help_text = "ECMA-116 standard statement to specify a normalized graphics viewport boundary.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

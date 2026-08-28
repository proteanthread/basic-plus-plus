// FILENAME: cause.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (cause.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the CAUSE statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/branch/cause.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_cause_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ERROR) ||
        (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "ERROR", 5) == 0)) {
        lex_next(lex);
    }

    BValue code_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (code_val.type != VAL_NUMBER) {
        err.code = 2; err.message = "Expected numeric error code after CAUSE ERROR";
        return err;
    }

    int code = (int)code_val.as.number;
    err.code = code;
    err.message = "User Caused Error";
    return err;
}

void stmt_cause_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CAUSE",
        .category = "Control Flow",
        .syntax = "CAUSE ERROR error_code",
        .help_text = "ECMA-116 standard statement to raise a runtime error.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

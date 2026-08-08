/**
 * @file on_error.c
 * @brief ON ERROR GOTO handler setup statement handler for BASIC++.
 */
#include "statements/event/on_error.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_on_error_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ON ERROR",
        .category = "Event Trapping",
        .syntax = "ON ERROR GOTO {line_label | 0}",
        .help_text = "Enables error-trapping routine and specifies the first statement of the handler.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_on_error_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOTO) {
        lex_next(lex);
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_NUMBER) {
            double target_line = val.as.number;
            vm_set_error_trap(vm, target_line);
        }
    } else {
        err.code = 2;
        err.message = "Syntax error in ON ERROR (expected GOTO)";
    }

    return err;
}

BppError stmt_onerr_handler(VMContext *vm, LexerContext *lex) {
    return stmt_on_error_handler(vm, lex);
}

BppError stmt_error_statement_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_NUMBER) {
        int code = (int)val.as.number;
        vm_set_error(vm, code, "User Error");
    }

    return err;
}

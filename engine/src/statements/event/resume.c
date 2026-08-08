/**
 * @file resume.c
 * @brief RESUME error recovery statement handler for BASIC++.
 */

#include "statements/event/resume.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_resume_register(void) {
    static const MicroLibMetadata meta = {
        .name = "RESUME",
        .category = "Event Trapping",
        .syntax = "RESUME [0 | NEXT | line_label]",
        .help_text = "Resumes program execution after an error-handling routine finishes.",
        .error_codes = "Error 20: RESUME Without Error, Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_resume_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm_is_in_error_handler(vm)) {
        err.code = 20;
        err.message = "RESUME Without Error";
        return err;
    }

    vm_set_in_error_handler(vm, false);

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        BppLineNumber fault_line = vm_get_error_occurred_line(vm);
        vm_jump(vm, fault_line, NULL);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_NEXT) {
        lex_next(lex);
        BppLineNumber fault_line = vm_get_error_occurred_line(vm);
        vm_jump(vm, fault_line + 1, NULL);
        return err;
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_NUMBER) {
        double target = val.as.number;
        vm_jump(vm, target, NULL);
    }

    return err;
}

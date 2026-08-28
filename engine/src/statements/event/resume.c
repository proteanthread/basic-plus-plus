// FILENAME: resume.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (continue.c, retry.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, resume.h, string.c)
// NEEDS: libengine (task.h, task.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the RESUME statement in BASIC++.
//
// ---- Includes ----

#include "statements/event/resume.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/task.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>

#ifdef _WIN32
#define strncasecmp _strnicmp
#endif

void stmt_resume_register(void) {
    static const MicroLibMetadata meta = {
        .name = "RESUME",
        .category = "Event Trapping",
        .syntax = "RESUME [0 | NEXT | line_label | TASK id | EVENT name$]",
        .help_text = "Resumes program execution after an error-handling routine finishes, or resumes a suspended task/event.",
        .error_codes = "Error 20: RESUME Without Error, Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_resume_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_RESUME) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    // 1. Extended: RESUME TASK id
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_TASK) ||
        (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "TASK", 4) == 0)) {
        lex_next(lex);
        BValue pid_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (pid_val.type != VAL_NUMBER && pid_val.type != VAL_INTEGER) {
            if (pid_val.type == VAL_STRING && pid_val.as.string) str_release(vm_get_str(vm), pid_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        int pid = (int)pid_val.as.number;
        task_switch(vm_get_vdev(vm), pid);
        return err;
    }

    // 2. Extended: RESUME EVENT name$
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_EVENT) ||
        (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "EVENT", 5) == 0)) {
        lex_next(lex);
        BValue ev_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ev_val.type == VAL_STRING && ev_val.as.string) {
            str_release(vm_get_str(vm), ev_val.as.string);
        }
        return err;
    }

    // 3. Classic GW-BASIC / QBASIC Error Handler Resume
    if (!vm_is_in_error_handler(vm)) {
        err.code = 20;
        err.message = "RESUME Without Error";
        return err;
    }

    vm_set_in_error_handler(vm, false);

    if (tok.type == TOK_EOL || tok.type == TOK_EOF || tok.type == TOK_BACKSLASH) {
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

    if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
        double target = val.as.number;
        vm_jump(vm, target, NULL);
    } else if (val.type == VAL_STRING && val.as.string) {
        str_release(vm_get_str(vm), val.as.string);
    }

    return err;
}

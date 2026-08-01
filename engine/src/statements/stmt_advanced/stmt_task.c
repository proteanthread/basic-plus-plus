/**
 * @file stmt_task.c
 * @brief Multitasking TASK statement handler.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Parses the TASK statement subcommands (LIST, WAIT, KILL, spawn, switch).
 * - Why it exists: Exposes the Task Manager APIs directly to BASIC++ code space.
 * - Why it works this way: It inspects lookahead tokens, decodes targets (string path or numeric PID),
 *   and executes the corresponding manager operation.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Error message strings, default behaviors on missing arguments.
 * - What cannot be changed: Token dispatch routing logic or function return types.
 * - What to expect: Invoking background tasks compiles/loads the file and starts the thread.
 * - What to do if something breaks: Trace token parsing blocks and verify PID lookup matrices.
 */

#include "stmt/stmt.h"
#include "lexer/lexer.h"
#include "runtime/task.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include <stdio.h>
#include <string.h>

BppError stmt_task_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Security check: multitasking requires SECOP_SYSTEM privilege */
    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70; /* Permission denied */
        err.message = "Permission denied: TASK multitasking blocked under sandbox settings";
        return err;
    }

    BppToken tok = lex_peek(lex);

    /* 1. TASK (no args) -> Show list */
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        task_list(vm_get_vdev(vm));
        return err;
    }

    /* 2. Check for keywords: LIST, WAIT, KILL */
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_LIST) {
            lex_next(lex); /* Consume LIST */
            task_list(vm_get_vdev(vm));
            return err;
        }

        if (tok.as.keyword == KW_WAIT) {
            lex_next(lex); /* Consume WAIT */
            BValue pid_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;

            int pid = 0;
            if (pid_val.type == VAL_NUMBER) {
                pid = (int)pid_val.as.number;
            } else {
                err.code = 13; err.message = "Type mismatch: Expected numeric PID for TASK WAIT";
                return err;
            }

            task_join(pid);
            return err;
        }

        if (tok.as.keyword == KW_KILL) {
            lex_next(lex); /* Consume KILL */
            BValue pid_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;

            int pid = 0;
            if (pid_val.type == VAL_NUMBER) {
                pid = (int)pid_val.as.number;
            } else {
                err.code = 13; err.message = "Type mismatch: Expected numeric PID for TASK KILL";
                return err;
            }

            task_kill(vm_get_vdev(vm), pid);
            return err;
        }
    }

    /* 3. Check for global label target: TASK ::label */
    if (tok.type == TOK_GLOBAL_LABEL) {
        lex_next(lex); /* Consume label */
        char label_name[64];
        int len = (int)(tok.length < sizeof(label_name) - 1 ? tok.length : sizeof(label_name) - 1);
        memcpy(label_name, tok.as.string, len);
        label_name[len] = '\0';

        const char *cur_file = vm_get_current_filename(vm);
        if (!cur_file || cur_file[0] == '\0') {
            err.code = 70; err.message = "Task spawn at global label requires a loaded program file";
            return err;
        }

        task_spawn_at_label(vm_get_vdev(vm), cur_file, label_name);
        return err;
    }

    /* 4. Otherwise: Evaluate expression (can be string filename or numeric PID) */
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_STRING) {
        const char *filename = str_data(val.as.string);
        task_spawn(vm_get_vdev(vm), filename);
        str_release(vm_get_str(vm), val.as.string);
    } else if (val.type == VAL_NUMBER) {
        int pid = (int)val.as.number;
        task_switch(vm_get_vdev(vm), pid);
    } else {
        err.code = 2; err.message = "Syntax error: Expected string filename or numeric PID after TASK";
    }

    return err;
}

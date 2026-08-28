// FILENAME: task.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (goodbye.c, resume.c, suspend.c, system.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt.h, string.c, task.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the TASK statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "lexer/lexer.h"
#include "runtime/task.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <stdio.h>
#include <string.h>

BppError stmt_task_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    // Security check: multitasking requires SECOP_SYSTEM privilege
    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70; // Permission denied
        err.message = "Permission denied: TASK multitasking blocked under sandbox settings";
        return err;
    }

    BppToken tok = lex_peek(lex);

    // 1. TASK (no args) -> Show list
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        task_list(vm_get_vdev(vm));
        return err;
    }

    // 2. Check for keywords: LIST, WAIT, KILL
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_LIST) {
            lex_next(lex); // Consume LIST
            task_list(vm_get_vdev(vm));
            return err;
        }

        if (tok.as.keyword == KW_WAIT) {
            lex_next(lex); // Consume WAIT
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
            lex_next(lex); // Consume KILL
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

    // 3. Check for global label target: TASK ::label
    if (tok.type == TOK_GLOBAL_LABEL) {
        lex_next(lex); // Consume label
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

    // 4. Otherwise: Evaluate expression (can be string filename or numeric PID)
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

void stmt_task_register(void) {
    static const MicroLibMetadata meta = {
        .name = "TASK",
        .category = "Control Flow",
        .syntax = "TASK [filename_expr$ | ::label | LIST | WAIT task_id | KILL task_id]",
        .help_text = "Manages concurrent background script tasks and thread execution.",
        .error_codes = "Error 2: Syntax Error, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}


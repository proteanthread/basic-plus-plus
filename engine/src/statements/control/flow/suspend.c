// FILENAME: suspend.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, string.c, suspend.h, task.h, task.c, vm.h)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SUSPEND statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/suspend.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/task.h"
#include "types/errors.h"
#include "platform/platform.h"
#include "vm/vm.h"
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#ifdef _WIN32
#define strncasecmp _strnicmp
#endif

extern int platform_inkey_char(void);

void stmt_suspend_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SUSPEND",
        .category = "Control Flow",
        .syntax = "SUSPEND [TIMER | KEY | COM | TASK id | EVENT name$ | expr] [, timeout]",
        .help_text = "Suspends execution until the specified event occurs, key is pressed, timer triggers, or timeout expires.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

typedef enum {
    SUSPEND_MODE_ANY,
    SUSPEND_MODE_TIMER,
    SUSPEND_MODE_KEY,
    SUSPEND_MODE_COM,
    SUSPEND_MODE_TASK,
    SUSPEND_MODE_EVENT
} SuspendMode;

BppError stmt_suspend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    SuspendMode mode = SUSPEND_MODE_ANY;
    int target_task_pid = 0;
    char target_event_name[64] = {0};
    double timeout_secs = -1.0; // -1 means wait indefinitely or until event

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_SUSPEND) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_BACKSLASH) {
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_TIMER) ||
            (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "TIMER", 5) == 0)) {
            mode = SUSPEND_MODE_TIMER;
            lex_next(lex);
        } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_KEY) ||
                   (tok.type == TOK_IDENT && tok.length == 3 && strncasecmp(tok.start, "KEY", 3) == 0)) {
            mode = SUSPEND_MODE_KEY;
            lex_next(lex);
        } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_COM) ||
                   (tok.type == TOK_IDENT && tok.length == 3 && strncasecmp(tok.start, "COM", 3) == 0)) {
            mode = SUSPEND_MODE_COM;
            lex_next(lex);
        } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_TASK) ||
                   (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "TASK", 4) == 0)) {
            mode = SUSPEND_MODE_TASK;
            lex_next(lex);
            BValue pid_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (pid_val.type != VAL_NUMBER && pid_val.type != VAL_INTEGER) {
                if (pid_val.type == VAL_STRING && pid_val.as.string) str_release(vm_get_str(vm), pid_val.as.string);
                err.code = ERR_TYPE_MISMATCH;
                return err;
            }
            target_task_pid = (int)pid_val.as.number;
        } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_EVENT) ||
                   (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "EVENT", 5) == 0)) {
            mode = SUSPEND_MODE_EVENT;
            lex_next(lex);
            BValue ev_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (ev_val.type == VAL_STRING && ev_val.as.string) {
                const char *s = str_data(ev_val.as.string);
                snprintf(target_event_name, sizeof(target_event_name), "%s", s ? s : "");
                str_release(vm_get_str(vm), ev_val.as.string);
            }
        } else {
            // Evaluate expression (could be numeric timeout or event name)
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                timeout_secs = val.as.number;
            } else if (val.type == VAL_STRING && val.as.string) {
                mode = SUSPEND_MODE_EVENT;
                const char *s = str_data(val.as.string);
                snprintf(target_event_name, sizeof(target_event_name), "%s", s ? s : "");
                str_release(vm_get_str(vm), val.as.string);
            }
        }

        // Check for trailing comma and timeout: SUSPEND KEY, 5.0
        BppToken comma_tok = lex_peek(lex);
        if (comma_tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue to_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (to_val.type == VAL_NUMBER || to_val.type == VAL_INTEGER) {
                timeout_secs = to_val.as.number;
            } else {
                if (to_val.type == VAL_STRING && to_val.as.string) str_release(vm_get_str(vm), to_val.as.string);
            }
        }
    }

    double start_time = platform_get_uptime();

    // Event suspension polling loop
    while (vm && vm_is_running(vm)) {
        double now = platform_get_uptime();
        if (timeout_secs >= 0.0 && (now - start_time >= timeout_secs)) {
            break;
        }

        // Trigger active event hooks
        vm_trigger_event_polling(vm);

        if (mode == SUSPEND_MODE_ANY) {
            if (timeout_secs < 0.0) {
                // If indefinite bare SUSPEND, break after a slice or input
                if (platform_inkey_char() > 0) break;
            }
        } else if (mode == SUSPEND_MODE_KEY) {
            if (platform_inkey_char() > 0) break;
        } else if (mode == SUSPEND_MODE_TASK) {
            int st = task_get_status(target_task_pid);
            if (st == TASK_DONE || st == TASK_DONE_ERR || st == TASK_ERROR || st == -1) {
                break;
            }
        }

        platform_sleep_ms(5);
        if (timeout_secs < 0.0 && (platform_get_uptime() - start_time >= 0.05)) {
            break; // default unblocked progression
        }
    }

    return err;
}

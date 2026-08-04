/**
 * @file stmt_debug.c
 * @brief ASSERT, TRON, TROFF, BREAK, and VARS statement handlers.
 *
 * SECTION 1: WHAT IT DOES
 * - Implements built-in debugging commands:
 *   - ASSERT: Evaluates a condition; if false, throws a runtime error or triggers a breakpoint.
 *   - TRON / TROFF: Enforces statement execution trace logs ([Line N]).
 *   - BREAK: Triggers a manual breakpoint inside the interactive prompt.
 *   - VARS: Lists all active variables in the current scope.
 *
 * SECTION 2: WHY IT EXISTS
 * - Provides native debugging capabilities directly in standard REPL and program execution,
 *   supporting early sanity checking and workspace analysis.
 *
 * SECTION 3: WHY IT WORKS THIS WAY
 * - Statement callbacks evaluate expressions, read/write trace flags in `BppLogger`,
 *   and invoke unified breakpoint hooks or variable dump utilities.
 *
 * SECTION 4: WHAT CAN BE CHANGED
 * - Breakpoint message formatting, default assertion error codes.
 *
 * SECTION 5: WHAT CANNOT BE CHANGED
 * - Variable traversal API interactions (must respect the VariableContext encapsulation).
 *
 * SECTION 6: WHAT TO EXPECT
 * - Bounded stack and memory consumption. Bypasses actual executions under dry-run mode where appropriate.
 *
 * SECTION 7: WHAT TO DO IF SOMETHING BREAKS
 * - Verify the debug mode is active and console VDev streams are properly configured.
 *
 * SECTION 8: ASSUMPTIONS
 * - Standard C17 runtime behaviors.
 *
 * SECTION 9: PORTABILITY CONCERNS
 * - Platform-independent logic conforming to cross-platform standard APIs.
 *
 * SECTION 10: FUTURE EXPANSIONS
 * - Dynamic expression watchpoints.
 *
 * SECTION 11: EXTERNAL EXTENSION HOOKS
 * - Custom debugger backends can override these dispatches to hook into third-party tools.
 */

#include "bpp_stmt.h"
#include "bpp_eval.h"
#include "bpp_logger.h"
#include "bpp_vdev.h"
#include "bpp_variables.h"
#include <string.h>
#include <stdlib.h>

extern void vm_trigger_breakpoint(VMContext *vm, const char *reason);

/* ASSERT statement handler */
BppError stmt_assert_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Evaluate assertion condition */
    BValue cond_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (cond_val.type == VAL_STRING) {
        err.code = 13; /* Type mismatch */
        err.message = "Assertion condition must be numeric";
        return err;
    }

    bool assertion_passed = (cond_val.as.number != 0.0);

    /* Parse optional custom error message */
    const char *custom_msg = NULL;
    BppToken comma = lex_peek(lex);
    if (comma.type == TOK_COMMA) {
        lex_next(lex); /* Consume ',' */
        BValue msg_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (msg_val.type != VAL_STRING) {
            err.code = 13;
            err.message = "Assertion message must be a string";
            return err;
        }
        if (msg_val.as.string) {
            custom_msg = str_data(msg_val.as.string);
        }
    }

    if (!assertion_passed) {
        char reason[256];
        if (custom_msg) {
            snprintf(reason, sizeof(reason), "Assertion failed: %s", custom_msg);
        } else {
            snprintf(reason, sizeof(reason), "Assertion failed");
        }

        bpp_log_error("%s", reason);

        if (bpp_logger_is_debug()) {
            vm_trigger_breakpoint(vm, reason);
        } else {
            err.code = 99; /* Custom assertion failure code */
            /* We need to assign a persistent message string that won't go out of scope,
             * or format it into a scratch space if supported. We copy to scratch or set standard. */
            if (custom_msg) {
                /* Copy message to program memory scratch space */
                char *sc = (char *)mem_scratch_alloc(vm_get_mem(vm), strlen(reason) + 1);
                if (sc) {
                    strcpy(sc, reason);
                    err.message = sc;
                } else {
                    err.message = "Assertion failed";
                }
            } else {
                err.message = "Assertion failed";
            }
        }
    }

    return err;
}

/* TRON statement handler */
BppError stmt_tron_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    bpp_logger_set_trace(true);
    bpp_log_info("Trace turned ON (TRON)");
    return err;
}

/* TROFF statement handler */
BppError stmt_troff_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    bpp_logger_set_trace(false);
    bpp_log_info("Trace turned OFF (TROFF)");
    return err;
}

/* BREAK statement handler */
BppError stmt_break_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    bpp_log_info("Manual breakpoint hit (BREAK)");
    if (bpp_logger_is_debug()) {
        vm_trigger_breakpoint(vm, "Manual breakpoint (BREAK statement)");
    } else {
        /* In non-debug mode, BREAK is treated as a log info trace (no-op) */
        VDevContext *vdev = vm_get_vdev(vm);
        if (vdev) {
            vdev_printf(vdev, "[BREAKPOINT at line %lld]\n", (long long)vm_get_current_line(vm));
        }
    }
    return err;
}

/* VARS statement handler */
BppError stmt_vars_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    VDevContext *vdev = vm_get_vdev(vm);
    var_print_all(vm_get_var(vm), vdev);
    return err;
}

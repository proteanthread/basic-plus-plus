/**
 * @file debug.c
 * @brief ASSERT, TRON, TROFF, BREAK, and VARS debugging statement handlers for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements debugging statements:
 * - ASSERT condition [, msg$]: Evaluates condition; triggers runtime error (ERR_ASSERTION_FAILED) if false.
 * - TRON / TROFF: Enables or disables line execution tracing.
 * - BREAK: Triggers a manual breakpoint pause in interactive REPL mode.
 * - VARS: Dumps active variables in current variable context.
 *
 * 2. WHY IT EXISTS:
 * Provides runtime program diagnostics and debugging capabilities per GW-BASIC / QBASIC standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * ASSERT evaluates expression; TRON/TROFF toggles trace_enabled flag in VMContext; VARS iterates over variable context entries using var_dump().
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_debug'. Includes "statements/system/debug.h",
 * "eval/eval.h", "runtime/variables.h", "vm/vm.h", "device/vdev.h", "security/security.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support conditional expression watchpoints (WATCH expr) and stack trace dumps (STACK).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Variable iteration discipline: MUST use var_dump() / var_lookup() public API without accessing private hash buckets directly.
 *
 * 8. WHAT TO EXPECT:
 * Toggles trace state or outputs variable listing; returns ERR_NONE or ERR_ASSERTION_FAILED.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check console output routing in vdev_printf().
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and VariableContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 7-bit ASCII text output.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/eval.c
 * - engine/src/runtime/variables.c
 * - engine/src/device/vdev.c
 * Prerequisite Header Files:
 * - engine/include/statements/system/debug.h
 * - engine/include/vm/vm.h
 * - engine/include/runtime/variables.h
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "debug/logger.h"
#include "device/vdev.h"
#include "runtime/variables.h"
#include "runtime/micro_lib_metadata.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
#include "types/version.h"
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

    if (vm_is_in_test(vm)) {
        vm_inc_test_total(vm);
        if (assertion_passed) {
            vm_inc_test_pass(vm);
        } else {
            vm_inc_test_fail(vm);
        }
    }

    if (!assertion_passed) {
        char reason[256];
        if (custom_msg) {
            snprintf(reason, sizeof(reason), "Assertion failed: %s", custom_msg);
        } else {
            snprintf(reason, sizeof(reason), "Assertion failed");
        }

        log_error("%s", reason);

        if (logger_is_debug()) {
            vm_trigger_breakpoint(vm, reason);
        } else {
            err.code = 99; /* Custom assertion failure code */
            if (custom_msg) {
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
    logger_set_trace(true);
    log_info("Trace turned ON (TRON)");
    return err;
}

/* TROFF statement handler */
BppError stmt_troff_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    logger_set_trace(false);
    log_info("Trace turned OFF (TROFF)");
    return err;
}

/* BREAK statement handler */
BppError stmt_break_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    log_info("Manual breakpoint hit (BREAK)");
    if (logger_is_debug()) {
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

/* BACKTRACE statement handler */
BppError stmt_backtrace_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "=== CALL STACK BACKTRACE ===\n");
    vdev_printf(vdev, "  Line: %lld\n", (long long)vm_get_current_line(vm));
    return err;
}

/* INFO statement handler */
BppError stmt_info_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    VDevContext *vdev = vm_get_vdev(vm);
    MemoryContext *mem = vm_get_mem(vm);

    size_t free_ram = mem ? mem_get_free_ram(mem) : 0;
    size_t used_ram = mem ? mem_get_used_ram(mem) : 0;
    const char *ver_tag = mem ? mem_program_get_version(mem) : "";
    if (!ver_tag || !ver_tag[0]) ver_tag = "(Untagged)";

    vdev_printf(vdev, "=== BASIC++ SYSTEM INFO DASHBOARD ===\n");
    vdev_printf(vdev, "  Engine Version:  %s\n", VERSION_STRING);
    vdev_printf(vdev, "  Program Version: %s\n", ver_tag);
    vdev_printf(vdev, "  Used RAM:        %zu bytes\n", used_ram);
    vdev_printf(vdev, "  Free RAM:        %zu bytes\n\n", free_ram);

    return err;
}

/* DUMP statement handler */
BppError stmt_dump_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    VDevContext *vdev = vm_get_vdev(vm);
    MemoryContext *mem = vm_get_mem(vm);

    char category[32] = "ALL";
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        snprintf(category, sizeof(category), "%.*s", (int)tok.length, tok.start);
    }

    vdev_printf(vdev, "=== DEBUG STATE DUMP (%s) ===\n", category);
    vdev_printf(vdev, "  Current Line:    %lld\n", (long long)vm_get_current_line(vm));
    vdev_printf(vdev, "  Program Version: %s\n", mem ? mem_program_get_version(mem) : "");
    vdev_printf(vdev, "  Debug Mode:      %s\n\n", vm_get_debug_active(vm) ? "ACTIVE" : "INACTIVE");

    return err;
}

/* TRACE statement handler */
BppError stmt_trace_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_OFF) {
        logger_set_trace(false);
        vm_set_single_step(vm, false);
    } else {
        logger_set_trace(true);
        vm_set_single_step(vm, true);
    }
    return err;
}

/* DEBUG statement handler */
BppError stmt_debug_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    char subcmd[32] = "ON";

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        snprintf(subcmd, sizeof(subcmd), "%.*s", (int)tok.length, tok.start);
    }

    if (strcasecmp(subcmd, "OFF") == 0) {
        vm_set_debug_active(vm, false);
    } else if (strcasecmp(subcmd, "DUMP") == 0) {
        return stmt_dump_handler(vm, lex);
    } else if (strcasecmp(subcmd, "STACK") == 0) {
        return stmt_backtrace_handler(vm, lex);
    } else if (strcasecmp(subcmd, "MEMORY") == 0) {
        return stmt_info_handler(vm, lex);
    } else if (strcasecmp(subcmd, "VARS") == 0) {
        return stmt_vars_handler(vm, lex);
    } else {
        vm_set_debug_active(vm, true);
    }

    return err;
}

void stmt_debug_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DEBUG",
        .category = "Debug & Testing",
        .syntax = "DEBUG [ON|OFF|DUMP|STACK|MEMORY|VARS] | TRACE | TRON | TROFF | BREAK | CONT | BACKTRACE | INFO | DUMP",
        .help_text = "Master interactive debugger control, execution tracing, breakpoint handling, and system introspection suite.",
        .error_codes = "Error 2: Syntax Error, Error 99: Assertion Failed"
    };
    microlib_register(&meta);
}


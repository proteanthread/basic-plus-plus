/**
 * @file stmt_event.c
 * @brief Statement handlers for Event Trapping (TIMER and KEY ON/OFF/STOP/GOSUB).
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements TIMER and KEY control statements for enabling/disabling event traps.
 * - Why it exists: Provides GW-BASIC and QBasic compatible asynchronous event trapping.
 * - Why it works this way: It parses statement parameters and configures the event trap status inside the VM context.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Trap limits, default key assignments.
 * - What cannot be changed: Control flow bounds.
 * - What to expect: Event triggers execute a VM jump mimicking GOSUB when polled between program lines.
 * - What to do if something breaks: Trace event state variables inside VMContext.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Relies on standard high-resolution timer access wrappers.
 * - Portability concerns: ANSI C17 standard compliant.
 */

#include "bpp_stmt.h"
#include "bpp_lexer.h"
#include "bpp_vm.h"
#include "bpp_eval.h"
#include "bpp_vdev.h"
#include "bpp_strings.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

BppError stmt_timer_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected ON, OFF, or STOP after TIMER";
        return err;
    }

    if (tok.as.keyword == KW_ON) {
        vm_set_timer_state(vm, 1);
    } else if (tok.as.keyword == KW_OFF) {
        vm_set_timer_state(vm, 0);
    } else if (tok.as.keyword == KW_STOP) {
        vm_set_timer_state(vm, 2);
    } else {
        err.code = 2; err.message = "Expected ON, OFF, or STOP after TIMER";
    }

    return err;
}

BppError stmt_key_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);

    /* Check for KEY(n) syntax */
    if (tok.type == TOK_LPAREN) {
        lex_next(lex); /* Consume '(' */
        BValue key_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (key_val.type == VAL_STRING) {
            err.code = 13; err.message = "Key index must be numeric";
            return err;
        }

        int idx = (int)key_val.as.number;
        if (idx < 1 || idx > 10) {
            err.code = 5; err.message = "Illegal function call: Key index must be 1 to 10";
            return err;
        }

        BppToken rparen = lex_next(lex);
        if (rparen.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after key index";
            return err;
        }

        BppToken action = lex_next(lex);
        if (action.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected ON, OFF, or STOP after KEY(...)";
            return err;
        }

        if (action.as.keyword == KW_ON) {
            vm_set_key_state(vm, idx, 1);
        } else if (action.as.keyword == KW_OFF) {
            vm_set_key_state(vm, idx, 0);
        } else if (action.as.keyword == KW_STOP) {
            vm_set_key_state(vm, idx, 2);
        } else {
            err.code = 2; err.message = "Expected ON, OFF, or STOP after KEY(...)";
        }
        return err;
    }

    /* Check for: KEY ON, KEY OFF, KEY LIST, KEY n, code */
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_ON || tok.as.keyword == KW_OFF) {
            lex_next(lex); /* Consume ON/OFF */
            /* Visual display of key macros: NOP on headless console */
            return err;
        } else if (tok.as.keyword == KW_LIST) {
            lex_next(lex); /* Consume LIST */
            VDevContext *vdev = vm_get_vdev(vm);
            vdev_printf(vdev, "Trap Keys Registered:\n");
            for (int i = 1; i <= 10; ++i) {
                vdev_printf(vdev, "  Key %d: ASCII Code %d\n", i, '0' + i);
            }
            return err;
        }
    }

    /* Check for: KEY n, expr */
    BValue key_idx_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (key_idx_val.type == VAL_STRING) {
        err.code = 13; err.message = "Key index must be numeric";
        return err;
    }

    int idx = (int)key_idx_val.as.number;
    if (idx < 1 || idx > 10) {
        err.code = 5; err.message = "Illegal function call: Key index must be 1 to 10";
        return err;
    }

    BppToken comma = lex_next(lex);
    if (comma.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after key index";
        return err;
    }

    BValue expr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (expr_val.type == VAL_NUMBER) {
        int code = (int)expr_val.as.number;
        vm_set_key_trap(vm, idx, code, 0); /* Updates key_code, line remains unchanged */
    } else {
        /* String: define key macro. NOP for event trap definition. */
        str_release(vm_get_str(vm), expr_val.as.string);
    }

    return err;
}

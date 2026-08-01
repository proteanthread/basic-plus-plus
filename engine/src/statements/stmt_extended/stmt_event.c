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

#include "stmt/stmt.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/strings.h"
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
        if (idx < 1 || idx >= 15100) {
            err.code = 5; err.message = "Illegal function call: Key index must be 1 to 15099";
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
            for (int i = 1; i < 15100; ++i) {
                if (vm_get_key_state(vm, i) > 0) {
                    vdev_printf(vdev, "  Key %d: State %d, Code %d, Target %d\n", i, vm_get_key_state(vm, i), vm_get_key_code(vm, i), (int)vm_get_key_gosub_line(vm, i));
                }
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
    if (idx < 1 || idx >= 15100) {
        err.code = 5; err.message = "Illegal function call: Key index must be 1 to 15099";
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

BppError stmt_alarm_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex); /* Consume '(' */
        BValue secs_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (secs_val.type == VAL_STRING) {
            err.code = 13; err.message = "ALARM argument must be numeric";
            return err;
        }
        double secs = secs_val.as.number;

        BppToken rparen = lex_next(lex);
        if (rparen.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after ALARM argument";
            return err;
        }

        BppToken next = lex_peek(lex);
        if (next.type == TOK_EQ) {
            err.code = 2; err.message = "Syntax error: assignment '=' not allowed. Use SET ALARM(n) ON/OFF/STOP/PAUSE/SNOOZE/UNSET";
            return err;
        }

        int state = 1;
        bool is_snooze = false;
        double snooze_amount = 300.0;
        bool is_unset = false;

        if (next.type == TOK_KEYWORD) {
            lex_next(lex); /* Consume keyword */
            if (next.as.keyword == KW_ON) {
                state = 1;
            } else if (next.as.keyword == KW_OFF) {
                state = 0;
            } else if (next.as.keyword == KW_STOP || next.as.keyword == KW_PAUSE) {
                state = 2;
            } else if (next.as.keyword == KW_UNSET) {
                is_unset = true;
            } else if (next.as.keyword == KW_SNOOZE) {
                is_snooze = true;
                /* check for optional expression */
                BppToken opt = lex_peek(lex);
                if (opt.type != TOK_EOF && opt.type != TOK_EOL) {
                    BValue snz_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (snz_val.type == VAL_STRING) {
                        err.code = 13; err.message = "SNOOZE duration must be numeric";
                        return err;
                    }
                    snooze_amount = snz_val.as.number;
                }
            } else {
                err.code = 2; err.message = "Expected ON, OFF, STOP, PAUSE, SNOOZE, or UNSET after ALARM(...)";
                return err;
            }
        } else {
            /* Default to ON */
            state = 1;
        }

        if (is_unset) {
            vm_unset_alarm_countdown(vm, secs);
        } else if (is_snooze) {
            vm_snooze_alarm_countdown(vm, secs, snooze_amount);
        } else {
            vm_set_alarm_countdown_state(vm, secs, state);
        }
        return err;
    }

    /* Parameterless: ALARM ON/OFF/STOP/PAUSE/SNOOZE/UNSET */
    if (tok.type == TOK_KEYWORD) {
        lex_next(lex);
        if (tok.as.keyword == KW_ON) {
            vm_set_global_alarm_state(vm, 1);
        } else if (tok.as.keyword == KW_OFF || tok.as.keyword == KW_UNSET) {
            vm_set_global_alarm_state(vm, 0);
        } else if (tok.as.keyword == KW_STOP || tok.as.keyword == KW_PAUSE) {
            vm_set_global_alarm_state(vm, 2);
        } else if (tok.as.keyword == KW_SNOOZE) {
            double snooze_amount = 300.0;
            BppToken opt = lex_peek(lex);
            if (opt.type != TOK_EOF && opt.type != TOK_EOL) {
                BValue snz_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (snz_val.type == VAL_STRING) {
                    err.code = 13; err.message = "SNOOZE duration must be numeric";
                    return err;
                }
                snooze_amount = snz_val.as.number;
            }
            vm_snooze_all_countdowns(vm, snooze_amount);
        } else {
            err.code = 2; err.message = "Expected ON, OFF, STOP, PAUSE, SNOOZE, or UNSET after ALARM";
        }
        return err;
    }

    if (tok.type == TOK_EQ) {
        err.code = 2; err.message = "Syntax error: assignment '=' not allowed. Use SET ALARM ON/OFF/STOP/PAUSE/SNOOZE/UNSET";
        return err;
    }

    err.code = 2; err.message = "Expected '(' or ON/OFF/STOP/PAUSE/SNOOZE/UNSET after ALARM";
    return err;
}

BppError stmt_alarm_str_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex); /* Consume '(' */
        BValue time_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        char time_str[128] = "";
        if (time_val.type == VAL_NUMBER) {
            int val_i = (int)time_val.as.number;
            int h = val_i / 10000;
            int m = (val_i % 10000) / 100;
            int s = val_i % 100;
            if (h < 0 || h >= 24 || m < 0 || m >= 60 || s < 0 || s >= 60) {
                err.code = 5; err.message = "Illegal function call: Invalid time format";
                return err;
            }
            snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", h, m, s);
        } else {
            size_t tlen = (str_len(time_val.as.string) < 127) ? str_len(time_val.as.string) : 127;
            memcpy(time_str, str_data(time_val.as.string), tlen);
            time_str[tlen] = '\0';
            str_release(vm_get_str(vm), time_val.as.string);
        }

        extern bool vm_validate_time_str(const char *time_str, int *out_secs);
        if (!vm_validate_time_str(time_str, NULL)) {
            err.code = 5; err.message = "Illegal function call: Invalid time format";
            return err;
        }

        BppToken rparen = lex_next(lex);
        if (rparen.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after ALARM$ argument";
            return err;
        }

        BppToken next = lex_peek(lex);
        if (next.type == TOK_EQ) {
            err.code = 2; err.message = "Syntax error: assignment '=' not allowed. Use SET ALARM$(time) ON/OFF/STOP/PAUSE/SNOOZE/UNSET";
            return err;
        }

        int state = 1;
        bool is_snooze = false;
        int snooze_amount = 300;
        bool is_unset = false;

        if (next.type == TOK_KEYWORD) {
            lex_next(lex); /* Consume keyword */
            if (next.as.keyword == KW_ON) {
                state = 1;
            } else if (next.as.keyword == KW_OFF) {
                state = 0;
            } else if (next.as.keyword == KW_STOP || next.as.keyword == KW_PAUSE) {
                state = 2;
            } else if (next.as.keyword == KW_UNSET) {
                is_unset = true;
            } else if (next.as.keyword == KW_SNOOZE) {
                is_snooze = true;
                /* check for optional expression */
                BppToken opt = lex_peek(lex);
                if (opt.type != TOK_EOF && opt.type != TOK_EOL) {
                    BValue snz_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (snz_val.type == VAL_STRING) {
                        err.code = 13; err.message = "SNOOZE duration must be numeric";
                        return err;
                    }
                    snooze_amount = (int)snz_val.as.number;
                }
            } else {
                err.code = 2; err.message = "Expected ON, OFF, STOP, PAUSE, SNOOZE, or UNSET after ALARM$(...)";
                return err;
            }
        } else {
            /* Default to ON */
            state = 1;
        }

        if (is_unset) {
            vm_unset_alarm_daily(vm, time_str);
        } else if (is_snooze) {
            vm_snooze_alarm_daily(vm, time_str, snooze_amount);
        } else {
            vm_set_alarm_daily_state(vm, time_str, state);
        }
        return err;
    }

    /* Parameterless: ALARM$ ON/OFF/STOP/PAUSE/SNOOZE/UNSET */
    if (tok.type == TOK_KEYWORD) {
        lex_next(lex);
        if (tok.as.keyword == KW_ON) {
            vm_set_global_alarm_str_state(vm, 1);
        } else if (tok.as.keyword == KW_OFF || tok.as.keyword == KW_UNSET) {
            vm_set_global_alarm_str_state(vm, 0);
        } else if (tok.as.keyword == KW_STOP || tok.as.keyword == KW_PAUSE) {
            vm_set_global_alarm_str_state(vm, 2);
        } else if (tok.as.keyword == KW_SNOOZE) {
            int snooze_amount = 300;
            BppToken opt = lex_peek(lex);
            if (opt.type != TOK_EOF && opt.type != TOK_EOL) {
                BValue snz_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (snz_val.type == VAL_STRING) {
                    err.code = 13; err.message = "SNOOZE duration must be numeric";
                    return err;
                }
                snooze_amount = (int)snz_val.as.number;
            }
            vm_snooze_all_dailies(vm, snooze_amount);
        } else {
            err.code = 2; err.message = "Expected ON, OFF, STOP, PAUSE, SNOOZE, or UNSET after ALARM$";
        }
        return err;
    }

    if (tok.type == TOK_EQ) {
        err.code = 2; err.message = "Syntax error: assignment '=' not allowed. Use SET ALARM$ ON/OFF/STOP/PAUSE/SNOOZE/UNSET";
        return err;
    }

    err.code = 2; err.message = "Expected '(' or ON/OFF/STOP/PAUSE/SNOOZE/UNSET after ALARM$";
    return err;
}

BppError stmt_set_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || (tok.as.keyword != KW_ALARM && tok.as.keyword != KW_ALARM_STR)) {
        err.code = 2; err.message = "Expected ALARM or ALARM$ after SET";
        return err;
    }

    if (tok.as.keyword == KW_ALARM) {
        return stmt_alarm_handler(vm, lex);
    } else {
        return stmt_alarm_str_handler(vm, lex);
    }
}

BppError stmt_com_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    int port = 1;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        port = (int)val.as.number;
        BppToken rparen = lex_next(lex);
        if (rparen.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after COM port index";
            return err;
        }
    }

    if (port < 1 || port > 4) {
        err.code = 5; err.message = "Illegal function call: COM port index must be 1 to 4";
        return err;
    }

    BppToken action = lex_next(lex);
    if (action.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected ON, OFF, or STOP after COM(...)";
        return err;
    }

    if (action.as.keyword == KW_ON) vm_set_com_state(vm, port, 1);
    else if (action.as.keyword == KW_OFF) vm_set_com_state(vm, port, 0);
    else if (action.as.keyword == KW_STOP) vm_set_com_state(vm, port, 2);
    else { err.code = 2; err.message = "Expected ON, OFF, or STOP after COM(...)"; }
    return err;
}

BppError stmt_pen_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken action = lex_next(lex);
    if (action.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected ON, OFF, or STOP after PEN";
        return err;
    }
    if (action.as.keyword == KW_ON) vm_set_pen_state(vm, 1);
    else if (action.as.keyword == KW_OFF) vm_set_pen_state(vm, 0);
    else if (action.as.keyword == KW_STOP) vm_set_pen_state(vm, 2);
    else { err.code = 2; err.message = "Expected ON, OFF, or STOP after PEN"; }
    return err;
}

BppError stmt_strig_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    int strig_idx = 0;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        strig_idx = (int)val.as.number;
        BppToken rparen = lex_next(lex);
        if (rparen.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after STRIG index";
            return err;
        }
    }

    if (strig_idx < 0 || strig_idx > 4) {
        err.code = 5; err.message = "Illegal function call: STRIG index must be 0 to 4";
        return err;
    }

    BppToken action = lex_next(lex);
    if (action.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected ON, OFF, or STOP after STRIG(...)";
        return err;
    }

    if (action.as.keyword == KW_ON) vm_set_strig_state(vm, strig_idx, 1);
    else if (action.as.keyword == KW_OFF) vm_set_strig_state(vm, strig_idx, 0);
    else if (action.as.keyword == KW_STOP) vm_set_strig_state(vm, strig_idx, 2);
    else { err.code = 2; err.message = "Expected ON, OFF, or STOP after STRIG(...)"; }
    return err;
}

/**
 * @file stmt_error.c
 * @brief Error trapping and flow branching statements command handlers (ON ERROR GOTO, RESUME, and computed ON GOTO/GOSUB).
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements error trapping and computed control flow:
 *   - ON ERROR GOTO: Registers a line number to branch to upon runtime error.
 *   - computed ON GOTO/GOSUB: Evaluates a index and branches to a line from a comma-separated list of targets.
 *   - RESUME: Resumes execution after error handling at same statement, next statement, or specific line.
 * - Why it exists: Provides structured error recovery and indexed branching controls.
 * - Why it works this way: It mutates VM context variables (error_trap_line, in_error_handler, error_occurred_line)
 *   and calls vm_jump to branch iteratively without recursive C stack allocations.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Maximum computed branch limits (currently 64), default error codes, resume states.
 * - What cannot be changed: Obligation to check in_error_handler before RESUME.
 * - What to expect: Calling RESUME when not in an active error handler returns error 20 (RESUME without error).
 * - What to do if something breaks: Trace resume jump coordinates.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Targets in list correspond to existing program line numbers.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add computed local GOSUBs or custom error handlers mappings.
 * - How to write external extensions: Plugins trigger errors which route naturally through the trap.
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include <string.h>

/* ON statement handler (Computed ON GOTO/GOSUB and ON ERROR GOTO) */
BppError stmt_on_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_ALARM || tok.as.keyword == KW_ALARM_STR)) {
        BppKeywordId alarm_kw = tok.as.keyword;
        lex_next(lex); /* Consume ALARM or ALARM$ */
        
        bool specific = false;
        double secs = -1.0;
        char time_str[128] = "";
        
        if (lex_peek(lex).type == TOK_LPAREN) {
            specific = true;
            lex_next(lex); /* Consume '(' */
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            
            if (alarm_kw == KW_ALARM) {
                if (val.type == VAL_STRING) {
                    err.code = 13; err.message = "ALARM interval must be numeric";
                    return err;
                }
                secs = val.as.number;
            } else {
                if (val.type != VAL_STRING) {
                    err.code = 13; err.message = "ALARM$ time must be string";
                    return err;
                }
                size_t len = (str_len(val.as.string) < 127) ? str_len(val.as.string) : 127;
                memcpy(time_str, str_data(val.as.string), len);
                time_str[len] = '\0';
                str_release(vm_get_str(vm), val.as.string);
                
                extern bool vm_validate_time_str(const char *time_str, int *out_secs);
                if (!vm_validate_time_str(time_str, NULL)) {
                    err.code = 5; err.message = "Illegal function call: Invalid time format";
                    return err;
                }
            }
            
            BppToken rparen = lex_next(lex);
            if (rparen.type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' after ALARM/ALARM$ argument";
                return err;
            }
        }
        
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON ALARM/ALARM$";
            return err;
        }
        
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        
        if (alarm_kw == KW_ALARM) {
            vm_set_alarm_countdown_trap(vm, secs, line_tok.as.number);
        } else {
            vm_set_alarm_daily_trap(vm, specific ? time_str : NULL, line_tok.as.number);
        }
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TIMER) {
        lex_next(lex); /* Consume TIMER */
        BppToken lparen = lex_next(lex);
        if (lparen.type != TOK_LPAREN) {
            err.code = 2; err.message = "Expected '(' after ON TIMER";
            return err;
        }
        BValue seconds_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (seconds_val.type == VAL_STRING) {
            err.code = 13; err.message = "Timer interval must be numeric";
            return err;
        }
        BppToken rparen = lex_next(lex);
        if (rparen.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after timer interval";
            return err;
        }
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON TIMER(...)";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_timer_trap(vm, seconds_val.as.number, line_tok.as.number);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_KEY) {
        lex_next(lex); /* Consume KEY */
        BppToken lparen = lex_next(lex);
        if (lparen.type != TOK_LPAREN) {
            err.code = 2; err.message = "Expected '(' after ON KEY";
            return err;
        }
        BValue key_idx_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (key_idx_val.type == VAL_STRING) {
            err.code = 13; err.message = "Key index must be numeric";
            return err;
        }
        int key_idx = (int)key_idx_val.as.number;
        if (key_idx < 1 || key_idx >= 15100) {
            err.code = 5; err.message = "Illegal function call: Key index must be 1 to 15099";
            return err;
        }
        BppToken rparen = lex_next(lex);
        if (rparen.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after key index";
            return err;
        }
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON KEY(...)";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_key_trap(vm, key_idx, 0, line_tok.as.number);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_PLAY) {
        lex_next(lex); /* Consume PLAY */
        BppToken lparen = lex_next(lex);
        if (lparen.type != TOK_LPAREN) {
            err.code = 2; err.message = "Expected '(' after ON PLAY";
            return err;
        }
        BValue notes_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (notes_val.type == VAL_STRING) {
            err.code = 13; err.message = "Play note threshold must be numeric";
            return err;
        }
        BppToken rparen = lex_next(lex);
        if (rparen.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after play note threshold";
            return err;
        }
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON PLAY(...)";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_play_trap(vm, (int)notes_val.as.number, line_tok.as.number);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_COM) {
        lex_next(lex);
        int port = 1;
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex);
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            port = (int)val.as.number;
            lex_next(lex); /* Consume ')' */
        }
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON COM";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_com_trap(vm, port, line_tok.as.number);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_PEN) {
        lex_next(lex);
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON PEN";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_pen_trap(vm, line_tok.as.number);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_STRIG) {
        lex_next(lex);
        int strig_idx = 0;
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex);
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            strig_idx = (int)val.as.number;
            lex_next(lex); /* Consume ')' */
        }
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON STRIG";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_strig_trap(vm, strig_idx, line_tok.as.number);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_MOUSE) {
        lex_next(lex); /* Consume MOUSE */
        int target_char = -2; /* use -2 as sentinel for no change */
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex); /* Consume '(' */
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val.type == VAL_STRING) {
                const char *s = str_data(val.as.string);
                target_char = (s && s[0]) ? (unsigned char)s[0] : 32;
            } else {
                target_char = (int)val.as.number;
            }
            BppToken rparen = lex_next(lex);
            if (rparen.type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' after MOUSE hover argument";
                return err;
            }
        }
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON MOUSE";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_mouse_trap(vm, 1, line_tok.as.number, target_char);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_HMOUSE) {
        lex_next(lex); /* Consume HMOUSE */
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON HMOUSE";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_hmouse_trap(vm, 1, line_tok.as.number);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_VMOUSE) {
        lex_next(lex); /* Consume VMOUSE */
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON VMOUSE";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_vmouse_trap(vm, 1, line_tok.as.number);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TRIG) {
        lex_next(lex); /* Consume TRIG */
        int target_btn = -2; /* use -2 as sentinel */
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex); /* Consume '(' */
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val.type == VAL_STRING) {
                err.code = 13; err.message = "TRIG button argument must be numeric";
                return err;
            }
            target_btn = (int)val.as.number;
            BppToken rparen = lex_next(lex);
            if (rparen.type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' after TRIG button argument";
                return err;
            }
        }
        BppToken gosub_tok = lex_next(lex);
        if (gosub_tok.type != TOK_KEYWORD || gosub_tok.as.keyword != KW_GOSUB) {
            err.code = 2; err.message = "Expected GOSUB after ON TRIG";
            return err;
        }
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected line number after GOSUB";
            return err;
        }
        vm_set_trig_trap(vm, 1, line_tok.as.number, target_btn);
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ERROR) {
        lex_next(lex); /* Consume ERROR */

        BppToken goto_tok = lex_next(lex);
        if (goto_tok.type != TOK_KEYWORD || goto_tok.as.keyword != KW_GOTO) {
            err.code = 2; /* Syntax error */
            err.message = "Expected GOTO after ON ERROR";
            return err;
        }

        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2;
            err.message = "Expected line number after ON ERROR GOTO";
            return err;
        }

        vm_set_error_trap(vm, line_tok.as.number);
        return err;
    }

    /* Computed GOTO / GOSUB */
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_STRING) {
        err.code = 13; /* Type mismatch */
        err.message = "Computed index must be numeric";
        return err;
    }

    int idx = (int)val.as.number;

    BppToken branch_tok = lex_next(lex);
    if (branch_tok.type != TOK_KEYWORD || 
        (branch_tok.as.keyword != KW_GOTO && branch_tok.as.keyword != KW_GOSUB)) {
        err.code = 2;
        err.message = "Expected GOTO or GOSUB in ON statement";
        return err;
    }

    bool is_gosub = (branch_tok.as.keyword == KW_GOSUB);

    double targets[64];
    int target_count = 0;

    while (true) {
        if (target_count >= 64) {
            err.code = 2;
            err.message = "Too many computed branches in ON statement";
            return err;
        }

        BppToken target_tok = lex_next(lex);
        if (target_tok.type != TOK_NUMBER) {
            err.code = 2;
            err.message = "Expected line number in branch list";
            return err;
        }

        targets[target_count++] = target_tok.as.number;

        BppToken comma_tok = lex_peek(lex);
        if (comma_tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
        } else {
            break;
        }
    }

    /* Branch if index is within range */
    if (idx >= 1 && idx <= target_count) {
        double target = targets[idx - 1];
        if (is_gosub) {
            /* Record GOSUB return position (the position after the ON statement) */
            const char *pos = lex_get_pos(lex);
            BppLineNumber line = vm_get_current_line(vm);
            if (!vm_gosub_push(vm, line, pos)) {
                err.code = 15; /* Call stack overflow */
                err.message = "Call stack overflow in computed GOSUB";
                return err;
            }
        }
        vm_jump(vm, target, NULL);
    }

    return err;
}

/* RESUME statement handler */
BppError stmt_resume_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm_is_in_error_handler(vm)) {
        err.code = 20; /* RESUME without error */
        err.message = "RESUME without error: not currently in an error handler";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_NEXT) {
        lex_next(lex); /* Consume NEXT */
        vm_set_in_error_handler(vm, false);
        vm_jump(vm, vm_get_error_occurred_line(vm), vm_get_error_next_pos(vm));
    } else if (tok.type == TOK_NUMBER) {
        lex_next(lex); /* Consume line number */
        double target = tok.as.number;
        vm_set_in_error_handler(vm, false);
        vm_jump(vm, target, NULL);
    } else {
        /* Standard RESUME: branch back to the statement that caused the error */
        vm_set_in_error_handler(vm, false);
        vm_jump(vm, vm_get_error_occurred_line(vm), vm_get_error_occurred_pos(vm));
    }

    return err;
}

const char *error_get_message(int code) {
    switch (code) {
        case 1: return "NEXT without FOR";
        case 2: return "Syntax error";
        case 3: return "RETURN without GOSUB";
        case 4: return "Out of DATA";
        case 5: return "Illegal function call";
        case 6: return "Overflow";
        case 7: return "Out of memory";
        case 8: return "Undefined line number";
        case 9: return "Subscript out of range";
        case 10: return "Duplicate definition";
        case 11: return "Division by zero";
        case 12: return "Illegal direct";
        case 13: return "Type mismatch";
        case 14: return "Out of string space";
        case 16: return "String formula too complex";
        case 17: return "Cannot continue";
        case 18: return "Undefined user function";
        case 19: return "No RESUME";
        case 20: return "RESUME without error";
        case 24: return "Device timeout";
        case 25: return "Device Fault";
        case 27: return "Out of paper";
        case 35: return "Subprogram not defined";
        case 50: return "FIELD overflow";
        case 51: return "Internal error";
        case 52: return "Bad file name or number";
        case 53: return "File not found";
        case 54: return "Bad file mode";
        case 55: return "File already open";
        case 57: return "Device I/O error";
        case 58: return "File already exists";
        case 61: return "Disk full";
        case 62: return "Input past end of file";
        case 63: return "Bad record number";
        case 64: return "Bad file name";
        case 67: return "Too many files";
        case 70: return "Permission denied";
        case 71: return "Disk not ready";
        case 72: return "Disk media error";
        case 74: return "Rename across disks";
        case 75: return "Path/File access error";
        case 76: return "Path not found";
        default: return "User raised error";
    }
}

BppError stmt_error_statement_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "ERROR expects a numeric error code";
        return err;
    }

    int err_code = (int)val.as.number;
    err.code = err_code;
    err.message = error_get_message(err_code);
    return err;
}

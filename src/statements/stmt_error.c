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

#include "bpp_stmt.h"
#include "bpp_eval.h"
#include <string.h>

/* ON statement handler (Computed ON GOTO/GOSUB and ON ERROR GOTO) */
BppError stmt_on_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
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
        if (key_idx < 1 || key_idx > 10) {
            err.code = 5; err.message = "Illegal function call: Key index must be 1 to 10";
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

/**
 * @file stmt_loop.c
 * @brief Loop management statements command handlers (FOR, NEXT, WHILE, WEND, DO, LOOP).
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements loop statement handlers for the VM:
 *   - FOR: Initializes a loop variable, evaluates boundaries/steps, and pushes a loop frame.
 *     If boundary checks fail initially, it scans forward to skip the loop body to the matching NEXT.
 *   - NEXT: Increments the loop variable, checks limits, and branches back to the loop body if continuing,
 *     otherwise pops the frame.
 *   - WHILE/WEND: Loop entry and end statements checking boolean conditional expressions.
 *   - DO/LOOP: Loops checking conditions at either DO or LOOP boundary using WHILE or UNTIL qualifiers.
 * - Why it exists: Provides core language iteration control capabilities.
 * - Why it works this way: It interacts directly with the ForStack, WhileStack, and DoStack context frames
 *   held inside the VMContext. This ensures the loop state resides in the VM heap, satisfying the C-stack-independent execution rule.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Pre-scan logic, default STEP values, loop termination condition checks.
 * - What cannot be changed: Memory clear guarantees on stack pops, case-insensitivity of variable name matching.
 * - What to expect: Entering invalid NEXT variable names will cause stack mismatches and emit runtime errors.
 * - What to do if something breaks: Trace active stack frames via logs or output active frame values.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Program lines are sequential and sorted in memory.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add REPEAT/UNTIL or exit command handlers (EXIT FOR, EXIT WHILE).
 * - How to write external extensions: Plugins query VM loop stack APIs to implement custom iteration steps.
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* Forward declare scan helpers */
static BppError skip_to_next(VMContext *vm, const char *var_name);
static BppError skip_to_wend(VMContext *vm);
static BppError skip_to_loop(VMContext *vm);

static BppError parse_loop_range(VMContext *vm, LexerContext *lex, double *out_start, double *out_end, double *out_step) {
    BppError err;
    memset(&err, 0, sizeof(err));
    
    BValue start_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (start_val.type == VAL_STRING) {
        err.code = 13;
        err.message = "String expression not allowed in loop range";
        return err;
    }
    *out_start = start_val.as.number;
    *out_end = *out_start;
    *out_step = 1.0;
    
    BppToken tok = lex_peek(lex);
    /* JOSS style: start(step)end */
    if (tok.type == TOK_LPAREN) {
        lex_next(lex); /* Consume '(' */
        BValue step_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (step_val.type == VAL_STRING) {
            err.code = 13;
            err.message = "String expression not allowed in loop step";
            return err;
        }
        *out_step = step_val.as.number;
        
        BppToken close_tok = lex_next(lex);
        if (close_tok.type != TOK_RPAREN) {
            err.code = 2;
            err.message = "Expected ')' after JOSS loop step";
            return err;
        }
        
        BValue end_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (end_val.type == VAL_STRING) {
            err.code = 13;
            err.message = "String expression not allowed in loop bounds";
            return err;
        }
        *out_end = end_val.as.number;
    }
    /* Standard TO */
    else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) {
        lex_next(lex); /* Consume TO */
        BValue end_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        *out_end = end_val.as.number;
        
        BppToken step_tok = lex_peek(lex);
        if (step_tok.type == TOK_KEYWORD && step_tok.as.keyword == KW_STEP) {
            lex_next(lex); /* Consume STEP */
            BValue step_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            *out_step = step_val.as.number;
        }
    }
    /* Reversed BY/STEP: BY STEP <step> TO <end> or STEP <step> TO <end> */
    else if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_BY || tok.as.keyword == KW_STEP)) {
        lex_next(lex); /* Consume BY or STEP */
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_STEP) {
            lex_next(lex); /* Consume STEP after BY */
        }
        BValue step_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        *out_step = step_val.as.number;
        
        BppToken to_tok = lex_next(lex);
        if (to_tok.type != TOK_KEYWORD || to_tok.as.keyword != KW_TO) {
            err.code = 2;
            err.message = "Expected TO keyword after step value";
            return err;
        }
        
        BValue end_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        *out_end = end_val.as.number;
    }
    
    return err;
}

/* FOR statement handler */
BppError stmt_for_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    
    char loop_vars[8][64];
    int loop_var_count = 0;
    
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2;
        err.message = "Expected variable name in FOR loop";
        return err;
    }
    
    size_t clen = (tok.length < 63) ? tok.length : 63;
    memcpy(loop_vars[loop_var_count], tok.start, clen);
    loop_vars[loop_var_count][clen] = '\0';
    loop_var_count++;
    
    BppToken next_tok = lex_peek(lex);
    while (next_tok.type == TOK_COMMA) {
        lex_next(lex); /* Consume comma */
        BppToken var_tok = lex_next(lex);
        if (var_tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected variable name after comma in FOR loop";
            return err;
        }
        if (loop_var_count >= 8) {
            err.code = 2;
            err.message = "Too many variables in FOR loop list";
            return err;
        }
        clen = (var_tok.length < 63) ? var_tok.length : 63;
        memcpy(loop_vars[loop_var_count], var_tok.start, clen);
        loop_vars[loop_var_count][clen] = '\0';
        loop_var_count++;
        next_tok = lex_peek(lex);
    }
    
    /* Expect '=' */
    tok = lex_next(lex);
    if (tok.type != TOK_EQ) {
        err.code = 2;
        err.message = "Expected '=' in FOR loop initialization";
        return err;
    }
    
    /* Parse the first range */
    double start_num = 0.0, end_num = 0.0, step_num = 1.0;
    err = parse_loop_range(vm, lex, &start_num, &end_num, &step_num);
    if (err.code != 0) return err;
    
    /* Check if there are trailing discrete ranges (comma-separated list after the first range) */
    const char *next_range_pos = NULL;
    BppToken comma_tok = lex_peek(lex);
    if (comma_tok.type == TOK_COMMA) {
        next_range_pos = comma_tok.start;
    }
    
    /* Consume all remaining discrete loop ranges so they are not executed as statements */
    BppToken skip_tok = lex_peek(lex);
    while (skip_tok.type != TOK_EOF && skip_tok.type != TOK_EOL) {
        lex_next(lex);
        skip_tok = lex_peek(lex);
    }
    
    VariableContext *var_ctx = vm_get_var(vm);
    BppLineNumber line = vm_get_current_line(vm);
    const char *body_pos = lex_get_pos(lex);
    
    bool skip_rest = false;
    for (int i = 0; i < loop_var_count; ++i) {
        BppForFrame active_frame;
        bool is_active = vm_for_peek(vm, loop_vars[i], &active_frame);
        if (is_active && active_frame.line == line) {
            /* Already active, skip initialization of this variable and proceed */
            continue;
        }
        
        BValue val;
        val.type = VAL_NUMBER;
        val.as.number = start_num;
        if (!var_assign(var_ctx, loop_vars[i], val)) {
            err.code = 13;
            err.message = "Failed to assign loop variable value";
            return err;
        }
        
        bool should_run = true;
        if (step_num >= 0.0) {
            if (start_num > end_num) should_run = false;
        } else {
            if (start_num < end_num) should_run = false;
        }
        
        if (should_run) {
            const char *ret_pos = (i == loop_var_count - 1) ? body_pos : lex_get_pos(lex);
            if (i < loop_var_count - 1) {
                ret_pos = vm_get_current_stmt_pos(vm);
            }
            
            if (!vm_for_push(vm, loop_vars[i], end_num, step_num, line, ret_pos)) {
                err.code = 15;
                err.message = "FOR loop nesting limit exceeded";
                return err;
            }
            
            if (i == loop_var_count - 1 && next_range_pos != NULL) {
                vm_for_update(vm, loop_vars[i], end_num, step_num, next_range_pos);
            }
        } else {
            err = skip_to_next(vm, loop_vars[i]);
            skip_rest = true;
            break;
        }
    }
    
    if (skip_rest) {
        return err;
    }
    
    return err;
}

/* NEXT statement handler */
BppError stmt_next_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    
    /* Support NEXT without variables: peek innermost frame and loop on its variable name */
    if (tok.type != TOK_IDENT) {
        BppForFrame frame;
        if (!vm_for_peek(vm, NULL, &frame)) {
            err.code = 1; /* NEXT without FOR */
            err.message = "NEXT without FOR: no active loops on stack";
            return err;
        }
        
        VariableContext *var_ctx = vm_get_var(vm);
        BValue *v = var_lookup(var_ctx, frame.var_name, false);
        if (!v) {
            err.code = 13;
            err.message = "Loop variable missing during loop increment";
            return err;
        }
        
        double current_num = v->as.number;
        current_num += frame.step;
        
        BValue val;
        val.type = VAL_NUMBER;
        val.as.number = current_num;
        var_assign(var_ctx, frame.var_name, val);
        
        bool should_continue = true;
        if (frame.step >= 0.0) {
            if (current_num > frame.target) should_continue = false;
        } else {
            if (current_num < frame.target) should_continue = false;
        }
        
        if (should_continue) {
            vm_jump(vm, frame.line, frame.pos);
            return err;
        } else {
            /* Check next_range_pos */
            if (frame.next_range_pos != NULL) {
                LexerContext *range_lex = lex_init(vm_get_mem(vm), frame.next_range_pos);
                if (range_lex) {
                    lex_next(range_lex); /* Consume comma */
                    double r_start = 0.0, r_end = 0.0, r_step = 1.0;
                    err = parse_loop_range(vm, range_lex, &r_start, &r_end, &r_step);
                    if (err.code == 0) {
                        const char *next_c_pos = NULL;
                        BppToken next_comma = lex_peek(range_lex);
                        if (next_comma.type == TOK_COMMA) {
                            next_c_pos = next_comma.start;
                        }
                        vm_for_update(vm, frame.var_name, r_end, r_step, next_c_pos);
                        val.as.number = r_start;
                        var_assign(var_ctx, frame.var_name, val);
                        vm_jump(vm, frame.line, frame.pos);
                        lex_shutdown(range_lex);
                        return err;
                    }
                    lex_shutdown(range_lex);
                }
            }
            
            /* Actually finish loop and pop it */
            vm_for_pop(vm, frame.var_name, NULL);
        }
        return err;
    }

    /* Support comma-separated variable list: NEXT C, B, A */
    while (tok.type == TOK_IDENT) {
        lex_next(lex); /* Consume identifier */
        char var_name[64];
        size_t clen = (tok.length < 63) ? tok.length : 63;
        memcpy(var_name, tok.start, clen);
        var_name[clen] = '\0';
        
        BppForFrame frame;
        if (!vm_for_peek(vm, var_name, &frame)) {
            err.code = 1;
            err.message = "NEXT without FOR: no active loop matches variable name";
            return err;
        }
        
        VariableContext *var_ctx = vm_get_var(vm);
        BValue *v = var_lookup(var_ctx, frame.var_name, false);
        if (!v) {
            err.code = 13;
            err.message = "Loop variable missing during loop increment";
            return err;
        }
        
        double current_num = v->as.number;
        current_num += frame.step;
        
        BValue val;
        val.type = VAL_NUMBER;
        val.as.number = current_num;
        var_assign(var_ctx, frame.var_name, val);
        
        bool should_continue = true;
        if (frame.step >= 0.0) {
            if (current_num > frame.target) should_continue = false;
        } else {
            if (current_num < frame.target) should_continue = false;
        }
        
        if (should_continue) {
            vm_jump(vm, frame.line, frame.pos);
            return err; /* Stop executing NEXT list, we jumped back! */
        } else {
            /* Check next_range_pos */
            if (frame.next_range_pos != NULL) {
                LexerContext *range_lex = lex_init(vm_get_mem(vm), frame.next_range_pos);
                if (range_lex) {
                    lex_next(range_lex); /* Consume comma */
                    double r_start = 0.0, r_end = 0.0, r_step = 1.0;
                    err = parse_loop_range(vm, range_lex, &r_start, &r_end, &r_step);
                    if (err.code == 0) {
                        const char *next_c_pos = NULL;
                        BppToken next_comma = lex_peek(range_lex);
                        if (next_comma.type == TOK_COMMA) {
                            next_c_pos = next_comma.start;
                        }
                        vm_for_update(vm, frame.var_name, r_end, r_step, next_c_pos);
                        val.as.number = r_start;
                        var_assign(var_ctx, frame.var_name, val);
                        vm_jump(vm, frame.line, frame.pos);
                        lex_shutdown(range_lex);
                        return err;
                    }
                    lex_shutdown(range_lex);
                }
            }
            
            /* Pop loop frame */
            vm_for_pop(vm, frame.var_name, NULL);
        }
        
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume comma */
            tok = lex_peek(lex);
        } else {
            break;
        }
    }
    
    return err;
}

/* WHILE statement handler */
BppError stmt_while_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Capture WHILE start position for looping back */
    BppLineNumber while_line = vm_get_current_line(vm);
    const char *while_pos = vm_get_current_stmt_pos(vm);

    /* Evaluate loop condition */
    BValue cond_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (cond_val.type == VAL_STRING) {
        err.code = 13;
        err.message = "String value not allowed as WHILE loop condition";
        return err;
    }

    bool is_true = (cond_val.as.number != 0.0);

    if (is_true) {
        /* Check if we are already inside this WHILE loop. If we aren't, push it. */
        BppLineNumber peek_line = 0.0;
        const char *peek_pos = NULL;
        bool exists = false;

        if (vm_while_peek(vm, &peek_line, &peek_pos)) {
            if (peek_line == while_line && peek_pos == while_pos) {
                exists = true;
            }
        }

        if (!exists) {
            if (!vm_while_push(vm, while_line, while_pos)) {
                err.code = 15; /* Stack overflow */
                err.message = "WHILE loop nesting limit exceeded";
            }
        }
    } else {
        /* Condition is false: pop if active, then skip loop body to WEND */
        BppLineNumber peek_line = 0.0;
        const char *peek_pos = NULL;
        if (vm_while_peek(vm, &peek_line, &peek_pos)) {
            if (peek_line == while_line && peek_pos == while_pos) {
                vm_while_pop(vm, NULL, NULL);
            }
        }
        err = skip_to_wend(vm);
    }

    return err;
}

/* WEND statement handler */
BppError stmt_wend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    BppLineNumber while_line = 0.0;
    const char *while_pos = NULL;

    if (!vm_while_peek(vm, &while_line, &while_pos)) {
        err.code = 21; /* WEND without WHILE */
        err.message = "WEND without WHILE: no active WHILE loop frame found";
        return err;
    }

    /* Jump back to WHILE statement to re-evaluate condition */
    vm_jump(vm, while_line, while_pos);

    return err;
}

/* DO statement handler */
BppError stmt_do_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppLineNumber do_line = vm_get_current_line(vm);
    const char *do_pos = vm_get_current_stmt_pos(vm);

    BppToken tok = lex_peek(lex);
    bool check_condition = false;
    bool is_while = false;
    bool cond_true = true;

    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_WHILE) {
            lex_next(lex);
            check_condition = true;
            is_while = true;
        } else if (tok.as.keyword == KW_UNTIL) {
            lex_next(lex);
            check_condition = true;
            is_while = false;
        }
    }

    if (check_condition) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING) {
            err.code = 13;
            err.message = "String values are not allowed as loop conditions";
            return err;
        }
        bool truth = (val.as.number != 0.0);
        cond_true = is_while ? truth : !truth;
    }

    if (cond_true) {
        /* Check if we need to push this loop frame */
        BppLineNumber peek_line = 0.0;
        const char *peek_pos = NULL;
        bool exists = false;
        if (vm_do_peek(vm, &peek_line, &peek_pos)) {
            if (peek_line == do_line && peek_pos == do_pos) {
                exists = true;
            }
        }
        if (!exists) {
            if (!vm_do_push(vm, do_line, do_pos)) {
                err.code = 15;
                err.message = "DO loop nesting limit exceeded";
            }
        }
    } else {
        /* Pop if active and skip loop body */
        BppLineNumber peek_line = 0.0;
        const char *peek_pos = NULL;
        if (vm_do_peek(vm, &peek_line, &peek_pos)) {
            if (peek_line == do_line && peek_pos == do_pos) {
                vm_do_pop(vm, NULL, NULL);
            }
        }
        err = skip_to_loop(vm);
    }

    return err;
}

/* LOOP statement handler */
BppError stmt_loop_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppLineNumber do_line = 0.0;
    const char *do_pos = NULL;

    if (!vm_do_peek(vm, &do_line, &do_pos)) {
        err.code = 22; /* LOOP without DO */
        err.message = "LOOP without DO: no active DO loop frame found";
        return err;
    }

    BppToken tok = lex_peek(lex);
    bool check_condition = false;
    bool is_while = false;
    bool cond_true = true;

    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_WHILE) {
            lex_next(lex);
            check_condition = true;
            is_while = true;
        } else if (tok.as.keyword == KW_UNTIL) {
            lex_next(lex);
            check_condition = true;
            is_while = false;
        }
    }

    if (check_condition) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING) {
            err.code = 13;
            err.message = "String values are not allowed as loop conditions";
            return err;
        }
        bool truth = (val.as.number != 0.0);
        cond_true = is_while ? truth : !truth;
    }

    if (cond_true) {
        /* Loop continuing: jump back to DO position */
        vm_jump(vm, do_line, do_pos);
    } else {
        /* Loop complete: pop DO frame from stack */
        vm_do_pop(vm, NULL, NULL);
    }

    return err;
}

/* --- Loop Skipping Scan Helpers --- */

static BppError skip_to_next(VMContext *vm, const char *var_name) {
    BppError err;
    memset(&err, 0, sizeof(err));

    MemoryContext *mem = vm_get_mem(vm);
    BppLineNumber cur_line = vm_get_current_line(vm);

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    size_t start_idx = 0;
    bool found_cur = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_line) {
            start_idx = i;
            found_cur = true;
            break;
        }
    }

    if (!found_cur) {
        err.code = 11;
        err.message = "Execution state corruption during loop scan";
        return err;
    }

    int nested_depth = 0;

    for (size_t i = start_idx; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) {
            err.code = 14;
            err.message = "Out of memory during loop scan";
            return err;
        }

        BppToken tok = lex_next(scan_lex);
        while (tok.type != TOK_EOF) {
            if (tok.type == TOK_KEYWORD) {
                if (tok.as.keyword == KW_FOR) {
                    BppToken vtok = lex_next(scan_lex);
                    if (vtok.type == TOK_IDENT) {
                        char vname[64];
                        size_t clen = (vtok.length < 63) ? vtok.length : 63;
                        memcpy(vname, vtok.start, clen);
                        vname[clen] = '\0';
                        if (strcmp(vname, var_name) == 0) {
                            nested_depth++;
                        }
                    }
                } else if (tok.as.keyword == KW_NEXT) {
                    BppToken vtok = lex_next(scan_lex);
                    bool var_matches = true;
                    if (vtok.type == TOK_IDENT) {
                        char vname[64];
                        size_t clen = (vtok.length < 63) ? vtok.length : 63;
                        memcpy(vname, vtok.start, clen);
                        vname[clen] = '\0';
                        if (strcmp(vname, var_name) != 0) {
                            var_matches = false;
                        }
                    }
                    if (var_matches) {
                        nested_depth--;
                        if (nested_depth == 0) {
                            tok = lex_peek(scan_lex);
                            const char *pos = lex_get_pos(scan_lex);
                            lex_shutdown(scan_lex);
                            vm_jump(vm, lines[i].line_number, pos);
                            return err;
                        }
                    }
                }
            }
            tok = lex_next(scan_lex);
        }
        lex_shutdown(scan_lex);
    }

    err.code = 1;
    err.message = "FOR without NEXT: matching loop variable not found";
    return err;
}

static BppError skip_to_wend(VMContext *vm) {
    BppError err;
    memset(&err, 0, sizeof(err));

    MemoryContext *mem = vm_get_mem(vm);
    BppLineNumber cur_line = vm_get_current_line(vm);

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    size_t start_idx = 0;
    bool found_cur = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_line) {
            start_idx = i;
            found_cur = true;
            break;
        }
    }

    if (!found_cur) {
        err.code = 11;
        err.message = "Execution state corruption during loop scan";
        return err;
    }

    int nested_depth = 0;

    for (size_t i = start_idx; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) {
            err.code = 14;
            err.message = "Out of memory during loop scan";
            return err;
        }

        BppToken tok = lex_next(scan_lex);
        while (tok.type != TOK_EOF) {
            if (tok.type == TOK_KEYWORD) {
                if (tok.as.keyword == KW_WHILE) {
                    nested_depth++;
                } else if (tok.as.keyword == KW_WEND) {
                    nested_depth--;
                    if (nested_depth == 0) {
                        tok = lex_peek(scan_lex);
                        const char *pos = lex_get_pos(scan_lex);
                        lex_shutdown(scan_lex);
                        vm_jump(vm, lines[i].line_number, pos);
                        return err;
                    }
                }
            }
            tok = lex_next(scan_lex);
        }
        lex_shutdown(scan_lex);
    }

    err.code = 2;
    err.message = "WHILE without WEND: matching WEND not found";
    return err;
}

static BppError skip_to_loop(VMContext *vm) {
    BppError err;
    memset(&err, 0, sizeof(err));

    MemoryContext *mem = vm_get_mem(vm);
    BppLineNumber cur_line = vm_get_current_line(vm);

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    size_t start_idx = 0;
    bool found_cur = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_line) {
            start_idx = i;
            found_cur = true;
            break;
        }
    }

    if (!found_cur) {
        err.code = 11;
        err.message = "Execution state corruption during loop scan";
        return err;
    }

    int nested_depth = 0;

    for (size_t i = start_idx; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) {
            err.code = 14;
            err.message = "Out of memory during loop scan";
            return err;
        }

        BppToken tok = lex_next(scan_lex);
        while (tok.type != TOK_EOF) {
            if (tok.type == TOK_KEYWORD) {
                if (tok.as.keyword == KW_DO) {
                    nested_depth++;
                } else if (tok.as.keyword == KW_LOOP) {
                    nested_depth--;
                    if (nested_depth == 0) {
                        tok = lex_peek(scan_lex);
                        const char *pos = lex_get_pos(scan_lex);
                        lex_shutdown(scan_lex);
                        vm_jump(vm, lines[i].line_number, pos);
                        return err;
                    }
                }
            }
            tok = lex_next(scan_lex);
        }
        lex_shutdown(scan_lex);
    }

    err.code = 2;
    err.message = "DO without LOOP: matching LOOP not found";
    return err;
}

BppError stmt_exit_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Syntax error: expected FOR, WHILE, DO, SUB, or FUNCTION after EXIT";
        return err;
    }

    if (tok.as.keyword == KW_FOR) {
        BppForFrame frame;
        memset(&frame, 0, sizeof(frame));
        if (!vm_for_pop(vm, NULL, &frame)) {
            err.code = 1; err.message = "EXIT FOR without active FOR loop";
            return err;
        }
        BppLineNumber prev_line = vm_get_current_line(vm);
        vm_set_current_line(vm, frame.line);
        err = skip_to_next(vm, frame.var_name);
        if (err.code != 0) {
            vm_set_current_line(vm, prev_line);
        }
        return err;
    }
    else if (tok.as.keyword == KW_WHILE) {
        BppLineNumber while_line = 0.0;
        const char *while_pos = NULL;
        if (!vm_while_pop(vm, &while_line, &while_pos)) {
            err.code = 1; err.message = "EXIT WHILE without active WHILE loop";
            return err;
        }
        BppLineNumber prev_line = vm_get_current_line(vm);
        vm_set_current_line(vm, while_line);
        err = skip_to_wend(vm);
        if (err.code != 0) {
            vm_set_current_line(vm, prev_line);
        }
        return err;
    }
    else if (tok.as.keyword == KW_DO) {
        BppLineNumber do_line = 0.0;
        const char *do_pos = NULL;
        if (!vm_do_pop(vm, &do_line, &do_pos)) {
            err.code = 1; err.message = "EXIT DO without active DO loop";
            return err;
        }
        BppLineNumber prev_line = vm_get_current_line(vm);
        vm_set_current_line(vm, do_line);
        err = skip_to_loop(vm);
        if (err.code != 0) {
            vm_set_current_line(vm, prev_line);
        }
        return err;
    }
    else if (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION) {
        BppLineNumber resume_line = 0.0;
        const char *resume_pos = NULL;
        if (!vm_gosub_pop(vm, &resume_line, &resume_pos)) {
            err.code = 3; err.message = "EXIT SUB/FUNCTION outside active call";
            return err;
        }
        vm_jump(vm, resume_line, resume_pos);
        vm_clear_event_handlers(vm);
        return err;
    }

    err.code = 2; err.message = "Syntax error: expected FOR, WHILE, DO, SUB, or FUNCTION after EXIT";
    return err;
}

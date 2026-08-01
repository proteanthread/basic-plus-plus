/**
 * @file stmt_select.c
 * @brief SELECT CASE control flow statement handler.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements SELECT CASE / CASE / CASE ELSE / END SELECT block structure.
 * - Why it exists: Provides structured multi-way branching without cluttered nested IF statements.
 * - Why it works this way: It evaluates the select expression, pushes it onto the select stack,
 *   and matches case conditions (ranges with TO, comparisons with IS, or single values).
 *   Unmatched case blocks are skipped by scanning line tokens to the next CASE or END SELECT.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Range matching bounds or type conversion checks.
 * - What cannot be changed: Nested stack boundaries tracking and match states updates.
 * - What to expect: Entering a nested SELECT CASE adds a frame to the select stack.
 * - What to do if something breaks: Trace SELECT nesting levels and check stack overflow states.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Program statements are stored sequentially.
 * - Portability concerns: None. C17 standard compliant.
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "vm/vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Helper to compare two BValue objects for equality.
 */
static bool values_equal(BValue v1, BValue v2) {
    if (v1.type != v2.type) return false;
    if (v1.type == VAL_STRING) {
        return strcmp(str_data(v1.as.string), str_data(v2.as.string)) == 0;
    }
    return v1.as.number == v2.as.number;
}

/**
 * @brief Helper to check if v1 <= v2.
 */
static bool value_less_or_equal(BValue v1, BValue v2) {
    if (v1.type == VAL_STRING && v2.type == VAL_STRING) {
        return strcmp(str_data(v1.as.string), str_data(v2.as.string)) <= 0;
    }
    if (v1.type != VAL_STRING && v2.type != VAL_STRING) {
        return v1.as.number <= v2.as.number;
    }
    return false;
}

/**
 * @brief Helper to check if v1 >= v2.
 */
static bool value_greater_or_equal(BValue v1, BValue v2) {
    if (v1.type == VAL_STRING && v2.type == VAL_STRING) {
        return strcmp(str_data(v1.as.string), str_data(v2.as.string)) >= 0;
    }
    if (v1.type != VAL_STRING && v2.type != VAL_STRING) {
        return v1.as.number >= v2.as.number;
    }
    return false;
}

/**
 * @brief Skips lines until a CASE, CASE ELSE, or END SELECT at the current nesting level is found.
 */
static BppError skip_to_next_case_or_end_select(VMContext *vm) {
    BppError err;
    memset(&err, 0, sizeof(err));

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    BppLineNumber cur_ln = vm_get_current_line(vm);
    size_t start_idx = 0;
    bool found = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) {
            start_idx = i;
            found = true;
            break;
        }
    }

    if (!found) {
        err.code = 11; err.message = "Execution state corruption during SELECT scan";
        return err;
    }

    int select_nesting = 0;

    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        BppToken tok = lex_next(scan_lex);

        if (tok.type == TOK_KEYWORD) {
            if (tok.as.keyword == KW_SELECT) {
                select_nesting++;
            } else if (tok.as.keyword == KW_END) {
                BppToken next_tok = lex_next(scan_lex);
                if (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_SELECT) {
                    if (select_nesting > 0) {
                        select_nesting--;
                    } else {
                        vm_jump(vm, lines[i].line_number, lines[i].text);
                        lex_shutdown(scan_lex);
                        return err;
                    }
                }
            } else if (tok.as.keyword == KW_CASE) {
                if (select_nesting == 0) {
                    vm_jump(vm, lines[i].line_number, lines[i].text);
                    lex_shutdown(scan_lex);
                    return err;
                }
            }
        }
        lex_shutdown(scan_lex);
    }

    err.code = 2; err.message = "SELECT CASE without END SELECT";
    return err;
}

/**
 * @brief SELECT CASE expression
 */
BppError stmt_select_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Expect CASE keyword next */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_CASE) {
        err.code = 2; err.message = "Expected CASE after SELECT";
        return err;
    }

    /* Evaluate select expression */
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    /* Push onto SELECT stack */
    if (!vm_select_push(vm, val, false, vm_get_current_line(vm), lex_get_pos(lex))) {
        err.code = 7; err.message = "SELECT CASE stack overflow";
        if (val.type == VAL_STRING) str_release(vm_get_str(vm), val.as.string);
        return err;
    }

    /* Release temporary string ownership to stack frame */
    if (val.type == VAL_STRING) {
        str_release(vm_get_str(vm), val.as.string);
    }

    return err;
}

/**
 * @brief CASE case_item1, case_item2, ... / CASE ELSE
 */
BppError stmt_case_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppSelectFrame frame;
    if (!vm_select_peek(vm, &frame)) {
        err.code = 2; err.message = "CASE without SELECT CASE";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE) {
        lex_next(lex); /* Consume ELSE */
        if (frame.matched) {
            /* Already matched a previous case, skip this block */
            return skip_to_next_case_or_end_select(vm);
        } else {
            /* Execute this CASE ELSE block */
            BppSelectFrame top_frame;
            vm_select_pop(vm, &top_frame);
            vm_select_push(vm, top_frame.val, true, top_frame.line, top_frame.pos);
            return err;
        }
    }

    if (frame.matched) {
        /* Already matched previously, skip this entire block */
        return skip_to_next_case_or_end_select(vm);
    }

    /* Evaluate case items list */
    bool match = false;
    while (true) {
        tok = lex_peek(lex);

        /* Case 1: IS operator expression (e.g. IS >= 5) */
        if (tok.type == TOK_IDENT) {
            char name[64] = {0};
            size_t len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
            memcpy(name, tok.start, len);
            for (size_t i = 0; i < len; i++) name[i] = (char)toupper((unsigned char)name[i]);

            if (strcmp(name, "IS") == 0) {
                lex_next(lex); /* Consume 'IS' */
                BppToken op_tok = lex_next(lex);
                BValue comp_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;

                bool cond = false;
                switch (op_tok.type) {
                    case TOK_EQ: cond = values_equal(frame.val, comp_val); break;
                    case TOK_NE: cond = !values_equal(frame.val, comp_val); break;
                    case TOK_LT: cond = !value_greater_or_equal(frame.val, comp_val); break;
                    case TOK_GT: cond = !value_less_or_equal(frame.val, comp_val); break;
                    case TOK_LE: cond = value_less_or_equal(frame.val, comp_val); break;
                    case TOK_GE: cond = value_greater_or_equal(frame.val, comp_val); break;
                    default:
                        err.code = 2; err.message = "Expected comparison operator after IS";
                        if (comp_val.type == VAL_STRING) str_release(vm_get_str(vm), comp_val.as.string);
                        return err;
                }

                if (comp_val.type == VAL_STRING) {
                    str_release(vm_get_str(vm), comp_val.as.string);
                }
                if (cond) match = true;
            } else {
                /* Treat as normal expression */
                BValue comp_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;

                /* Check TO range */
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_TO) {
                    lex_next(lex); /* Consume 'TO' */
                    BValue max_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) {
                        if (comp_val.type == VAL_STRING) str_release(vm_get_str(vm), comp_val.as.string);
                        return err;
                    }
                    if (value_greater_or_equal(frame.val, comp_val) && value_less_or_equal(frame.val, max_val)) {
                        match = true;
                    }
                    if (comp_val.type == VAL_STRING) str_release(vm_get_str(vm), comp_val.as.string);
                    if (max_val.type == VAL_STRING) str_release(vm_get_str(vm), max_val.as.string);
                } else {
                    if (values_equal(frame.val, comp_val)) match = true;
                    if (comp_val.type == VAL_STRING) str_release(vm_get_str(vm), comp_val.as.string);
                }
            }
        } else {
            /* Case 2: normal expression or expression TO expression */
            BValue comp_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;

            /* Check TO range */
            BppToken next_tok = lex_peek(lex);
            if (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_TO) {
                lex_next(lex); /* Consume 'TO' */
                BValue max_val = eval_expression(vm, lex, &err);
                if (err.code != 0) {
                    if (comp_val.type == VAL_STRING) str_release(vm_get_str(vm), comp_val.as.string);
                    return err;
                }
                if (value_greater_or_equal(frame.val, comp_val) && value_less_or_equal(frame.val, max_val)) {
                    match = true;
                }
                if (comp_val.type == VAL_STRING) str_release(vm_get_str(vm), comp_val.as.string);
                if (max_val.type == VAL_STRING) str_release(vm_get_str(vm), max_val.as.string);
            } else {
                if (values_equal(frame.val, comp_val)) match = true;
                if (comp_val.type == VAL_STRING) str_release(vm_get_str(vm), comp_val.as.string);
            }
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
        } else {
            break;
        }
    }

    if (match) {
        /* Execute this CASE block, mark matched */
        BppSelectFrame top_frame;
        vm_select_pop(vm, &top_frame);
        vm_select_push(vm, top_frame.val, true, top_frame.line, top_frame.pos);
        return err;
    } else {
        /* Skip to next case condition */
        return skip_to_next_case_or_end_select(vm);
    }
}

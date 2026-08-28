// FILENAME: select.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c)
// NEEDS: libcore (ctype.h, ctype.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, stmt.h, string.c, vm.h)
// Provides runtime implementation for the SELECT statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "vm/vm.h"
#include "runtime/micro_lib_metadata.h"

void stmt_select_register(void) {
    MicroLibMetadata meta = {
        .name = "SELECT",
        .category = "Control Flow",
        .syntax = "SELECT CASE test_expression ... CASE expression_list ... END SELECT",
        .help_text = "Executes one of several blocks of statements depending on the value of an expression.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch, Error 37: SELECT Without CASE"
    };
    microlib_register(&meta);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static bool is_numeric_type(ValueType t) {
    return (t == VAL_NUMBER || t == VAL_INTEGER);
}

// @brief Helper to compare two BValue objects for equality.
static bool values_equal(BValue v1, BValue v2) {
    if (v1.type == VAL_STRING && v2.type == VAL_STRING) {
        return (v1.as.string && v2.as.string) ? (strcmp(str_data(v1.as.string), str_data(v2.as.string)) == 0) : (v1.as.string == v2.as.string);
    }
    if (is_numeric_type(v1.type) && is_numeric_type(v2.type)) {
        return v1.as.number == v2.as.number;
    }
    return false;
}

// @brief Helper to check if v1 <= v2.
static bool value_less_or_equal(BValue v1, BValue v2) {
    if (v1.type == VAL_STRING && v2.type == VAL_STRING) {
        return (v1.as.string && v2.as.string) ? (strcmp(str_data(v1.as.string), str_data(v2.as.string)) <= 0) : false;
    }
    if (is_numeric_type(v1.type) && is_numeric_type(v2.type)) {
        return v1.as.number <= v2.as.number;
    }
    return false;
}

// @brief Helper to check if v1 >= v2.
static bool value_greater_or_equal(BValue v1, BValue v2) {
    if (v1.type == VAL_STRING && v2.type == VAL_STRING) {
        return (v1.as.string && v2.as.string) ? (strcmp(str_data(v1.as.string), str_data(v2.as.string)) >= 0) : false;
    }
    if (is_numeric_type(v1.type) && is_numeric_type(v2.type)) {
        return v1.as.number >= v2.as.number;
    }
    return false;
}

// @brief Skips lines until a CASE, CASE ELSE, or END SELECT at the current nesting level is found.
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
        if (!scan_lex) continue;

        while (true) {
            BppToken tok = lex_peek(scan_lex);
            if (tok.type == TOK_EOF) break;

            if (tok.type == TOK_EOL) {
                lex_next(scan_lex);
                continue;
            }

            bool is_select = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_SELECT) ||
                              (tok.type == TOK_IDENT && tok.length == 6 && strncasecmp(tok.start, "SELECT", 6) == 0);
            bool is_end    = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) ||
                              (tok.type == TOK_IDENT && tok.length == 3 && strncasecmp(tok.start, "END", 3) == 0);
            bool is_case   = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_CASE) ||
                              (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "CASE", 4) == 0);

            if (is_select) {
                select_nesting++;
                lex_next(scan_lex);
            } else if (is_end) {
                const char *stmt_start = tok.start;
                lex_next(scan_lex); // Consume END
                BppToken next_tok = lex_peek(scan_lex);
                bool next_is_select = (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_SELECT) ||
                                       (next_tok.type == TOK_IDENT && next_tok.length == 6 && strncasecmp(next_tok.start, "SELECT", 6) == 0);
                if (next_is_select) {
                    if (select_nesting > 0) {
                        select_nesting--;
                        lex_next(scan_lex);
                    } else {
                        ptrdiff_t offset = stmt_start - lines[i].text;
                        if (offset < 0) offset = 0;
                        vm_jump(vm, lines[i].line_number, lines[i].text + offset);
                        lex_shutdown(scan_lex);
                        return err;
                    }
                }
            } else if (is_case) {
                if (select_nesting == 0) {
                    ptrdiff_t offset = tok.start - lines[i].text;
                    if (offset < 0) offset = 0;
                    vm_jump(vm, lines[i].line_number, lines[i].text + offset);
                    lex_shutdown(scan_lex);
                    return err;
                }
                lex_next(scan_lex);
            } else {
                lex_next(scan_lex);
            }

            // Advance scan_lex to next statement separator (TOK_EOL or TOK_EOF)
            while (true) {
                BppToken skip_tok = lex_peek(scan_lex);
                if (skip_tok.type == TOK_EOF || skip_tok.type == TOK_EOL) break;
                lex_next(scan_lex);
            }
        }
        lex_shutdown(scan_lex);
    }

    err.code = 2; err.message = "SELECT CASE without END SELECT";
    return err;
}

// @brief SELECT CASE expression
BppError stmt_select_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    // Expect CASE keyword next
    BppToken tok = lex_next(lex);
    bool is_case = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_CASE) ||
                   (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "CASE", 4) == 0);
    if (!is_case) {
        err.code = 2; err.message = "Expected CASE after SELECT";
        return err;
    }

    // Evaluate select expression
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    // Push onto SELECT stack
    if (!vm_select_push(vm, val, false, vm_get_current_line(vm), lex_get_pos(lex))) {
        err.code = 7; err.message = "SELECT CASE stack overflow";
        if (val.type == VAL_STRING) str_release(vm_get_str(vm), val.as.string);
        return err;
    }

    // Release temporary string ownership to stack frame
    if (val.type == VAL_STRING) {
        str_release(vm_get_str(vm), val.as.string);
    }

    // Jump to first CASE or END SELECT statement
    return skip_to_next_case_or_end_select(vm);
}

// @brief CASE case_item1, case_item2, ... / CASE ELSE
BppError stmt_case_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppSelectFrame frame;
    if (!vm_select_peek(vm, &frame)) {
        err.code = 2; err.message = "CASE without SELECT CASE";
        return err;
    }

    BppToken tok = lex_peek(lex);
    bool is_else = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE) ||
                   (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "ELSE", 4) == 0);

    if (is_else) {
        lex_next(lex); // Consume ELSE
        if (frame.matched) {
            // Already matched a previous case, skip this block
            return skip_to_next_case_or_end_select(vm);
        } else {
            // Execute this CASE ELSE block
            BppSelectFrame top_frame;
            vm_select_pop(vm, &top_frame);
            vm_select_push(vm, top_frame.val, true, top_frame.line, top_frame.pos);
            return err;
        }
    }

    if (frame.matched) {
        // Already matched previously, skip this entire block
        return skip_to_next_case_or_end_select(vm);
    }

    // Evaluate case items list
    bool match = false;
    while (true) {
        tok = lex_peek(lex);

        bool is_is = (tok.type == TOK_IDENT && tok.length == 2 && strncasecmp(tok.start, "IS", 2) == 0);

        // Case 1: IS operator expression (e.g. IS >= 5)
        if (is_is) {
            lex_next(lex); // Consume 'IS'
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
            // Case 2: normal expression or expression TO expression
            BValue comp_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;

            // Check TO range
            BppToken next_tok = lex_peek(lex);
            bool is_to = (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_TO) ||
                         (next_tok.type == TOK_IDENT && next_tok.length == 2 && strncasecmp(next_tok.start, "TO", 2) == 0);

            if (is_to) {
                lex_next(lex); // Consume 'TO'
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
            lex_next(lex); // Consume ','
        } else {
            break;
        }
    }

    if (match) {
        // Execute this CASE block, mark matched
        BppSelectFrame top_frame;
        vm_select_pop(vm, &top_frame);
        vm_select_push(vm, top_frame.val, true, top_frame.line, top_frame.pos);
        return err;
    } else {
        // Skip to next case condition
        return skip_to_next_case_or_end_select(vm);
    }
}

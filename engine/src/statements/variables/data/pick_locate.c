// FILENAME: pick_locate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, strings.h, string.h, memory.h)
// NEEDS: libengine (eval.h, lexer.h, vm.h, pick_locate.h)
// Provides runtime implementation for MultiValue dynamic array locate and find operations.
//
// ---- Includes ----

#include "statements/variables/data/pick_locate.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/variables.h"
#include "runtime/conv/float_parse.h"

static inline bool is_tok_then(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_THEN) ||
           (tok.type == TOK_IDENT && tok.length == 4 && runtime_strncasecmp(tok.start, "THEN", 4) == 0);
}

static inline bool is_tok_else(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE) ||
           (tok.type == TOK_IDENT && tok.length == 4 && runtime_strncasecmp(tok.start, "ELSE", 4) == 0);
}

static inline bool is_tok_goto(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOTO) ||
           (tok.type == TOK_IDENT && tok.length == 4 && runtime_strncasecmp(tok.start, "GOTO", 4) == 0);
}

static BppError dispatch_branch(VMContext *vm, LexerContext *lex) {
    extern BppError execute_single_statement(VMContext *vm, LexerContext *lex);
    return execute_single_statement(vm, lex);
}

// MultiValue dynamic array search algorithm
bool pick_locate_search(const char *target, const char *dynarray, int *out_pos, const char *order) {
    if (!out_pos) return false;
    *out_pos = 1;

    if (!target) target = "";
    if (!dynarray || *dynarray == '\0') {
        *out_pos = 1;
        return false;
    }

    // Determine attribute delimiter: standard Pick 0xFE (\xfe) or fallback caret '^'
    char delim = '^';
    if (runtime_strchr(dynarray, '\xfe')) {
        delim = '\xfe';
    }

    bool is_ar = (order && (runtime_strcasecmp(order, "AR") == 0 || runtime_strcasecmp(order, "A") == 0));
    bool is_al = (order && runtime_strcasecmp(order, "AL") == 0);
    bool is_dr = (order && runtime_strcasecmp(order, "DR") == 0);
    bool is_dl = (order && (runtime_strcasecmp(order, "DL") == 0 || runtime_strcasecmp(order, "D") == 0));

    double target_num = 0.0;
    if (is_ar || is_dr) {
        target_num = runtime_atof(target);
    }

    const char *p = dynarray;
    int idx = 1;

    while (*p) {
        const char *next = runtime_strchr(p, delim);
        size_t len = next ? (size_t)(next - p) : runtime_strlen(p);

        char item_buf[512];
        const char *item = p;
        if (next) {
            size_t copy_len = len < sizeof(item_buf) - 1 ? len : sizeof(item_buf) - 1;
            runtime_memcpy(item_buf, p, copy_len);
            item_buf[copy_len] = '\0';
            item = item_buf;
        }

        // Exact equality check
        if (runtime_strcmp(target, item) == 0) {
            *out_pos = idx;
            return true;
        }

        // Sorted ordering checks
        if (is_al) {
            if (runtime_strcmp(target, item) < 0) {
                *out_pos = idx;
                return false;
            }
        } else if (is_ar) {
            double item_num = runtime_atof(item);
            if (target_num < item_num) {
                *out_pos = idx;
                return false;
            }
        } else if (is_dl) {
            if (runtime_strcmp(target, item) > 0) {
                *out_pos = idx;
                return false;
            }
        } else if (is_dr) {
            double item_num = runtime_atof(item);
            if (target_num > item_num) {
                *out_pos = idx;
                return false;
            }
        }

        if (!next) break;
        p = next + 1;
        idx++;
    }

    *out_pos = idx + (*p ? 1 : 0);
    return false;
}

bool pick_locate_has_in_clause(LexerContext *lex, const char **out_in_pos) {
    if (!lex) return false;
    LexerContext *look = lex_init(NULL, lex_get_pos(lex));
    if (!look) return false;
    int parens = 0;
    bool found = false;
    while (true) {
        BppToken t = lex_peek(look);
        if (t.type == TOK_EOF || t.type == TOK_EOL) break;
        if (t.type == TOK_LPAREN || t.type == TOK_LBRACKET || t.type == TOK_LBRACE) {
            parens++;
        } else if (t.type == TOK_RPAREN || t.type == TOK_RBRACKET || t.type == TOK_RBRACE) {
            if (parens > 0) parens--;
        } else if (parens == 0) {
            bool is_in = (t.type == TOK_IN) ||
                         (t.type == TOK_KEYWORD && t.as.keyword == KW_IN) ||
                         (t.type == TOK_IDENT && t.length == 2 && runtime_strncasecmp(t.start, "IN", 2) == 0);
            if (is_in) {
                if (out_in_pos) *out_in_pos = t.start;
                found = true;
                break;
            }
        }
        lex_next(look);
    }
    lex_shutdown(look);
    return found;
}

// Unified statement executor for LOCATE ... IN, FIND ... IN, and ARRAY FIND ... IN
BppError stmt_pick_locate_execute(VMContext *vm, LexerContext *lex, BValue target) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    // Convert target to string
    char target_buf[512] = "";
    if (target.type == VAL_STRING && target.as.string) {
        const char *t_data = str_data(target.as.string);
        size_t t_len = str_len(target.as.string);
        size_t c_len = t_len < sizeof(target_buf) - 1 ? t_len : sizeof(target_buf) - 1;
        runtime_memcpy(target_buf, t_data, c_len);
        target_buf[c_len] = '\0';
        str_release(vm_get_str(vm), target.as.string);
    } else if (target.type == VAL_NUMBER || target.type == VAL_INTEGER) {
        runtime_snprintf(target_buf, sizeof(target_buf), "%.14g", target.as.number);
    }

    // 1. Consume IN keyword
    BppToken in_tok = lex_next(lex);
    (void)in_tok;

    // 2. Parse dynarray expression
    BValue dyn_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    char num_dyn_buf[64] = "";
    const char *dyn_str = "";
    if (dyn_val.type == VAL_STRING && dyn_val.as.string) {
        dyn_str = str_data(dyn_val.as.string);
    } else if (dyn_val.type == VAL_NUMBER || dyn_val.type == VAL_INTEGER) {
        runtime_snprintf(num_dyn_buf, sizeof(num_dyn_buf), "%.14g", dyn_val.as.number);
        dyn_str = num_dyn_buf;
    }

    // 3. Optional SETTING pos_var and/or BY order_expr (in any order)
    char pos_var_name[128] = "";
    char order_str[16] = "";
    BppToken peek;

    while (true) {
        peek = lex_peek(lex);
        bool is_setting = (peek.type == TOK_IDENT && peek.length == 7 && runtime_strncasecmp(peek.start, "SETTING", 7) == 0) ||
                          (peek.type == TOK_KEYWORD && peek.length == 7 && runtime_strncasecmp(peek.start, "SETTING", 7) == 0);
        bool is_by = (peek.type == TOK_KEYWORD && peek.as.keyword == KW_BY) ||
                     (peek.type == TOK_IDENT && peek.length == 2 && runtime_strncasecmp(peek.start, "BY", 2) == 0);

        if (is_setting) {
            lex_next(lex); // consume SETTING
            BppToken var_tok = lex_next(lex);
            if (var_tok.type != TOK_IDENT && var_tok.type != TOK_KEYWORD) {
                if (dyn_val.type == VAL_STRING && dyn_val.as.string) str_release(vm_get_str(vm), dyn_val.as.string);
                err.code = 2; err.message = "Expected variable name after SETTING";
                return err;
            }
            size_t vlen = var_tok.length < sizeof(pos_var_name) - 1 ? var_tok.length : sizeof(pos_var_name) - 1;
            runtime_memcpy(pos_var_name, var_tok.start, vlen);
            pos_var_name[vlen] = '\0';
        } else if (is_by) {
            lex_next(lex); // consume BY
            BValue order_val = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                if (dyn_val.type == VAL_STRING && dyn_val.as.string) str_release(vm_get_str(vm), dyn_val.as.string);
                return err;
            }
            if (order_val.type == VAL_STRING && order_val.as.string) {
                const char *odata = str_data(order_val.as.string);
                size_t olen = str_len(order_val.as.string);
                size_t clen = olen < sizeof(order_str) - 1 ? olen : sizeof(order_str) - 1;
                runtime_memcpy(order_str, odata, clen);
                order_str[clen] = '\0';
                str_release(vm_get_str(vm), order_val.as.string);
            }
        } else {
            break;
        }
    }

    // 5. Execute MultiValue dynamic array locate
    int found_pos = 1;
    bool found = pick_locate_search(target_buf, dyn_str, &found_pos, order_str[0] ? order_str : NULL);

    if (dyn_val.type == VAL_STRING && dyn_val.as.string) {
        str_release(vm_get_str(vm), dyn_val.as.string);
    }

    // 6. Assign position variable if requested
    if (pos_var_name[0] != '\0') {
        BValue pos_val;
        pos_val.type = VAL_INTEGER;
        pos_val.as.number = (double)found_pos;
        var_assign(vm_get_var(vm), pos_var_name, pos_val);
    }

    // 7. Check for THEN and ELSE branch clauses
    peek = lex_peek(lex);
    if (is_tok_then(peek)) {
        lex_next(lex); // consume THEN
        if (found) {
            // Check if THEN is followed by a line number
            BppToken next = lex_peek(lex);
            if (next.type == TOK_NUMBER) {
                lex_next(lex);
                BppLineNumber target_line = (BppLineNumber)next.as.number;
                vm_jump(vm, target_line, NULL);
                return err;
            } else if (is_tok_goto(next)) {
                lex_next(lex);
                BppToken target_tok = lex_next(lex);
                if (target_tok.type == TOK_NUMBER) {
                    vm_jump(vm, (BppLineNumber)target_tok.as.number, NULL);
                    return err;
                }
            }

            // Execute statements in the THEN branch until ELSE or EOL
            while (true) {
                err = dispatch_branch(vm, lex);
                if (err.code != 0 || vm_is_jump_active(vm)) return err;
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_EOL && *next_tok.start == ':') {
                    lex_next(lex);
                    if (is_tok_else(lex_peek(lex))) break;
                } else {
                    break;
                }
            }

            // Skip trailing ELSE branch
            BppToken skip = lex_peek(lex);
            if (is_tok_else(skip)) {
                while (skip.type != TOK_EOF && (skip.type != TOK_EOL || *skip.start == ':')) {
                    lex_next(lex);
                    skip = lex_peek(lex);
                }
            }
        } else {
            // Not found: skip THEN branch to ELSE or EOL
            BppToken skip = lex_peek(lex);
            while (skip.type != TOK_EOF && (skip.type != TOK_EOL || *skip.start == ':')) {
                if (is_tok_else(skip)) break;
                lex_next(lex);
                skip = lex_peek(lex);
            }

            if (is_tok_else(skip)) {
                lex_next(lex); // consume ELSE
                BppToken next = lex_peek(lex);
                if (next.type == TOK_NUMBER) {
                    lex_next(lex);
                    vm_jump(vm, (BppLineNumber)next.as.number, NULL);
                    return err;
                } else if (is_tok_goto(next)) {
                    lex_next(lex);
                    BppToken target_tok = lex_next(lex);
                    if (target_tok.type == TOK_NUMBER) {
                        vm_jump(vm, (BppLineNumber)target_tok.as.number, NULL);
                        return err;
                    }
                }

                while (true) {
                    err = dispatch_branch(vm, lex);
                    if (err.code != 0 || vm_is_jump_active(vm)) return err;
                    BppToken next_tok = lex_peek(lex);
                    if (next_tok.type == TOK_EOL && *next_tok.start == ':') {
                        lex_next(lex);
                    } else {
                        break;
                    }
                }
            }
        }
    } else if (is_tok_else(peek)) {
        lex_next(lex); // consume ELSE
        if (!found) {
            BppToken next = lex_peek(lex);
            if (next.type == TOK_NUMBER) {
                lex_next(lex);
                vm_jump(vm, (BppLineNumber)next.as.number, NULL);
                return err;
            } else if (is_tok_goto(next)) {
                lex_next(lex);
                BppToken target_tok = lex_next(lex);
                if (target_tok.type == TOK_NUMBER) {
                    vm_jump(vm, (BppLineNumber)target_tok.as.number, NULL);
                    return err;
                }
            }

            while (true) {
                err = dispatch_branch(vm, lex);
                if (err.code != 0 || vm_is_jump_active(vm)) return err;
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_EOL && *next_tok.start == ':') {
                    lex_next(lex);
                } else {
                    break;
                }
            }
        } else {
            // Found: skip ELSE branch
            BppToken skip = lex_peek(lex);
            while (skip.type != TOK_EOF && (skip.type != TOK_EOL || *skip.start == ':')) {
                lex_next(lex);
                skip = lex_peek(lex);
            }
        }
    }

    return err;
}

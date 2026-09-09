// FILENAME: eval_set.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// NEEDS: libcore (arrays.h, map.h, strops.h), libengine (eval_expr_internal.h, set.h)
// Implements expression evaluation for Set/Group literals and braced group path addressing.

#include "eval/eval_expr_internal.h"
#include "eval/functions/string/manipulation/pick.h"
#include "runtime/set.h"
#include "runtime/arrays.h"
#include "runtime/map.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "lexer/lexer_internal.h"

// Parse a Group literal { key1: expr1, key2: expr2 }
static BValue parse_group_body(VMContext *vm, const char *str, BppError *err) {
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_GROUP;
    res.as.group = group_create(NULL);

    LexerContext *lex = lex_init(vm_get_mem(vm), str);
    if (!lex) {
        err->code = 14; err->message = "Out of memory initializing lexer";
        return res;
    }

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BppToken k_tok = lex_next(lex);
        char key_buf[64];
        if (k_tok.type == TOK_STRING) {
            size_t klen = (k_tok.length < sizeof(key_buf) - 1) ? k_tok.length : sizeof(key_buf) - 1;
            runtime_memcpy(key_buf, k_tok.as.string, klen);
            key_buf[klen] = '\0';
        } else if (k_tok.type == TOK_IDENT || k_tok.type == TOK_KEYWORD) {
            size_t klen = (k_tok.length < sizeof(key_buf) - 1) ? k_tok.length : sizeof(key_buf) - 1;
            runtime_memcpy(key_buf, k_tok.start, klen);
            key_buf[klen] = '\0';
        } else {
            break;
        }

        // Consume colon or equals
        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_EQ || (sep.type == TOK_EOL && sep.start && sep.start[0] == ':')) {
            lex_next(lex);
        }

        BValue val = eval_expression(vm, lex, err);
        if (err->code != 0) {
            lex_shutdown(lex);
            return res;
        }

        group_set(vm_get_str(vm), res.as.group, key_buf, val);

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    lex_shutdown(lex);
    return res;
}

// Parse a Set literal { expr1, expr2, ... }
static BValue parse_set_body(VMContext *vm, const char *str, BppError *err) {
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_SET;
    res.as.set = set_create(8, false);

    LexerContext *lex = lex_init(vm_get_mem(vm), str);
    if (!lex) {
        err->code = 14; err->message = "Out of memory initializing lexer";
        return res;
    }

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BValue val = eval_expression(vm, lex, err);
        if (err->code != 0) {
            lex_shutdown(lex);
            return res;
        }
        set_add(vm_get_str(vm), res.as.set, val);

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    lex_shutdown(lex);
    return res;
}

BValue eval_parse_braced_literal(VMContext *vm, const char *raw_str, BppError *out_err) {
    BValue res;
    runtime_memset(&res, 0, sizeof(res));

    // Skip leading whitespace
    const char *p = raw_str;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
    if (*p == '\0') {
        res.type = VAL_SET;
        res.as.set = set_create(0, false);
        return res;
    }

    // Inspect top-level delimiters
    bool has_colon = false;
    bool has_comma = false;
    int brace_depth = 0;
    bool in_str = false;

    for (const char *scan = p; *scan; ++scan) {
        if (*scan == '"') in_str = !in_str;
        else if (!in_str) {
            if (*scan == '{') brace_depth++;
            else if (*scan == '}') brace_depth--;
            else if (brace_depth == 0) {
                if (*scan == ':') has_colon = true;
                else if (*scan == ',') has_comma = true;
            }
        }
    }

    if (has_colon) {
        return parse_group_body(vm, p, out_err);
    }
    if (has_comma) {
        return parse_set_body(vm, p, out_err);
    }

    // Single item or RPN expression
    LexerContext *rpn_lex = lex_init(vm_get_mem(vm), p);
    if (!rpn_lex) {
        out_err->code = 14; out_err->message = "Out of memory";
        return res;
    }
    BValue rpn_res = eval_expression_rpn(vm, rpn_lex, out_err);
    lex_shutdown(rpn_lex);

    if (out_err->code == 0) {
        return rpn_res;
    }

    // Fallback: evaluate as single-element set { expr }
    out_err->code = 0;
    out_err->message = NULL;
    LexerContext *s_lex = lex_init(vm_get_mem(vm), p);
    BValue single_val = eval_expression(vm, s_lex, out_err);
    lex_shutdown(s_lex);

    if (out_err->code == 0) {
        res.type = VAL_SET;
        res.as.set = set_create(1, false);
        set_add(vm_get_str(vm), res.as.set, single_val);
        return res;
    }
    return res;
}

bool eval_parse_brace_access(VMContext *vm, LexerContext *lex, const char *name_buf, BValue *out_val, BppError *out_err) {
    if (!vm || !lex || !name_buf || !out_val || !out_err) return false;

    // Retrieve brace contents
    BppToken tok = lex_next(lex);
    char *body = NULL;
    if (tok.type == TOK_RPN_LITERAL) {
        body = (char *)mem_scratch_alloc(vm_get_mem(vm), tok.length + 1);
        if (!body) {
            out_err->code = 14; out_err->message = "Out of memory";
            return false;
        }
        runtime_memcpy(body, tok.as.string, tok.length);
        body[tok.length] = '\0';
    } else if (tok.type == TOK_LBRACE) {
        // Collect until closing brace
        return false;
    } else {
        return false;
    }

    // Check array unification first
    if (arr_exists(vm_get_arr(vm), name_buf)) {
        LexerContext *sub_lex = lex_init(vm_get_mem(vm), body);
        BValue idx_val = eval_expression(vm, sub_lex, out_err);
        lex_shutdown(sub_lex);
        if (out_err->code != 0) return false;
        int idx = (int)idx_val.as.number;
        return set_unify_array_get(vm, name_buf, idx, out_val, out_err);
    }

    // Look up variable
    BValue *var = var_lookup(vm_get_var(vm), name_buf, false);
    if (!var) {
        out_err->code = 9; out_err->message = "Variable not found for brace indexing";
        return false;
    }

    LexerContext *sub_lex = lex_init(vm_get_mem(vm), body);
    if (!sub_lex) return false;

    BValue curr = *var;
    while (lex_peek(sub_lex).type != TOK_EOF && lex_peek(sub_lex).type != TOK_EOL) {
        BppToken peek_t = lex_peek(sub_lex);
        if (curr.type == VAL_SET && curr.as.set && (peek_t.type == TOK_MUL || (peek_t.length == 1 && peek_t.start && peek_t.start[0] == '*'))) {
            lex_next(sub_lex); // Consume '*'
            if (lex_peek(sub_lex).type == TOK_COMMA) {
                lex_next(sub_lex); // Consume ','
            }

            BppToken first_tok = lex_peek(sub_lex);
            if (first_tok.type == TOK_STRING || first_tok.type == TOK_IDENT) {
                LexerContext checkpoint = *sub_lex;
                lex_next(sub_lex);
                if (lex_peek(sub_lex).type == TOK_EQ) {
                    lex_next(sub_lex); // Consume '='
                    char filter_k[64];
                    size_t flen = (first_tok.length < sizeof(filter_k) - 1) ? first_tok.length : sizeof(filter_k) - 1;
                    const char *src = (first_tok.type == TOK_STRING && first_tok.as.string) ? first_tok.as.string : first_tok.start;
                    runtime_memcpy(filter_k, src, flen);
                    filter_k[flen] = '\0';

                    BValue filter_val = eval_expression(vm, sub_lex, out_err);
                    if (out_err->code != 0) { lex_shutdown(sub_lex); return false; }
                    if (lex_peek(sub_lex).type == TOK_COMMA) {
                        lex_next(sub_lex); // Consume ','
                    }
                    BValue field_val = eval_expression(vm, sub_lex, out_err);
                    if (out_err->code != 0) { lex_shutdown(sub_lex); return false; }
                    const char *field_k = (field_val.type == VAL_STRING && field_val.as.string) ? str_data(field_val.as.string) : "";
                    BppSet *proj = set_project_filter(vm_get_str(vm), curr.as.set, filter_k, filter_val, field_k);
                    curr.type = VAL_SET;
                    curr.as.set = proj;
                    break;
                }
                *sub_lex = checkpoint;
            }

            BValue expr1 = eval_expression(vm, sub_lex, out_err);
            if (out_err->code != 0) { lex_shutdown(sub_lex); return false; }
            const char *field_k = (expr1.type == VAL_STRING && expr1.as.string) ? str_data(expr1.as.string) : "";
            BppSet *proj = set_project(vm_get_str(vm), curr.as.set, field_k);
            curr.type = VAL_SET;
            curr.as.set = proj;
            break;
        } else if (curr.type == VAL_STRING && curr.as.string) {
            int attr = 0, val = 0, subval = 0;
            BValue i1 = eval_expression(vm, sub_lex, out_err);
            if (out_err->code != 0) { lex_shutdown(sub_lex); return false; }
            attr = (int)i1.as.number;
            if (lex_peek(sub_lex).type == TOK_COMMA) {
                lex_next(sub_lex);
                BValue i2 = eval_expression(vm, sub_lex, out_err);
                if (out_err->code != 0) { lex_shutdown(sub_lex); return false; }
                val = (int)i2.as.number;
                if (lex_peek(sub_lex).type == TOK_COMMA) {
                    lex_next(sub_lex);
                    BValue i3 = eval_expression(vm, sub_lex, out_err);
                    if (out_err->code != 0) { lex_shutdown(sub_lex); return false; }
                    subval = (int)i3.as.number;
                }
            }
            BValue p_args[4];
            p_args[0] = curr;
            p_args[1].type = VAL_NUMBER; p_args[1].as.number = (double)attr;
            p_args[2].type = VAL_NUMBER; p_args[2].as.number = (double)val;
            p_args[3].type = VAL_NUMBER; p_args[3].as.number = (double)subval;
            int p_count = (subval > 0) ? 4 : ((val > 0) ? 3 : 2);
            curr = func_extract_eval(vm, "EXTRACT", p_count, p_args, out_err);
            break;
        } else if (curr.type == VAL_SET && curr.as.set) {
            BValue idx_val = eval_expression(vm, sub_lex, out_err);
            if (out_err->code != 0) { lex_shutdown(sub_lex); return false; }
            int idx = (int)idx_val.as.number;
            BValue next_val;
            if (!set_get(curr.as.set, idx, &next_val)) {
                out_err->code = 9; out_err->message = "Set index out of range";
                lex_shutdown(sub_lex);
                return false;
            }
            curr = next_val;
        } else if (curr.type == VAL_GROUP && curr.as.group) {
            char key[64];
            if (peek_t.type == TOK_STRING) {
                lex_next(sub_lex);
                size_t klen = (peek_t.length < sizeof(key) - 1) ? peek_t.length : sizeof(key) - 1;
                runtime_memcpy(key, peek_t.as.string, klen);
                key[klen] = '\0';
            } else if (peek_t.type == TOK_IDENT || peek_t.type == TOK_KEYWORD) {
                lex_next(sub_lex);
                size_t klen = (peek_t.length < sizeof(key) - 1) ? peek_t.length : sizeof(key) - 1;
                runtime_memcpy(key, peek_t.start, klen);
                key[klen] = '\0';
            } else {
                BValue idx_val = eval_expression(vm, sub_lex, out_err);
                if (out_err->code != 0) { lex_shutdown(sub_lex); return false; }
                int idx = (int)idx_val.as.number;
                BValue next_val;
                if (!group_get_at(curr.as.group, idx, NULL, &next_val)) {
                    out_err->code = 9; out_err->message = "Group index out of range";
                    lex_shutdown(sub_lex);
                    return false;
                }
                curr = next_val;
                goto check_comma;
            }
            BValue next_val;
            if (!group_get(curr.as.group, key, &next_val)) {
                out_err->code = 9; out_err->message = "Group member not found";
                lex_shutdown(sub_lex);
                return false;
            }
            curr = next_val;
        } else if (curr.type == VAL_MAP && curr.as.map) {
            char key[64];
            if (peek_t.type == TOK_STRING) {
                lex_next(sub_lex);
                size_t klen = (peek_t.length < sizeof(key) - 1) ? peek_t.length : sizeof(key) - 1;
                runtime_memcpy(key, peek_t.as.string, klen);
                key[klen] = '\0';
            } else {
                lex_next(sub_lex);
                size_t klen = (peek_t.length < sizeof(key) - 1) ? peek_t.length : sizeof(key) - 1;
                runtime_memcpy(key, peek_t.start, klen);
                key[klen] = '\0';
            }
            BValue next_val;
            if (!map_get(curr.as.map, key, &next_val)) {
                out_err->code = 9; out_err->message = "Map key not found";
                lex_shutdown(sub_lex);
                return false;
            }
            curr = next_val;
        } else {
            out_err->code = 13; out_err->message = "Brace indexing applied to non-set/group/map";
            lex_shutdown(sub_lex);
            return false;
        }

check_comma:
        if (lex_peek(sub_lex).type == TOK_COMMA) {
            lex_next(sub_lex);
        } else {
            break;
        }
    }

    lex_shutdown(sub_lex);
    *out_val = curr;
    if (out_val->type == VAL_STRING && out_val->as.string) str_add_ref(out_val->as.string);
    else if (out_val->type == VAL_MAP && out_val->as.map) map_add_ref(out_val->as.map);
    else if (out_val->type == VAL_SET && out_val->as.set) set_add_ref(out_val->as.set);
    else if (out_val->type == VAL_GROUP && out_val->as.group) group_add_ref(out_val->as.group);
    return true;
}

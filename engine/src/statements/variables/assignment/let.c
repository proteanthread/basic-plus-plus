// FILENAME: let.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine
// Provides runtime implementation for the LET statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/assignment/let.h"
#include "statements/oop/sub.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include "runtime/map.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdlib.h>
#include "runtime/format/snprintf.h"


void stmt_let_register(void) {
    static const MicroLibMetadata meta = {
        .name = "LET",
        .category = "Variables & Memory",
        .syntax = "[LET] variable = expression",
        .help_text = "Assigns the value of an expression to a variable or array element.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

typedef struct {
    char var_name[64];
    bool is_array;
    int num_dims;
    int indices[4];
    bool is_slice;
    int slice_start;
    int slice_end;
} AssignTarget;

static bool parse_single_target(VMContext *vm, LexerContext *lex, AssignTarget *target, BppError *err) {
    memset(target, 0, sizeof(*target));
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err->code = 2;
        err->message = "Syntax Error in LET (expected variable name)";
        return false;
    }
    size_t len = (tok.length < sizeof(target->var_name) - 1) ? tok.length : (sizeof(target->var_name) - 1);
    memcpy(target->var_name, tok.start, len);
    target->var_name[len] = '\0';

    while (lex_peek(lex).type == TOK_PERIOD) {
        lex_next(lex); // Consume '.'
        BppToken sub_tok = lex_next(lex);
        if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) {
            err->code = 2; err->message = "Expected identifier after '.' in assignment target";
            return false;
        }
        char sub_part[64];
        size_t slen = (sub_tok.length < sizeof(sub_part) - 1) ? sub_tok.length : sizeof(sub_part) - 1;
        memcpy(sub_part, sub_tok.start, slen);
        sub_part[slen] = '\0';

        char combined[64];
        runtime_snprintf(combined, sizeof(combined), "%s.%s", target->var_name, sub_part);
        strncpy(target->var_name, combined, sizeof(target->var_name) - 1);

        target->var_name[sizeof(target->var_name) - 1] = '\0';
    }

    BppToken next_tok = lex_peek(lex);
    if (next_tok.type == TOK_LBRACKET) {
        target->is_slice = true;
        lex_next(lex); // Consume '['
        BValue s_val = eval_expression(vm, lex, err);
        if (err->code != 0) return false;
        target->slice_start = (int)s_val.as.number;
        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_COMMA || (sep.type == TOK_KEYWORD && sep.as.keyword == KW_TO) ||
            (sep.type == TOK_IDENT && sep.length == 2 && strncasecmp(sep.start, "TO", 2) == 0)) {
            lex_next(lex);
            BValue e_val = eval_expression(vm, lex, err);
            if (err->code != 0) return false;
            target->slice_end = (int)e_val.as.number;
        } else {
            target->slice_end = 0; // Open-ended / full RHS replacement from slice_start
        }
        BppToken rbrk = lex_next(lex);
        if (rbrk.type != TOK_RBRACKET) {
            err->code = 2; err->message = "Expected ']' in string slice";
            return false;
        }
        return true;
    }

    if (next_tok.type == TOK_LPAREN) {
        // Check if string slice or array index
        bool is_str_slice = false;
        if (strchr(target->var_name, '$') && !arr_exists(vm_get_arr(vm), target->var_name)) {
            is_str_slice = true;
        }

        lex_next(lex); // Consume '('
        BValue idx1 = eval_expression(vm, lex, err);
        if (err->code != 0) return false;
        if (idx1.type != VAL_NUMBER && idx1.type != VAL_INTEGER) {
            if (idx1.type == VAL_STRING && idx1.as.string) str_release(vm_get_str(vm), idx1.as.string);
            err->code = 13; err->message = "Type mismatch in index/slice";
            return false;
        }

        BppToken sep = lex_peek(lex);
        if (is_str_slice && (sep.type == TOK_COMMA || (sep.type == TOK_KEYWORD && sep.as.keyword == KW_TO) ||
            (sep.type == TOK_IDENT && sep.length == 2 && strncasecmp(sep.start, "TO", 2) == 0))) {
            target->is_slice = true;
            target->slice_start = (int)idx1.as.number;
            lex_next(lex); // Consume ',' or 'TO'
            BValue idx2 = eval_expression(vm, lex, err);
            if (err->code != 0) return false;
            target->slice_end = (int)idx2.as.number;
            BppToken rpar = lex_next(lex);
            if (rpar.type != TOK_RPAREN) {
                err->code = 2; err->message = "Expected ')' after slice";
                return false;
            }
            return true;
        }

        // Standard Array Index
        target->is_array = true;
        target->indices[target->num_dims++] = (int)idx1.as.number;
        if (sep.type == TOK_RPAREN) {
            lex_next(lex); // Consume ')'
            return true;
        }

        while (true) {
            if (target->num_dims >= 4) {
                err->code = 9;
                err->message = "Subscript out of range (max 4 dimensions)";
                return false;
            }
            BppToken p_comma = lex_next(lex);
            if (p_comma.type != TOK_COMMA) {
                err->code = 2; err->message = "Expected ',' in array subscript";
                return false;
            }
            BValue idx_val = eval_expression(vm, lex, err);
            if (err->code != 0) return false;
            if (idx_val.type != VAL_NUMBER && idx_val.type != VAL_INTEGER) {
                if (idx_val.type == VAL_STRING && idx_val.as.string) {
                    str_release(vm_get_str(vm), idx_val.as.string);
                }
                err->code = 13;
                err->message = "Type mismatch in array index";
                return false;
            }
            target->indices[target->num_dims++] = (int)idx_val.as.number;

            BppToken peek_sub = lex_peek(lex);
            if (peek_sub.type == TOK_COMMA) {
                continue;
            } else if (peek_sub.type == TOK_RPAREN) {
                lex_next(lex);
                break;
            } else {
                err->code = 2;
                err->message = "Syntax error in array subscript (expected ',' or ')')";
                return false;
            }
        }
    }
    return true;
}

BppError stmt_let_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    AssignTarget targets[16];
    int target_count = 0;

    bool is_explicit_let = false;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_LET) {
        is_explicit_let = true;
        lex_next(lex);
    }

    if (!parse_single_target(vm, lex, &targets[target_count++], &err)) {
        return err;
    }

    while (target_count < 16) {
        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
            if (!parse_single_target(vm, lex, &targets[target_count++], &err)) {
                return err;
            }
            continue;
        }

        if (sep.type == TOK_EQ) {
            if (!is_explicit_let) {
                // Implicit LET A = B = C: only 1 target, rest is comparison expr
                lex_next(lex); // Consume '='
                break;
            }

            // Explicit LET A = B = C: lookahead to see if another assignment target follows
            LexerContext *look_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            lex_next(look_lex); // Consume '='
            BppToken look_tok = lex_next(look_lex);
            bool is_next_target = false;
            if (look_tok.type == TOK_IDENT || look_tok.type == TOK_KEYWORD) {
                BppToken after_tok = lex_peek(look_lex);
                if (after_tok.type == TOK_LPAREN) {
                    // Scan past array parens
                    int pcount = 0;
                    while (after_tok.type != TOK_EOF && after_tok.type != TOK_EOL) {
                        if (after_tok.type == TOK_LPAREN) pcount++;
                        else if (after_tok.type == TOK_RPAREN) {
                            pcount--;
                            if (pcount == 0) {
                                lex_next(look_lex);
                                break;
                            }
                        }
                        lex_next(look_lex);
                        after_tok = lex_peek(look_lex);
                    }
                    after_tok = lex_peek(look_lex);
                }
                if (after_tok.type == TOK_EQ || after_tok.type == TOK_COMMA) {
                    is_next_target = true;
                }
            }
            lex_shutdown(look_lex);

            if (is_next_target) {
                lex_next(lex); // Consume '='
                if (!parse_single_target(vm, lex, &targets[target_count++], &err)) {
                    return err;
                }
                continue;
            } else {
                lex_next(lex); // Consume '=' before RHS expr
                break;
            }
        }

        err.code = 2;
        err.message = "Syntax Error in LET (expected '=' or ',')";
        return err;
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    ArrayContext *arr_ctx = vm_get_arr(vm);
    VariableContext *vc = vm_get_var(vm);

    for (int i = 0; i < target_count; i++) {
        if (targets[i].is_slice) {
            BValue *cur = var_lookup(vc, targets[i].var_name, false);
            const char *orig_str = (cur && cur->type == VAL_STRING && cur->as.string) ? str_data(cur->as.string) : "";
            int orig_len = (int)strlen(orig_str);
            const char *rhs_str = (val.type == VAL_STRING && val.as.string) ? str_data(val.as.string) : "";
            int rhs_len = (int)strlen(rhs_str);

            int start = targets[i].slice_start;
            int end = (targets[i].slice_end > 0) ? targets[i].slice_end : (start + rhs_len - 1);
            if (start < 1) start = 1;
            if (end < start) end = start;

            int max_len = orig_len;
            if (end > max_len) max_len = end;
            if (start - 1 + rhs_len > max_len) max_len = start - 1 + rhs_len;
            if (max_len < 0) max_len = 0;

            char *buf = (char *)malloc(max_len + 1);
            if (buf) {
                for (int j = 0; j < max_len; ++j) {
                    buf[j] = (j < orig_len) ? orig_str[j] : ' ';
                }
                buf[max_len] = '\0';
                int slice_cap = end - start + 1;
                int copy_len = (rhs_len < slice_cap) ? rhs_len : slice_cap;
                for (int j = 0; j < copy_len; ++j) {
                    buf[start - 1 + j] = rhs_str[j];
                }
                BValue new_val;
                new_val.type = VAL_STRING;
                new_val.as.string = str_create(vm_get_str(vm), buf, max_len);
                var_assign(vc, targets[i].var_name, new_val);
                free(buf);
            }
        } else if (targets[i].is_array) {
            BValue *target = arr_get_element(arr_ctx, targets[i].var_name, targets[i].num_dims, targets[i].indices, &err);
            if (err.code != 0 || !target) {
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
                return err;
            }
            if (target->type == VAL_STRING && target->as.string) {
                str_release(vm_get_str(vm), target->as.string);
            }
            *target = val;
            if (val.type == VAL_STRING && val.as.string) {
                str_add_ref(val.as.string);
            }
        } else {
            const char *dot = strchr(targets[i].var_name, '.');
            bool handled_map = false;
            if (dot) {
                char base_name[64] = {0};
                size_t blen = (size_t)(dot - targets[i].var_name);
                if (blen < sizeof(base_name)) {
                    memcpy(base_name, targets[i].var_name, blen);
                    const char *field_name = dot + 1;
                    BValue *base_val = var_lookup(vc, base_name, false);
                    if (base_val && base_val->type == VAL_MAP && base_val->as.map) {
                        // Check for PROPERTY SET / PROPERTY LET procedure
                        BValue type_val;
                        bool has_prop_proc = false;
                        if (map_get(base_val->as.map, "__type__", &type_val) && type_val.type == VAL_STRING && type_val.as.string) {
                            char prop_proc[128];
                            runtime_snprintf(prop_proc, sizeof(prop_proc), "%s.%s", str_data(type_val.as.string), field_name);
                            BppLineNumber target_line = 0;

                            const char *target_text = NULL;
                            bool is_lib = false;
                            if (find_procedure_ex(vm, prop_proc, KW_PROPERTY, &target_line, &target_text, &is_lib) ||
                                find_procedure_ex(vm, prop_proc, KW_SUB, &target_line, &target_text, &is_lib)) {
                                BValue p_args[2];
                                p_args[0] = *base_val;
                                map_add_ref(p_args[0].as.map);
                                p_args[1] = val;
                                if (val.type == VAL_STRING && val.as.string) str_add_ref(val.as.string);
                                else if (val.type == VAL_MAP && val.as.map) map_add_ref(val.as.map);
                                BppError p_err;
                                memset(&p_err, 0, sizeof(p_err));
                                invoke_user_function(vm, prop_proc, p_args, 2, &p_err);
                                if (p_args[0].type == VAL_MAP && p_args[0].as.map) map_release(vm_get_str(vm), p_args[0].as.map);
                                if (p_args[1].type == VAL_STRING && p_args[1].as.string) str_release(vm_get_str(vm), p_args[1].as.string);
                                else if (p_args[1].type == VAL_MAP && p_args[1].as.map) map_release(vm_get_str(vm), p_args[1].as.map);
                                has_prop_proc = true;
                                handled_map = true;
                            }
                        }
                        if (!has_prop_proc) {
                            map_set(vm_get_str(vm), base_val->as.map, field_name, val);
                            handled_map = true;
                        }
                    } else if (!base_val) {
                        BppMap *new_map = map_create();
                        if (new_map) {
                            map_set(vm_get_str(vm), new_map, field_name, val);
                            BValue map_val = { .type = VAL_MAP, .as.map = new_map };
                            var_assign(vc, base_name, map_val);
                            map_release(vm_get_str(vm), new_map);
                            handled_map = true;
                        }
                    }
                }
            } else if (val.type == VAL_MAP && val.as.map == NULL) {
                // SET obj = NOTHING
                BValue *cur_var = var_lookup(vc, targets[i].var_name, false);
                if (cur_var && cur_var->type == VAL_MAP && cur_var->as.map) {
                    BValue type_val;
                    if (map_get(cur_var->as.map, "__type__", &type_val) && type_val.type == VAL_STRING && type_val.as.string) {
                        char dt_name[128];
                        runtime_snprintf(dt_name, sizeof(dt_name), "%s.DESTRUCTOR", str_data(type_val.as.string));
                        BppLineNumber target_line = 0;
                        const char *target_text = NULL;
                        bool is_lib = false;
                        if (find_procedure_ex(vm, dt_name, KW_DESTRUCTOR, &target_line, &target_text, &is_lib) ||
                            find_procedure_ex(vm, dt_name, KW_SUB, &target_line, &target_text, &is_lib)) {
                            BValue d_arg = *cur_var;
                            map_add_ref(d_arg.as.map);
                            BppError d_err;
                            memset(&d_err, 0, sizeof(d_err));
                            invoke_user_function(vm, dt_name, &d_arg, 1, &d_err);
                            map_release(vm_get_str(vm), d_arg.as.map);
                        } else {
                            runtime_snprintf(dt_name, sizeof(dt_name), "%s.Class_Terminate", str_data(type_val.as.string));

                            if (find_procedure_ex(vm, dt_name, KW_SUB, &target_line, &target_text, &is_lib)) {
                                BValue d_arg = *cur_var;
                                map_add_ref(d_arg.as.map);
                                BppError d_err;
                                memset(&d_err, 0, sizeof(d_err));
                                invoke_user_function(vm, dt_name, &d_arg, 1, &d_err);
                                map_release(vm_get_str(vm), d_arg.as.map);
                            }
                        }
                    }
                    map_release(vm_get_str(vm), cur_var->as.map);
                    cur_var->as.map = NULL;
                    handled_map = true;
                }
            }

            if (!handled_map) {
                if (!var_assign(vc, targets[i].var_name, val)) {
                    err.code = 13; // Type mismatch
                    err.message = "Type Mismatch in variable assignment";
                }
            }
        }
    }

    if (val.type == VAL_STRING && val.as.string) {
        str_release(vm_get_str(vm), val.as.string);
    }

    return err;
}


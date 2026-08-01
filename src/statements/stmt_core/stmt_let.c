#include "bpp_stmt.h"
#include "bpp_eval.h"
#include "bpp_map.h"
#include "bpp_struct.h"
#include "bpp_vfs.h"
#include "bpp_platform.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern double platform_get_timer(void);
extern double platform_get_uptime(void);
extern void vm_set_ti_offset(VMContext *vm, double val);

static void split_member_chain(const char *start, size_t len, char *var_name, size_t var_name_max, char member_chain[8][64], int *member_count) {
    *member_count = 0;
    
    /* Find first dot */
    size_t dot_idx = 0;
    while (dot_idx < len && start[dot_idx] != '.') {
        dot_idx++;
    }
    
    if (dot_idx == len) {
        /* No dot: single variable name */
        size_t clen = (len < var_name_max - 1) ? len : var_name_max - 1;
        memcpy(var_name, start, clen);
        var_name[clen] = '\0';
        return;
    }
    
    /* Copy var_name */
    size_t clen = (dot_idx < var_name_max - 1) ? dot_idx : var_name_max - 1;
    memcpy(var_name, start, clen);
    var_name[clen] = '\0';
    
    /* Parse members */
    size_t i = dot_idx + 1;
    while (i < len && *member_count < 8) {
        size_t next_dot = i;
        while (next_dot < len && start[next_dot] != '.') {
            next_dot++;
        }
        
        size_t mlen = next_dot - i;
        size_t copy_mlen = (mlen < 63) ? mlen : 63;
        memcpy(member_chain[*member_count], start + i, copy_mlen);
        member_chain[*member_count][copy_mlen] = '\0';
        (*member_count)++;
        
        i = next_dot + 1;
    }
}

#define MAX_LET_TARGETS 8
struct LetTarget {
    char var_name[256];
    char member_chain[8][64];
    int member_count;
    bool is_array;
    bool is_slice;
    int indices[4];
    int num_indices;
    int start;
    int end;
    bool end_specified;
    BppTokenType slice_open_tok;
};

BppError stmt_let_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_LET) {
        lex_next(lex); /* Consume 'LET' */
        tok = lex_peek(lex);
    }

    bool is_path_target = false;
    if (tok.type == TOK_IDENT) {
        if (tok.length == 4 && platform_strncasecmp(tok.start, "PATH", 4) == 0) {
            is_path_target = true;
        } else if (tok.length == 5 && platform_strncasecmp(tok.start, "PATH$", 5) == 0) {
            is_path_target = true;
        }
    } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_PATH) {
        is_path_target = true;
    }

    if (is_path_target) {
        lex_next(lex); /* Consume 'PATH' / 'PATH$' */
        bool is_cat = false;
        char category[64] = "";
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex); /* Consume '(' */
            BValue cat_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (cat_val.type != VAL_STRING) {
                err.code = 13; err.message = "Expected string category in PATH$";
                return err;
            }
            strncpy(category, str_data(cat_val.as.string), sizeof(category) - 1);
            category[sizeof(category) - 1] = '\0';
            str_release(vm_get_str(vm), cat_val.as.string);

            BppToken close_tok = lex_next(lex);
            if (close_tok.type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' in PATH$ assignment";
                return err;
            }
            is_cat = true;
        }

        BppToken eq_tok = lex_next(lex);
        if (eq_tok.type != TOK_EQ) {
            err.code = 2; err.message = "Expected '=' in PATH$ assignment";
            return err;
        }

        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type != VAL_STRING) {
            err.code = 13; err.message = "PATH$ assignment requires string value";
            return err;
        }

        if (is_cat) {
            vfs_set_category_path(vm_get_vfs(vm), category, str_data(val.as.string));
        } else {
            vfs_set_search_path(vm_get_vfs(vm), str_data(val.as.string));
        }
        str_release(vm_get_str(vm), val.as.string);
        return err;
    }

    struct LetTarget targets[MAX_LET_TARGETS];
    int target_count = 0;

    extern bool arr_exists(ArrayContext *ctx, const char *name);

    while (true) {
        if (target_count >= MAX_LET_TARGETS) {
            err.code = 9; err.message = "Too many assignment targets in LET"; return err;
        }

        extern const char *vm_with_stack_peek(VMContext *vm);
        tok = lex_peek(lex);
        bool is_with_target = false;
        if (tok.type == TOK_PERIOD) {
            const char *with_prefix = vm_with_stack_peek(vm);
            if (!with_prefix) {
                err.code = 2; err.message = "Leading '.' outside of WITH block in assignment";
                return err;
            }
            lex_next(lex); /* Consume '.' */
            tok = lex_peek(lex);
            if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
                err.code = 2; err.message = "Expected variable name after '.' in WITH assignment";
                return err;
            }
            is_with_target = true;
        } else if (tok.type != TOK_IDENT &&
                   !(tok.type == TOK_KEYWORD &&
                     (tok.as.keyword == KW_MOUSE ||
                      tok.as.keyword == KW_HMOUSE ||
                      tok.as.keyword == KW_VMOUSE ||
                      tok.as.keyword == KW_TRIG))) {
            err.code = 2; err.message = "Expected variable name in LET assignment";
            return err;
        }
        lex_next(lex); /* Consume identifier */

        struct LetTarget *t = &targets[target_count++];
        memset(t, 0, sizeof(struct LetTarget));
        t->end = -1;
        t->start = 1;
        t->slice_open_tok = TOK_LPAREN;

        if (is_with_target) {
            const char *with_prefix = vm_with_stack_peek(vm);
            char fq_name[512];
            snprintf(fq_name, sizeof(fq_name), "%s.%.*s", with_prefix, (int)tok.length, tok.start);
            split_member_chain(fq_name, strlen(fq_name), t->var_name, sizeof(t->var_name), t->member_chain, &t->member_count);
        } else {
            split_member_chain(tok.start, tok.length, t->var_name, sizeof(t->var_name), t->member_chain, &t->member_count);
        }

        if (lex_peek(lex).type == TOK_LBRACKET) {
            t->is_slice = true;
            t->slice_open_tok = TOK_LBRACKET;
        } else if (lex_peek(lex).type == TOK_LPAREN) {
            if (t->var_name[strlen(t->var_name) - 1] == '$' && !arr_exists(vm_get_arr(vm), t->var_name)) {
                t->is_slice = true;
                t->slice_open_tok = TOK_LPAREN;
            } else {
                t->is_array = true;
            }
        }

        if (t->is_slice) {
            lex_next(lex); /* Consume open token */
            BppTokenType close_tok = (t->slice_open_tok == TOK_LPAREN) ? TOK_RPAREN : TOK_RBRACKET;

            BppToken next = lex_peek(lex);
            if (next.type == TOK_KEYWORD && next.as.keyword == KW_TO) {
                lex_next(lex); /* Consume 'TO' */
                if (lex_peek(lex).type != close_tok) {
                    BValue end_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (end_val.type == VAL_STRING) { err.code = 13; err.message = "String slice bound must be numeric"; return err; }
                    t->end = (int)end_val.as.number;
                    t->end_specified = true;
                }
            } else if (next.type == close_tok) {
                /* full slice */
            } else {
                BValue start_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (start_val.type == VAL_STRING) { err.code = 13; err.message = "String slice bound must be numeric"; return err; }
                t->start = (int)start_val.as.number;

                next = lex_peek(lex);
                if (next.type == TOK_KEYWORD && next.as.keyword == KW_TO) {
                    lex_next(lex); /* Consume 'TO' */
                    if (lex_peek(lex).type != close_tok) {
                        BValue end_val = eval_expression(vm, lex, &err);
                        if (err.code != 0) return err;
                        if (end_val.type == VAL_STRING) { err.code = 13; err.message = "String slice bound must be numeric"; return err; }
                        t->end = (int)end_val.as.number;
                        t->end_specified = true;
                    }
                } else if (next.type == TOK_COMMA) {
                    lex_next(lex); /* Consume ',' */
                    BValue end_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (end_val.type == VAL_STRING) { err.code = 13; err.message = "String slice bound must be numeric"; return err; }
                    t->end = (int)end_val.as.number;
                    t->end_specified = true;
                } else {
                    t->end = t->start;
                    t->end_specified = true;
                }
            }

            if (lex_peek(lex).type != close_tok) {
                err.code = 2; err.message = (t->slice_open_tok == TOK_LPAREN) ? "Expected ')' in slice" : "Expected ']' in slice";
                return err;
            }
            lex_next(lex); /* Consume close token */
        } else if (t->is_array) {
            lex_next(lex); /* Consume '(' */
            while (true) {
                if (t->num_indices >= 4) {
                    err.code = 9; err.message = "Too many dimensions for array assignment"; return err;
                }
                BValue idx_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (idx_val.type == VAL_STRING) {
                    err.code = 13; err.message = "String values are not allowed as array indices"; return err;
                }
                t->indices[t->num_indices++] = (int)idx_val.as.number;

                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (next_tok.type == TOK_RPAREN) {
                    break;
                } else {
                    err.code = 2; err.message = "Expected ',' or ')' in array index list"; return err;
                }
            }
            lex_next(lex); /* Consume ')' */
        }

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    /* Expect '=' */
    tok = lex_next(lex);
    if (tok.type != TOK_EQ) {
        err.code = 2; err.message = "Expected '=' in LET assignment";
        return err;
    }

    /* Evaluate RHS expression */
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    /* Apply assignment to all targets */
    for (int t_idx = 0; t_idx < target_count; t_idx++) {
        struct LetTarget *t = &targets[t_idx];
        
        /* If we are assigning to multiple targets, we need to clone strings and maps if we are not on the last target to avoid double free */
        BValue target_val = val;
        if (t_idx < target_count - 1) {
            if (val.type == VAL_STRING && val.as.string) {
                target_val.as.string = str_create(vm_get_str(vm), str_data(val.as.string), str_len(val.as.string));
            } else if (val.type == VAL_MAP && val.as.map) {
                char copy_err[128];
                BppMap *new_map = bpp_map_create();
                struct_copy_instance(vm, new_map, val.as.map, copy_err, sizeof(copy_err));
                target_val.as.map = new_map;
            }
        }

        if (t->is_slice) {
            if (target_val.type != VAL_STRING) {
                err.code = 13; err.message = "Type mismatch: string slice requires string value";
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                return err;
            }

            VariableContext *var_ctx = vm_get_var(vm);
            BValue *target_var = var_lookup(var_ctx, t->var_name, true);
            if (!target_var) {
                err.code = 2; err.message = "Undeclared variable in assignment";
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                return err;
            }

            const char *orig_str = "";
            size_t orig_len = 0;
            if (target_var->type == VAL_STRING && target_var->as.string) {
                orig_str = str_data(target_var->as.string);
                orig_len = str_len(target_var->as.string);
            }

            if (t->start < 1) t->start = 1;
            if (t->end == -1 || !t->end_specified) t->end = (int)orig_len;
            if (t->end < t->start) t->end = t->start - 1;
            if (t->start > (int)orig_len + 1) t->start = (int)orig_len + 1;

            size_t slice_len = (size_t)(t->end - t->start + 1);
            const char *rhs_str = str_data(target_val.as.string);
            size_t rhs_len = strlen(rhs_str);

            size_t new_alloc = orig_len + slice_len + rhs_len + 256;
            char *new_buf = (char *)calloc(1, new_alloc);
            if (!new_buf) {
                err.code = 5; err.message = "Out of memory in slice assignment";
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                return err;
            }

            size_t ptr = 0;
            if (t->start > 1) {
                size_t prefix_len = (size_t)(t->start - 1);
                if (prefix_len > orig_len) prefix_len = orig_len;
                memcpy(new_buf + ptr, orig_str, prefix_len);
                ptr += prefix_len;
            }
            for (size_t i = 0; i < slice_len; i++) {
                if (i < rhs_len) new_buf[ptr++] = rhs_str[i];
                else new_buf[ptr++] = ' ';
            }
            if (t->end < (int)orig_len) {
                size_t suffix_start = (size_t)t->end;
                size_t suffix_len = orig_len - suffix_start;
                memcpy(new_buf + ptr, orig_str + suffix_start, suffix_len);
                ptr += suffix_len;
            }
            new_buf[ptr] = '\0';

            BppStringRef old_str = (target_var->type == VAL_STRING) ? target_var->as.string : NULL;
            target_var->type = VAL_STRING;
            target_var->as.string = str_create(vm_get_str(vm), new_buf, ptr);

            if (old_str) str_release(vm_get_str(vm), old_str);
            str_release(vm_get_str(vm), target_val.as.string);
            free(new_buf);
            continue;
        }

        if (t->member_count > 0) {
            BValue *base_var = NULL;
            if (t->is_array) {
                base_var = arr_get_element(vm_get_arr(vm), t->var_name, t->num_indices, t->indices, &err);
                if (err.code != 0 || !base_var) {
                    if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                    else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                    return err;
                }
            } else {
                base_var = var_lookup(vm_get_var(vm), t->var_name, true);
                if (!base_var) {
                    err.code = 35; err.message = "Variable not declared";
                    if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                    else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                    return err;
                }
            }

            if (base_var->type != VAL_MAP || !base_var->as.map) {
                err.code = 13; err.message = "Member assignment on non-object variable";
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                return err;
            }
            BppMap *curr_map = base_var->as.map;

            for (int i = 0; i < t->member_count - 1; i++) {
                BValue temp;
                if (!bpp_map_get(curr_map, t->member_chain[i], &temp)) {
                    err.code = 35; err.message = "Member field not found";
                    if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                    else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                    return err;
                }
                if (temp.type != VAL_MAP || !temp.as.map) {
                    err.code = 13; err.message = "Member field is not an object";
                    if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                    else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                    return err;
                }
                curr_map = temp.as.map;
            }

            BValue field_var;
            const char *last_field = t->member_chain[t->member_count - 1];
            if (!bpp_map_get(curr_map, last_field, &field_var)) {
                err.code = 35; err.message = "Member field not defined in UDT/Class";
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                return err;
            }

            if (field_var.type != target_val.type) {
                err.code = 13; err.message = "Type mismatch in member field assignment";
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                return err;
            }

            bpp_map_set(vm_get_str(vm), curr_map, last_field, target_val);
            if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
            else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
            continue;
        }

        if (t->is_array) {
            BValue *elem = arr_get_element(vm_get_arr(vm), t->var_name, t->num_indices, t->indices, &err);
            if (err.code != 0 || !elem) {
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                return err;
            }

            bool elem_is_numeric = (elem->type == VAL_NUMBER || elem->type == VAL_INTEGER);
            bool val_is_numeric = (target_val.type == VAL_NUMBER || target_val.type == VAL_INTEGER);
            if (elem_is_numeric != val_is_numeric || (elem->type == VAL_STRING && target_val.type != VAL_STRING) || (elem->type == VAL_MAP && target_val.type != VAL_MAP)) {
                err.code = 13; err.message = "Type mismatch in array assignment";
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
                return err;
            }

            if (elem->type == VAL_MAP) {
                char copy_err[128];
                if (!struct_copy_instance(vm, elem->as.map, target_val.as.map, copy_err, sizeof(copy_err))) {
                    err.code = 13; err.message = "Type mismatch in structure assignment";
                    bpp_map_release(vm_get_str(vm), target_val.as.map);
                    return err;
                }
                bpp_map_release(vm_get_str(vm), target_val.as.map);
                continue;
            }

            if (elem->type == VAL_STRING && elem->as.string) str_release(vm_get_str(vm), elem->as.string);

            if (elem->type == VAL_INTEGER) elem->as.number = (double)((int32_t)target_val.as.number);
            else *elem = target_val;
            continue;
        }

        VariableContext *var_ctx = vm_get_var(vm);
        BValue *existing_var = var_lookup(var_ctx, t->var_name, false);
        if (existing_var && existing_var->type == VAL_MAP && target_val.type == VAL_MAP) {
            char copy_err[128];
            if (!struct_copy_instance(vm, existing_var->as.map, target_val.as.map, copy_err, sizeof(copy_err))) {
                err.code = 13; err.message = "Type mismatch in structure assignment";
                bpp_map_release(vm_get_str(vm), target_val.as.map);
                return err;
            }
            bpp_map_release(vm_get_str(vm), target_val.as.map);
            continue;
        }

        if (platform_strncasecmp(t->var_name, "TI$", 3) == 0) {
            if (target_val.type == VAL_STRING) {
                const char *data = str_data(target_val.as.string);
                size_t len = str_len(target_val.as.string);
                long long days = 0;
                int hr = 0, min = 0, sec = 0;
                const char *colon = strchr(data, ':');
                if (colon) {
                    char days_buf[64] = {0};
                    size_t days_len = colon - data;
                    if (days_len > 63) days_len = 63;
                    memcpy(days_buf, data, days_len);
                    days = atoll(days_buf);
                    const char *time_part = colon + 1;
                    if (strlen(time_part) >= 6) {
                        hr = (time_part[0] - '0') * 10 + (time_part[1] - '0');
                        min = (time_part[2] - '0') * 10 + (time_part[3] - '0');
                        sec = (time_part[4] - '0') * 10 + (time_part[5] - '0');
                    } else {
                        err.code = 13; err.message = "TI$ expects time suffix in HHMMSS format (e.g., DD:HHMMSS)";
                        if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                        return err;
                    }
                } else if (len >= 6) {
                    hr = (data[0] - '0') * 10 + (data[1] - '0');
                    min = (data[2] - '0') * 10 + (data[3] - '0');
                    sec = (data[4] - '0') * 10 + (data[5] - '0');
                } else {
                    err.code = 13; err.message = "TI$ expects string in DD:HHMMSS or HHMMSS format";
                    if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                    return err;
                }
                double new_sec = days * 86400.0 + hr * 3600.0 + min * 60.0 + sec;
                double current_sec = platform_get_uptime();
                vm_set_ti_offset(vm, new_sec - current_sec);
            } else {
                err.code = 13; err.message = "TI$ expects a string value";
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                return err;
            }
            if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
            continue;
        }

        if (!var_assign(var_ctx, t->var_name, target_val)) {
            err.code = 13; err.message = "Type mismatch or undeclared variable in assignment";
            if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
            else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
            return err;
        }

        if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
        else if (target_val.type == VAL_MAP && target_val.as.map) bpp_map_release(vm_get_str(vm), target_val.as.map);
    }


    return err;
}

BppError stmt_swap_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    
    /* Helper lambda-like to parse a variable reference */
    BValue *ref1 = NULL;
    BValue *ref2 = NULL;
    
    for (int i = 0; i < 2; i++) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected variable name in SWAP";
            return err;
        }
        
        char var_name[64];
        size_t clen = (tok.length < 63) ? tok.length : 63;
        memcpy(var_name, tok.start, clen);
        var_name[clen] = '\0';
        
        bool is_array = false;
        int indices[4];
        int num_indices = 0;
        
        if (lex_peek(lex).type == TOK_LPAREN) {
            is_array = true;
            lex_next(lex); /* Consume '(' */
            while (true) {
                if (num_indices >= 4) {
                    err.code = 9; err.message = "Too many dimensions"; return err;
                }
                BValue idx_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                indices[num_indices++] = (int)idx_val.as.number;
                
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (next_tok.type == TOK_RPAREN) {
                    break;
                } else {
                    err.code = 2; err.message = "Expected ',' or ')'"; return err;
                }
            }
            lex_next(lex); /* Consume ')' */
        }
        
        BValue *target = NULL;
        if (is_array) {
            target = arr_get_element(vm_get_arr(vm), var_name, num_indices, indices, &err);
            if (err.code != 0) return err;
        } else {
            target = var_lookup(vm_get_var(vm), var_name, true);
        }
        
        if (i == 0) {
            ref1 = target;
            BppToken sep = lex_next(lex);
            if (sep.type != TOK_COMMA) {
                err.code = 2; err.message = "Expected ',' in SWAP"; return err;
            }
        } else {
            ref2 = target;
        }
    }
    
    if (ref1 && ref2) {
        if (ref1->type != ref2->type && ref1->type != VAL_NONE && ref2->type != VAL_NONE) {
            err.code = 13; err.message = "Type mismatch in SWAP"; return err;
        }
        BValue temp = *ref1;
        *ref1 = *ref2;
        *ref2 = temp;
    }
    
    return err;
}

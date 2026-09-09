// FILENAME: ast_eval_stmt_assign.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast_eval_stmt.c)
// NEEDS: libcore, libengine (ast_internal.h)
// AST assignment evaluators for scalar variables and multidimensional arrays.
//
// ---- Includes ----

#include "eval/ast_internal.h"
#include "eval/rpn_mirror.h"
#include "reg/reg_udx.h"
#include "reg/reg_hw.h"
#include "eval/functions/system/hardware/func_baud.h"
#include "runtime/arrays.h"
#include "runtime/format/snprintf.h"
#include "runtime/map.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/variables.h"
#include "runtime/variables_internal.h"
#include "statements/oop/sub.h"

// executes an AST assignment statement (scalar, 1D array, or 2D array)
bool eval_ast_exec_assign(VMContext *vm, EvalAstNode *node, BppError *err) {
    if (!vm || !node) return false;

    switch (node->type) {
        case AST_NODE_ASSIGNMENT: {
            if (node->cached_var_ptr && (node->cached_var_ptr->type == VAL_NUMBER || node->cached_var_ptr->type == VAL_INTEGER)) {
                BValue val = eval_ast_evaluate(vm, node->right, err);
                if (err->code != 0) return true;
                if (val.type == VAL_SET || val.type == VAL_GROUP || val.type == VAL_MAP || val.type == VAL_STRING) {
                    var_assign(vm_get_var(vm), node->var_name, val);
                    return true;
                }
                if (node->cached_var_ptr->type == VAL_INTEGER) {
                    node->cached_var_ptr->as.number = (double)((int32_t)val.as.number);
                } else {
                    node->cached_var_ptr->type = VAL_NUMBER;
                    node->cached_var_ptr->as.number = val.as.number;
                }
                if (rpn_get_eval_mirror()) {
                    rpn_mirror_eval_result(val);
                }
                return true;
            }
            if (reg_is_hardware_domain(node->var_name) || reg_is_math_domain(node->var_name) || reg_is_udx_domain(node->var_name)) {
                const char *rhs_hint = (node->right && node->right->var_name) ? node->right->var_name : NULL;
                BValue val = eval_ast_evaluate(vm, node->right, err);
                if (err->code != 0) return true;
                reg_assign_builtin(node->var_name, val, rhs_hint, err);
                return true;
            }
            if (runtime_strcasecmp(node->var_name, "SPEED") == 0) {
                double num = eval_ast_eval_num(vm, node->right, err);
                if (err->code != 0) return true;
                bool is_fractional_baud = ((num > 45.4 && num < 45.5) ||
                                           (num > 56.8 && num < 56.9) ||
                                           (num > 74.1 && num < 74.3) ||
                                           (num > 134.4 && num < 134.6));
                if (num > 255.0 || is_fractional_baud) {
                    baud_set_channel_rate(0, num);
                    speed_set_apple_speed(255.0);
                } else {
                    speed_set_apple_speed(num);
                    baud_set_channel_rate(0, 0.0);
                }
                return true;
            }
            if (runtime_strcasecmp(node->var_name, "SPEED%") == 0) {
                double num = eval_ast_eval_num(vm, node->right, err);
                if (err->code != 0) return true;
                speed_set_apple_speed(num);
                baud_set_channel_rate(0, 0.0);
                return true;
            }
            if (runtime_strcasecmp(node->var_name, "SPEED&") == 0 || runtime_strcasecmp(node->var_name, "BAUD") == 0) {
                double num = eval_ast_eval_num(vm, node->right, err);
                if (err->code != 0) return true;
                baud_set_channel_rate(0, num);
                speed_set_apple_speed(255.0);
                return true;
            }
            const char *dot_pos = runtime_strchr(node->var_name, '.');
            if (dot_pos) {
                char base_name[128];
                size_t blen = (size_t)(dot_pos - node->var_name);
                if (blen < sizeof(base_name)) {
                    runtime_memcpy(base_name, node->var_name, blen);
                    base_name[blen] = '\0';
                    const char *field_name = dot_pos + 1;
                    BValue *base_val = var_lookup(vm_get_var(vm), base_name, false);
                    if (base_val && base_val->type == VAL_MAP && base_val->as.map) {
                        BValue val = eval_ast_evaluate(vm, node->right, err);
                        if (err->code != 0) return true;
                        map_set(vm_get_str(vm), base_val->as.map, field_name, val);
                        if (val.type == VAL_STRING && val.as.string) {
                            str_release(vm_get_str(vm), val.as.string);
                        }
                        return true;
                    }
                }
            }

            if (!node->cached_var_ptr) {
                DynamicVarEntry *dvar = var_find_dynamic(vm_get_var(vm), node->var_name);
                if (dvar && (dvar->setter || dvar->write_fn[0] != '\0')) {
                    BValue val = eval_ast_evaluate(vm, node->right, err);
                    if (err->code != 0) return true;
                    if (dvar->setter) {
                        dvar->setter(vm_get_var(vm), node->var_name, val, dvar->user_data);
                    } else if (dvar->write_fn[0] != '\0') {
                        BValue w_args[1];
                        w_args[0] = val;
                        if (val.type == VAL_STRING && val.as.string) str_add_ref(val.as.string);
                        else if (val.type == VAL_MAP && val.as.map) map_add_ref(val.as.map);
                        BppError w_err;
                        runtime_memset(&w_err, 0, sizeof(w_err));
                        invoke_user_function(vm, dvar->write_fn, w_args, 1, &w_err);
                        if (w_args[0].type == VAL_STRING && w_args[0].as.string) str_release(vm_get_str(vm), w_args[0].as.string);
                        else if (w_args[0].type == VAL_MAP && w_args[0].as.map) map_release(vm_get_str(vm), w_args[0].as.map);
                        if (w_err.code != 0) *err = w_err;
                    }
                    if (val.type == VAL_STRING && val.as.string) {
                        str_release(vm_get_str(vm), val.as.string);
                    }
                    return true;
                }
                node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
            }

            size_t name_len = runtime_strlen(node->var_name);
            bool is_str_var = (name_len > 0 && node->var_name[name_len - 1] == '$') ||
                              (node->cached_var_ptr && node->cached_var_ptr->type == VAL_STRING) ||
                              (node->cached_var_ptr && node->cached_var_ptr->type == VAL_NONE && is_ast_string_node(node->right));

            if (is_str_var) {
                size_t max_len = var_get_max_len(vm_get_var(vm), node->var_name);
                if (max_len > 0) {
                    BValue val = eval_ast_evaluate(vm, node->right, err);
                    if (err->code != 0) return true;
                    var_assign(vm_get_var(vm), node->var_name, val);
                    if (val.type == VAL_STRING && val.as.string) {
                        str_release(vm_get_str(vm), val.as.string);
                    }
                    return true;
                }
            }

            if (!is_str_var && node->cached_var_ptr) {
                BValue val = eval_ast_evaluate(vm, node->right, err);
                if (err->code != 0) return true;
                if (val.type == VAL_SET || val.type == VAL_GROUP || val.type == VAL_MAP) {
                    var_assign(vm_get_var(vm), node->var_name, val);
                    return true;
                }
                if (node->cached_var_ptr->type == VAL_STRING && node->cached_var_ptr->as.string) {
                    str_release(vm_get_str(vm), node->cached_var_ptr->as.string);
                }
                if (val.type == VAL_INTEGER) {
                    node->cached_var_ptr->type = VAL_INTEGER;
                    node->cached_var_ptr->as.number = (double)((int32_t)val.as.number);
                } else if (val.type == VAL_NUMBER) {
                    node->cached_var_ptr->type = VAL_NUMBER;
                    node->cached_var_ptr->as.number = val.as.number;
                } else {
                    var_assign(vm_get_var(vm), node->var_name, val);
                }
                if (rpn_get_eval_mirror()) {
                    rpn_mirror_eval_result(val);
                }
                return true;
            }

            if (is_str_var && node->cached_var_ptr) {
                StringContext *sc = vm_get_str(vm);
                if (node->cached_var_ptr->type == VAL_NONE) {
                    node->cached_var_ptr->type = VAL_STRING;
                    node->cached_var_ptr->as.string = NULL;
                }

                // Fast-path: var$ = literal string
                if (node->right && node->right->type == AST_NODE_LITERAL && node->right->val.type == VAL_STRING) {
                    if (node->cached_var_ptr->type == VAL_STRING && node->cached_var_ptr->as.string) {
                        str_release(sc, node->cached_var_ptr->as.string);
                    }
                    node->cached_var_ptr->type = VAL_STRING;
                    node->cached_var_ptr->as.string = node->right->val.as.string;
                    if (node->cached_var_ptr->as.string) str_add_ref(node->cached_var_ptr->as.string);
                    return true;
                }

                // Fast-path: var$ = var$
                if (node->right && node->right->type == AST_NODE_VARIABLE) {
                    if (!node->right->cached_var_ptr) node->right->cached_var_ptr = var_lookup(vm_get_var(vm), node->right->var_name, false);
                    BValue *vp = node->right->cached_var_ptr;
                    if (vp && vp->type == VAL_STRING) {
                        if (node->cached_var_ptr->type == VAL_STRING && node->cached_var_ptr->as.string) {
                            str_release(sc, node->cached_var_ptr->as.string);
                        }
                        node->cached_var_ptr->type = VAL_STRING;
                        node->cached_var_ptr->as.string = vp->as.string;
                        if (node->cached_var_ptr->as.string) str_add_ref(node->cached_var_ptr->as.string);
                        return true;
                    }
                }

                // Multi-concat fast-path: var$ = a$ + b$ + c$ ...
                if (node->right && node->right->type == AST_NODE_BINARY_OP && node->right->op == TOK_PLUS) {
                    const char *parts[16];
                    size_t lens[16];
                    char num_bufs[16][64];
                    size_t num_buf_idx = 0;
                    BppString *to_rel[16];
                    size_t scount = 0, rcount = 0;

                    eval_ast_flatten_concat(vm, node->right->left, parts, lens, num_bufs, &num_buf_idx, to_rel, &scount, &rcount, 16, err);
                    if (err->code == 0) {
                        eval_ast_flatten_concat(vm, node->right->right, parts, lens, num_bufs, &num_buf_idx, to_rel, &scount, &rcount, 16, err);
                    }
                    if (err->code == 0) {
                        if (node->cached_var_ptr->type == VAL_STRING) {
                            node->cached_var_ptr->as.string = str_concat_multi_inplace(sc, node->cached_var_ptr->as.string, parts, lens, scount);
                        } else {
                            if (node->cached_var_ptr->type == VAL_STRING && node->cached_var_ptr->as.string) {
                                str_release(sc, node->cached_var_ptr->as.string);
                            }
                            node->cached_var_ptr->type = VAL_STRING;
                            node->cached_var_ptr->as.string = str_concat_multi(sc, parts, lens, scount);
                        }
                        for (size_t i = 0; i < rcount; i++) str_release(sc, to_rel[i]);
                        return true;
                    }
                    for (size_t i = 0; i < rcount; i++) str_release(sc, to_rel[i]);
                    return true;
                }

                // UCASE$ fast-path: var$ = UCASE$(expr)
                if (node->right && node->right->type == AST_NODE_STRING_FUNC && node->right->str_func == AST_STR_UCASE) {
                    BValue src_val = eval_ast_evaluate(vm, node->right->left, err);
                    if (err->code == 0 && src_val.type == VAL_STRING && src_val.as.string) {
                        const char *sdata = str_data(src_val.as.string);
                        size_t slen = str_len(src_val.as.string);
                        char stack_buf[256];
                        char *buf = (slen < sizeof(stack_buf)) ? stack_buf : (char *)runtime_malloc(slen + 1);
                        if (buf) {
                            for (size_t i = 0; i < slen; i++) {
                                unsigned char ch = (unsigned char)sdata[i];
                                buf[i] = (char)((ch >= 'a' && ch <= 'z') ? (ch - 32) : ch);
                            }
                            buf[slen] = '\0';
                            BppStringRef cur_s = (node->cached_var_ptr->type == VAL_STRING) ? node->cached_var_ptr->as.string : NULL;
                            node->cached_var_ptr->type = VAL_STRING;
                            node->cached_var_ptr->as.string = str_assign_inplace(sc, cur_s, buf, slen);
                            if (buf != stack_buf) runtime_free(buf);
                        }
                        str_release(sc, src_val.as.string);
                        return true;
                    }
                }

                // LCASE$ fast-path: var$ = LCASE$(expr)
                if (node->right && node->right->type == AST_NODE_STRING_FUNC && node->right->str_func == AST_STR_LCASE) {
                    BValue src_val = eval_ast_evaluate(vm, node->right->left, err);
                    if (err->code == 0 && src_val.type == VAL_STRING && src_val.as.string) {
                        const char *sdata = str_data(src_val.as.string);
                        size_t slen = str_len(src_val.as.string);
                        char stack_buf[256];
                        char *buf = (slen < sizeof(stack_buf)) ? stack_buf : (char *)runtime_malloc(slen + 1);
                        if (buf) {
                            for (size_t i = 0; i < slen; i++) {
                                unsigned char ch = (unsigned char)sdata[i];
                                buf[i] = (char)((ch >= 'A' && ch <= 'Z') ? (ch + 32) : ch);
                            }
                            buf[slen] = '\0';
                            BppStringRef cur_s = (node->cached_var_ptr->type == VAL_STRING) ? node->cached_var_ptr->as.string : NULL;
                            node->cached_var_ptr->type = VAL_STRING;
                            node->cached_var_ptr->as.string = str_assign_inplace(sc, cur_s, buf, slen);
                            if (buf != stack_buf) runtime_free(buf);
                        }
                        str_release(sc, src_val.as.string);
                        return true;
                    }
                }

                // MID$ fast-path: var$ = MID$(expr, start, len)
                if (node->right && node->right->type == AST_NODE_STRING_FUNC && node->right->str_func == AST_STR_MID) {
                    BValue src_val = eval_ast_evaluate(vm, node->right->left, err);
                    if (err->code == 0 && src_val.type == VAL_STRING && src_val.as.string) {
                        BValue start_v = eval_ast_evaluate(vm, node->right->right, err);
                        int start = (int)((start_v.type == VAL_NUMBER || start_v.type == VAL_INTEGER) ? start_v.as.number : 1);
                        int length = -1;
                        if (node->right->condition) {
                            BValue len_v = eval_ast_evaluate(vm, node->right->condition, err);
                            length = (int)((len_v.type == VAL_NUMBER || len_v.type == VAL_INTEGER) ? len_v.as.number : -1);
                        }
                        const char *sdata = str_data(src_val.as.string);
                        size_t total_len = str_len(src_val.as.string);
                        if (total_len == 0 || start == 0) {
                            BppStringRef cur_s = (node->cached_var_ptr->type == VAL_STRING) ? node->cached_var_ptr->as.string : NULL;
                            node->cached_var_ptr->type = VAL_STRING;
                            node->cached_var_ptr->as.string = str_assign_inplace(sc, cur_s, "", 0);
                            str_release(sc, src_val.as.string);
                            return true;
                        }
                        size_t sub_start = 0;
                        if (start < 0) {
                            long long calc_start = (long long)total_len + start + 1;
                            if (calc_start <= 0) calc_start = 1;
                            if ((size_t)calc_start > total_len) {
                                BppStringRef cur_s = (node->cached_var_ptr->type == VAL_STRING) ? node->cached_var_ptr->as.string : NULL;
                                node->cached_var_ptr->type = VAL_STRING;
                                node->cached_var_ptr->as.string = str_assign_inplace(sc, cur_s, "", 0);
                                str_release(sc, src_val.as.string);
                                return true;
                            }
                            sub_start = (size_t)(calc_start - 1);
                        } else {
                            if ((size_t)start > total_len) {
                                BppStringRef cur_s = (node->cached_var_ptr->type == VAL_STRING) ? node->cached_var_ptr->as.string : NULL;
                                node->cached_var_ptr->type = VAL_STRING;
                                node->cached_var_ptr->as.string = str_assign_inplace(sc, cur_s, "", 0);
                                str_release(sc, src_val.as.string);
                                return true;
                            }
                            sub_start = (size_t)(start - 1);
                        }
                        size_t remaining = total_len - sub_start;
                        size_t sub_len = 0;
                        if (length < 0 && node->right->condition) {
                            size_t skip_end = (size_t)(-length);
                            sub_len = (sub_start + skip_end >= total_len) ? 0 : (total_len - sub_start - skip_end);
                        } else {
                            sub_len = (length < 0 || (sub_start + (size_t)length > total_len)) ? remaining : (size_t)length;
                        }
                        BppStringRef cur_s = (node->cached_var_ptr->type == VAL_STRING) ? node->cached_var_ptr->as.string : NULL;
                        node->cached_var_ptr->type = VAL_STRING;
                        node->cached_var_ptr->as.string = str_assign_inplace(sc, cur_s, (sub_len > 0) ? (sdata + sub_start) : "", sub_len);
                        str_release(sc, src_val.as.string);
                        return true;
                    }
                }
            }

            if (!is_str_var) {
                if (!node->cached_var_ptr) {
                    node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
                }
                if (node->cached_var_ptr) {
                    if (node->cached_var_ptr->type == VAL_STRING && node->cached_var_ptr->as.string) {
                        str_release(vm_get_str(vm), node->cached_var_ptr->as.string);
                    }
                    double num = eval_ast_eval_num(vm, node->right, err);
                    if (err->code != 0) return true;
                    if (node->cached_var_ptr->type == VAL_INTEGER) {
                        node->cached_var_ptr->as.number = (double)((int32_t)num);
                    } else {
                        node->cached_var_ptr->type = VAL_NUMBER;
                        node->cached_var_ptr->as.number = num;
                    }
                    return true;
                }
            }

            BValue val = eval_ast_evaluate(vm, node->right, err);
            if (err->code != 0) return true;

            if (node->cached_var_ptr && (val.type == VAL_NUMBER || val.type == VAL_INTEGER)) {
                if (node->cached_var_ptr->type == VAL_STRING && node->cached_var_ptr->as.string) {
                    str_release(vm_get_str(vm), node->cached_var_ptr->as.string);
                    node->cached_var_ptr->type = VAL_NUMBER;
                }
                if (node->cached_var_ptr->type == VAL_INTEGER) {
                    node->cached_var_ptr->as.number = (double)((int32_t)val.as.number);
                } else {
                    node->cached_var_ptr->type = VAL_NUMBER;
                    node->cached_var_ptr->as.number = val.as.number;
                }
            } else if (node->cached_var_ptr && val.type == VAL_STRING) {
                if (node->cached_var_ptr->type == VAL_STRING && node->cached_var_ptr->as.string) {
                    str_release(vm_get_str(vm), node->cached_var_ptr->as.string);
                }
                node->cached_var_ptr->type = VAL_STRING;
                node->cached_var_ptr->as.string = val.as.string;
            } else {
                var_assign(vm_get_var(vm), node->var_name, val);
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
            }
            return true;
        }
        case AST_NODE_ARRAY_ASSIGN: {
            if (!node->cached_arr) {
                node->cached_arr = arr_find_entry(vm_get_arr(vm), node->var_name);
            }
            ArrayEntry *arr = (ArrayEntry *)node->cached_arr;
            int idx = (int)eval_ast_eval_num(vm, node->index_expr, err);
            if (err->code != 0) return true;

            int base = arr_get_option_base(vm_get_arr(vm));
            size_t nlen = runtime_strlen(node->var_name);
            bool is_str_arr = (nlen > 0 && node->var_name[nlen - 1] == '$');
            if (!is_str_arr && arr && arr->num_dims == 1 && idx >= base && idx <= arr->bounds[0]) {
                int flat = idx - base;
                BValue *elem = &arr->elements[flat];
                if (elem->type != VAL_STRING) {
                    elem->type = VAL_NUMBER;
                    elem->as.number = eval_ast_eval_num(vm, node->right, err);
                    return true;
                }
            }
            BValue val = eval_ast_evaluate(vm, node->right, err);
            if (err->code != 0) return true;
            int indices[1] = { idx };
            BValue *elem = arr_get_element(vm_get_arr(vm), node->var_name, 1, indices, err);
            if (err->code == 0 && elem) {
                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                *elem = val;
            }
            return true;
        }
        case AST_NODE_ARRAY2D_ASSIGN: {
            if (!node->cached_arr) {
                node->cached_arr = arr_find_entry(vm_get_arr(vm), node->var_name);
            }
            ArrayEntry *arr = (ArrayEntry *)node->cached_arr;
            int idx1 = (int)eval_ast_eval_num(vm, node->index_expr, err);
            if (err->code != 0) return true;
            int idx2 = (int)eval_ast_eval_num(vm, node->index2_expr, err);
            if (err->code != 0) return true;

            int base = arr_get_option_base(vm_get_arr(vm));
            size_t nlen = runtime_strlen(node->var_name);
            bool is_str_arr = (nlen > 0 && node->var_name[nlen - 1] == '$');
            if (!is_str_arr && arr && arr->num_dims == 2 && idx1 >= base && idx1 <= arr->bounds[0] && idx2 >= base && idx2 <= arr->bounds[1]) {
                int flat = (idx1 - base) * (arr->bounds[1] - base + 1) + (idx2 - base);
                BValue *elem = &arr->elements[flat];
                if (elem->type != VAL_STRING) {
                    elem->type = VAL_NUMBER;
                    elem->as.number = eval_ast_eval_num(vm, node->right, err);
                    return true;
                }
            }
            BValue val = eval_ast_evaluate(vm, node->right, err);
            if (err->code != 0) return true;
            int indices[2] = { idx1, idx2 };
            BValue *elem = arr_get_element(vm_get_arr(vm), node->var_name, 2, indices, err);
            if (err->code == 0 && elem) {
                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                *elem = val;
            }
            return true;
        }
        default:
            return false;
    }
}

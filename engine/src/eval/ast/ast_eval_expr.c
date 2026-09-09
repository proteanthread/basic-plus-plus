// FILENAME: ast_eval_expr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, file.h, num_format.h, strings.h, variables.h)
// NEEDS: libengine (ast_internal.h, bios.h, eval_internal.h)
// Core AST expression evaluation entry point and dispatcher.
//
// ---- Includes ----

#include "bios/bios.h"
#include "device/bus.h"
#include "eval/ast_internal.h"
#include "eval/eval.h"
#include "eval/eval_internal.h"
#include "eval/functions/network/func_ip.h"
#include "eval/functions/system/time/func_tim.h"
#include "runtime/arrays.h"
#include "runtime/file.h"
#include "runtime/map.h"
#include "runtime/num_format.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "runtime/variables_internal.h"
#include "statements/oop/sub.h"
#include "eval/functions/math/random/rnd.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/math/math.h"

static inline BValue resolve_field_string(VMContext *vm, BValue val) {
    if (val.type != VAL_FIELD_STRING) return val;
    int ch = val.as.field_str.channel;
    unsigned char *rec_buf = file_get_record_buffer(vm_get_file(vm), ch);
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;
    if (rec_buf) {
        res.as.string = str_create(vm_get_str(vm), (const char *)(rec_buf + val.as.field_str.offset), (size_t)val.as.field_str.length);
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

// evaluates an AST expression node recursively returning a BValue result
BValue eval_ast_evaluate(VMContext *vm, EvalAstNode *node, BppError *err) {
    BValue null_val;
    runtime_memset(&null_val, 0, sizeof(null_val));
    if (!vm || !node) return null_val;

    switch (node->type) {
        case AST_NODE_LITERAL: {
            if (node->val.type == VAL_STRING && node->val.as.string) {
                str_add_ref(node->val.as.string);
            }
            return node->val;
        }
        case AST_NODE_VARIABLE: {
            BValue builtin_val;
            if (eval_try_resolve_builtin_constant_or_system_var(vm, node->var_name, &builtin_val)) {
                return builtin_val;
            }

            if (node->cached_var_ptr) {
                BValue res = *node->cached_var_ptr;
                if (res.type == VAL_FIELD_STRING) {
                    res = resolve_field_string(vm, res);
                } else if (res.type == VAL_STRING && res.as.string) {
                    str_add_ref(res.as.string);
                }
                return res;
            }

            VariableContext *vc = vm_get_var(vm);
            DynamicVarEntry *dvar = var_find_dynamic(vc, node->var_name);
            if (dvar) {
                if (dvar->getter) {
                    return dvar->getter(vm, node->var_name, dvar->user_data);
                }
                if (dvar->read_fn[0] != '\0') {
                    return invoke_user_function(vm, dvar->read_fn, NULL, 0, err);
                }
            }

            // Bare IP$ without parentheses
            if (runtime_strcasecmp(node->var_name, "IP$") == 0) {
                return func_ip_eval(vm, "IP$", 0, NULL, err);
            }

            // Bare TIM without parentheses
            if (runtime_strcasecmp(node->var_name, "TIM") == 0) {
                return func_tim_eval(vm, "TIM", 0, NULL, err);
            }

            // Bare ALARM without parentheses
            if (runtime_strcasecmp(node->var_name, "ALARM") == 0) {
                BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
                return res;
            }

            // Bare ALARM$ without parentheses
            if (runtime_strcasecmp(node->var_name, "ALARM$") == 0) {
                BValue res = { .type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0) };
                return res;
            }

            const char *dot_pos = runtime_strchr(node->var_name, '.');
            if (dot_pos) {
                char base_name[128];
                size_t blen = (size_t)(dot_pos - node->var_name);
                if (blen < sizeof(base_name)) {
                    runtime_memcpy(base_name, node->var_name, blen);
                    base_name[blen] = '\0';
                    const char *field_name = dot_pos + 1;
                    BValue *base_val = var_lookup(vc, base_name, false);
                    if (base_val && base_val->type == VAL_MAP && base_val->as.map) {
                        BValue prop_val;
                        if (map_get(base_val->as.map, field_name, &prop_val)) {
                            if (prop_val.type == VAL_STRING && prop_val.as.string) {
                                str_add_ref(prop_val.as.string);
                            } else if (prop_val.type == VAL_MAP && prop_val.as.map) {
                                map_add_ref(prop_val.as.map);
                            }
                            return prop_val;
                        }
                    }
                }
            }

            BValue *ptr = var_lookup(vc, node->var_name, false);
            if (ptr && ptr->type != VAL_NONE) {
                node->cached_var_ptr = ptr;
                BValue res = *ptr;
                if (res.type == VAL_FIELD_STRING) {
                    res = resolve_field_string(vm, res);
                } else if (res.type == VAL_STRING && res.as.string) {
                    str_add_ref(res.as.string);
                }
                return res;
            }

            ptr = var_lookup(vc, node->var_name, true);
            if (ptr) {
                node->cached_var_ptr = ptr;
                if (ptr->type == VAL_NONE) {
                    ptr->type = VAL_NUMBER;
                    ptr->as.number = 0.0;
                }
                BValue res = *ptr;
                if (res.type == VAL_FIELD_STRING) {
                    res = resolve_field_string(vm, res);
                } else if (res.type == VAL_STRING && res.as.string) {
                    str_add_ref(res.as.string);
                }
                return res;
            }
            BValue zero = { .type = VAL_NUMBER, .as.number = 0.0 };
            return zero;
        }
        case AST_NODE_MATH_FUNC: {
            BValue arg = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            double x = (arg.type == VAL_NUMBER || arg.type == VAL_INTEGER) ? arg.as.number : 0.0;
            double ans = 0.0;
            int mode = vm_get_angle_mode(vm);
            double in_x = x;
            if (node->math_func == AST_MATH_SIN || node->math_func == AST_MATH_COS || node->math_func == AST_MATH_TAN) {
                if (mode == 1) in_x = x * (3.14159265358979323846 / 180.0);
                else if (mode == 2) in_x = x * (3.14159265358979323846 / 200.0);
            }
            switch (node->math_func) {
                case AST_MATH_SIN: ans = runtime_sin(in_x); break;
                case AST_MATH_COS: ans = runtime_cos(in_x); break;
                case AST_MATH_TAN: ans = runtime_tan(in_x); break;
                case AST_MATH_SQR:
                    if (x < 0.0) { err->code = 5; err->message = "Illegal function call in SQR"; return null_val; }
                    ans = runtime_sqrt(x);
                    break;
                case AST_MATH_ABS: ans = runtime_fabs(x); break;
                case AST_MATH_INT: ans = runtime_floor(x); break;
                case AST_MATH_FIX: ans = (x >= 0.0) ? runtime_floor(x) : runtime_ceil(x); break;
                case AST_MATH_EXP: ans = runtime_exp(x); break;
                case AST_MATH_LOG:
                    if (x <= 0.0) { err->code = 5; err->message = "Illegal function call in LOG"; return null_val; }
                    ans = runtime_log(x);
                    break;
                case AST_MATH_ATN:
                    ans = runtime_atan(x);
                    if (mode == 1) ans *= (180.0 / 3.14159265358979323846);
                    else if (mode == 2) ans *= (200.0 / 3.14159265358979323846);
                    break;
                case AST_MATH_SGN: ans = (x > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0); break;
                case AST_MATH_RND: {
                    BValue arg_val = { .type = VAL_NUMBER, .as.number = x };
                    BValue r = func_rnd_eval(vm, "RND", 1, &arg_val, err);
                    ans = r.as.number;
                    break;
                }
                case AST_MATH_EOF: {
                    int ch = (int)x;
                    ans = file_eof(vm_get_file(vm), ch) ? -1.0 : 0.0;
                    break;
                }
            }
            BValue res = { .type = VAL_NUMBER, .as.number = ans };
            return res;
        }
        case AST_NODE_STRING_FUNC: {
            return eval_ast_eval_str_func(vm, node, err);
        }
        case AST_NODE_ARRAY_READ: {
            if (!node->cached_arr) {
                node->cached_arr = arr_find_entry(vm_get_arr(vm), node->var_name);
            }
            ArrayEntry *arr = (ArrayEntry *)node->cached_arr;
            int idx = 0;
            if (node->index_expr->type == AST_NODE_VARIABLE) {
                if (!node->index_expr->cached_var_ptr) node->index_expr->cached_var_ptr = var_lookup(vm_get_var(vm), node->index_expr->var_name, false);
                BValue *vp = node->index_expr->cached_var_ptr;
                idx = (int)(vp ? vp->as.number : 0);
            } else if (node->index_expr->type == AST_NODE_LITERAL) {
                idx = (int)node->index_expr->val.as.number;
            } else {
                BValue ival = eval_ast_evaluate(vm, node->index_expr, err);
                if (err->code != 0) return null_val;
                idx = (int)((ival.type == VAL_NUMBER || ival.type == VAL_INTEGER) ? ival.as.number : 0);
            }
            int base = arr_get_option_base(vm_get_arr(vm));
            if (arr && arr->num_dims == 1 && idx >= base && idx <= arr->bounds[0]) {
                int flat = idx - base;
                BValue res = arr->elements[flat];
                if (res.type == VAL_STRING && res.as.string) str_add_ref(res.as.string);
                return res;
            }
            int indices[1] = { idx };
            BValue *elem = arr_get_element(vm_get_arr(vm), node->var_name, 1, indices, err);
            if (!elem || err->code != 0) return null_val;
            BValue res = *elem;
            if (res.type == VAL_STRING && res.as.string) {
                str_add_ref(res.as.string);
            }
            return res;
        }
        case AST_NODE_ARRAY2D_READ: {
            if (!node->cached_arr) {
                node->cached_arr = arr_find_entry(vm_get_arr(vm), node->var_name);
            }
            ArrayEntry *arr = (ArrayEntry *)node->cached_arr;
            int idx1 = 0, idx2 = 0;
            if (node->index_expr->type == AST_NODE_VARIABLE) {
                if (!node->index_expr->cached_var_ptr) node->index_expr->cached_var_ptr = var_lookup(vm_get_var(vm), node->index_expr->var_name, false);
                BValue *vp1 = node->index_expr->cached_var_ptr;
                idx1 = (int)(vp1 ? vp1->as.number : 0);
            } else if (node->index_expr->type == AST_NODE_LITERAL) {
                idx1 = (int)node->index_expr->val.as.number;
            } else {
                BValue ival1 = eval_ast_evaluate(vm, node->index_expr, err);
                if (err->code != 0) return null_val;
                idx1 = (int)((ival1.type == VAL_NUMBER || ival1.type == VAL_INTEGER) ? ival1.as.number : 0);
            }

            if (node->index2_expr->type == AST_NODE_VARIABLE) {
                if (!node->index2_expr->cached_var_ptr) node->index2_expr->cached_var_ptr = var_lookup(vm_get_var(vm), node->index2_expr->var_name, false);
                BValue *vp2 = node->index2_expr->cached_var_ptr;
                idx2 = (int)(vp2 ? vp2->as.number : 0);
            } else if (node->index2_expr->type == AST_NODE_LITERAL) {
                idx2 = (int)node->index2_expr->val.as.number;
            } else {
                BValue ival2 = eval_ast_evaluate(vm, node->index2_expr, err);
                if (err->code != 0) return null_val;
                idx2 = (int)((ival2.type == VAL_NUMBER || ival2.type == VAL_INTEGER) ? ival2.as.number : 0);
            }

            int base = arr_get_option_base(vm_get_arr(vm));
            if (arr && arr->num_dims == 2 && idx1 >= base && idx1 <= arr->bounds[0] && idx2 >= base && idx2 <= arr->bounds[1]) {
                int flat = (idx1 - base) * (arr->bounds[1] - base + 1) + (idx2 - base);
                BValue res = arr->elements[flat];
                if (res.type == VAL_STRING && res.as.string) str_add_ref(res.as.string);
                return res;
            }
            int indices[2] = { idx1, idx2 };
            BValue *elem = arr_get_element(vm_get_arr(vm), node->var_name, 2, indices, err);
            if (!elem || err->code != 0) return null_val;
            BValue res = *elem;
            if (res.type == VAL_STRING && res.as.string) {
                str_add_ref(res.as.string);
            }
            return res;
        }
        case AST_NODE_BINARY_OP: {
            if ((node->op == TOK_EQ || node->op == TOK_NE || node->op == TOK_LT ||
                 node->op == TOK_LE || node->op == TOK_GT || node->op == TOK_GE) &&
                (is_ast_string_node(node->left) || is_ast_string_node(node->right))) {
                BValue s1 = eval_ast_evaluate(vm, node->left, err);
                if (err->code != 0) return null_val;
                BValue s2 = eval_ast_evaluate(vm, node->right, err);
                if (err->code != 0) {
                    if (s1.type == VAL_STRING && s1.as.string) str_release(vm_get_str(vm), s1.as.string);
                    return null_val;
                }
                const char *str1 = (s1.type == VAL_STRING && s1.as.string) ? str_data(s1.as.string) : "";
                const char *str2 = (s2.type == VAL_STRING && s2.as.string) ? str_data(s2.as.string) : "";
                int cmp = runtime_strcmp(str1, str2);
                if (s1.type == VAL_STRING && s1.as.string) str_release(vm_get_str(vm), s1.as.string);
                if (s2.type == VAL_STRING && s2.as.string) str_release(vm_get_str(vm), s2.as.string);

                double ans = 0.0;
                switch (node->op) {
                    case TOK_EQ: ans = (cmp == 0) ? -1.0 : 0.0; break;
                    case TOK_NE: ans = (cmp != 0) ? -1.0 : 0.0; break;
                    case TOK_LT: ans = (cmp < 0)  ? -1.0 : 0.0; break;
                    case TOK_LE: ans = (cmp <= 0) ? -1.0 : 0.0; break;
                    case TOK_GT: ans = (cmp > 0)  ? -1.0 : 0.0; break;
                    case TOK_GE: ans = (cmp >= 0) ? -1.0 : 0.0; break;
                    default: break;
                }
                BValue res = { .type = VAL_NUMBER, .as.number = ans };
                return res;
            }

            if (node->op == TOK_PLUS && (is_ast_string_node(node->left) || is_ast_string_node(node->right))) {
                StringContext *sc = vm_get_str(vm);
                const char *parts[16];
                size_t lens[16];
                char num_bufs[16][64];
                size_t num_buf_idx = 0;
                BppString *to_rel[16];
                size_t scount = 0, rcount = 0;

                eval_ast_flatten_concat(vm, node->left, parts, lens, num_bufs, &num_buf_idx, to_rel, &scount, &rcount, 16, err);
                if (err->code == 0) {
                    eval_ast_flatten_concat(vm, node->right, parts, lens, num_bufs, &num_buf_idx, to_rel, &scount, &rcount, 16, err);
                }
                if (err->code != 0) {
                    for (size_t i = 0; i < rcount; i++) str_release(sc, to_rel[i]);
                    return null_val;
                }

                BppString *res_str = str_concat_multi(sc, parts, lens, scount);
                for (size_t i = 0; i < rcount; i++) {
                    str_release(sc, to_rel[i]);
                }
                BValue res = { .type = VAL_STRING, .as.string = res_str };
                return res;
            }

            BValue v1 = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            BValue v2 = eval_ast_evaluate(vm, node->right, err);
            if (err->code != 0) return null_val;

            if (v1.type == VAL_SET || v2.type == VAL_SET || v1.type == VAL_GROUP || v2.type == VAL_GROUP || v1.type == VAL_MAP || v2.type == VAL_MAP) {
                BValue stack[2] = { v1, v2 };
                size_t val_ptr = 2;
                if (eval_execute_op(vm, node->op, stack, &val_ptr, err)) {
                    return stack[0];
                }
                return null_val;
            }

            double n1 = (v1.type == VAL_NUMBER || v1.type == VAL_INTEGER) ? v1.as.number : 0.0;
            double n2 = (v2.type == VAL_NUMBER || v2.type == VAL_INTEGER) ? v2.as.number : 0.0;
            if (node->op == TOK_DIV && n2 == 0.0) {
                err->code = 11; err->message = "Division by zero";
                return null_val;
            }
            double ans = eval_ast_calc_binary_op(node->op, n1, n2);
            BValue res = { .type = VAL_NUMBER, .as.number = ans };
            return res;
        }
        case AST_NODE_UNARY_OP: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            if (node->op == TOK_UNARY_MINUS) {
                if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                    val.as.number = -val.as.number;
                }
            } else if (node->op == TOK_NOT) {
                if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                    if (val.as.number == 1.0) {
                        val.as.number = 0.0;
                    } else if (val.as.number == -1.0) {
                        val.as.number = 0.0;
                    } else {
                        val.as.number = (double)(~(int64_t)val.as.number);
                    }
                }
            }
            return val;
        }
        default:
            return null_val;
    }
}

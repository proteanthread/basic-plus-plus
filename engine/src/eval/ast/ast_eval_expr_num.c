// FILENAME: ast_eval_expr_num.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast_eval_expr.c, ast_eval_stmt.c)
// NEEDS: libcore, libengine (ast_internal.h)
// Fast scalar numeric and binary operator evaluation for AST nodes.
//
// ---- Includes ----

#include "eval/ast_internal.h"
#include "runtime/math/math.h"
#include "runtime/variables.h"
#include "runtime/variables_internal.h"
#include "runtime/arrays.h"
#include "eval/functions/math/random/rnd.h"

// calculates the scalar result of a binary arithmetic or logical operation
double eval_ast_calc_binary_op(BppTokenType op, double n1, double n2) {
    switch (op) {
        case TOK_PLUS: return n1 + n2;
        case TOK_MINUS: return n1 - n2;
        case TOK_MUL: return n1 * n2;
        case TOK_DIV: return (n2 != 0.0) ? (n1 / n2) : 0.0;
        case TOK_POW: return runtime_pow(n1, n2);
        case TOK_MOD: return ((int64_t)n2 != 0) ? (double)((int64_t)n1 % (int64_t)n2) : 0.0;
        case TOK_BACKSLASH: return ((int64_t)n2 != 0) ? (double)((int64_t)n1 / (int64_t)n2) : 0.0;
        case TOK_LT: return (n1 < n2) ? -1.0 : 0.0;
        case TOK_LE: return (n1 <= n2) ? -1.0 : 0.0;
        case TOK_GT: return (n1 > n2) ? -1.0 : 0.0;
        case TOK_GE: return (n1 >= n2) ? -1.0 : 0.0;
        case TOK_EQ: return (n1 == n2) ? -1.0 : 0.0;
        case TOK_NE: return (n1 != n2) ? -1.0 : 0.0;
        case TOK_AND: return (double)((int64_t)n1 & (int64_t)n2);
        case TOK_OR: return (double)((int64_t)n1 | (int64_t)n2);
        case TOK_XOR: return (double)((int64_t)n1 ^ (int64_t)n2);
        case TOK_IMP: return (double)(~((int64_t)n1) | (int64_t)n2);
        case TOK_EQV: return (double)(~((int64_t)n1 ^ (int64_t)n2));
        default: return 0.0;
    }
}

// fast scalar numeric AST evaluator directly returning double in CPU register
double eval_ast_eval_num(VMContext *vm, EvalAstNode *node, BppError *err) {
    if (!node) return 0.0;
    switch (node->type) {
        case AST_NODE_VARIABLE: {
            if (node->cached_var_ptr) {
                if (node->cached_var_ptr->type == VAL_NUMBER || node->cached_var_ptr->type == VAL_INTEGER) {
                    return node->cached_var_ptr->as.number;
                }
                return 0.0;
            }
            BValue bval;
            if (eval_try_resolve_builtin_constant_or_system_var(vm, node->var_name, &bval)) {
                return (bval.type == VAL_NUMBER || bval.type == VAL_INTEGER) ? bval.as.number : 0.0;
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
                        BValue prop_val;
                        if (map_get(base_val->as.map, field_name, &prop_val)) {
                            return (prop_val.type == VAL_NUMBER || prop_val.type == VAL_INTEGER) ? prop_val.as.number : 0.0;
                        }
                    }
                }
            }

            VariableContext *vc = vm_get_var(vm);
            BValue *ptr = var_lookup(vc, node->var_name, false);
            if (ptr && ptr->type != VAL_NONE) {
                node->cached_var_ptr = ptr;
                if (ptr->type == VAL_NUMBER || ptr->type == VAL_INTEGER) {
                    return ptr->as.number;
                }
                return 0.0;
            }
            return 0.0;
        }
        case AST_NODE_LITERAL:
            return node->val.as.number;
        case AST_NODE_BINARY_OP: {
            BValue res = eval_ast_evaluate(vm, node, err);
            if (err->code != 0) return 0.0;
            return (res.type == VAL_NUMBER || res.type == VAL_INTEGER) ? res.as.number : 0.0;
        }
        case AST_NODE_MATH_FUNC: {
            double x = eval_ast_eval_num(vm, node->left, err);
            int mode = vm_get_angle_mode(vm);
            double in_x = x;
            if (node->math_func == AST_MATH_SIN || node->math_func == AST_MATH_COS || node->math_func == AST_MATH_TAN) {
                if (mode == 1) in_x = x * (3.14159265358979323846 / 180.0);
                else if (mode == 2) in_x = x * (3.14159265358979323846 / 200.0);
            }
            switch (node->math_func) {
                case AST_MATH_SIN: return runtime_sin(in_x);
                case AST_MATH_COS: return runtime_cos(in_x);
                case AST_MATH_TAN: return runtime_tan(in_x);
                case AST_MATH_SQR: return (x >= 0.0) ? runtime_sqrt(x) : 0.0;
                case AST_MATH_ABS: return runtime_fabs(x);
                case AST_MATH_INT: return runtime_floor(x);
                case AST_MATH_FIX: return (x >= 0.0) ? runtime_floor(x) : runtime_ceil(x);
                case AST_MATH_EXP: return runtime_exp(x);
                case AST_MATH_LOG: return (x > 0.0) ? runtime_log(x) : 0.0;
                case AST_MATH_ATN: {
                    double ans = runtime_atan(x);
                    if (mode == 1) ans *= (180.0 / 3.14159265358979323846);
                    else if (mode == 2) ans *= (200.0 / 3.14159265358979323846);
                    return ans;
                }
                case AST_MATH_SGN: return (x > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0);
                case AST_MATH_RND: {
                    BValue arg_val = { .type = VAL_NUMBER, .as.number = x };
                    BValue r = func_rnd_eval(vm, "RND", 1, &arg_val, err);
                    return r.as.number;
                }
                default: break;
            }
            break;
        }
        case AST_NODE_ARRAY_READ: {
            if (!node->cached_arr) node->cached_arr = arr_find_entry(vm_get_arr(vm), node->var_name);
            ArrayEntry *arr = (ArrayEntry *)node->cached_arr;
            int idx = (int)eval_ast_eval_num(vm, node->index_expr, err);
            int base = arr_get_option_base(vm_get_arr(vm));
            if (arr && arr->num_dims == 1 && idx >= base && idx <= arr->bounds[0]) {
                return arr->elements[idx - base].as.number;
            }
            break;
        }
        case AST_NODE_ARRAY2D_READ: {
            if (!node->cached_arr) node->cached_arr = arr_find_entry(vm_get_arr(vm), node->var_name);
            ArrayEntry *arr = (ArrayEntry *)node->cached_arr;
            int idx1 = (int)eval_ast_eval_num(vm, node->index_expr, err);
            int idx2 = (int)eval_ast_eval_num(vm, node->index2_expr, err);
            int base = arr_get_option_base(vm_get_arr(vm));
            if (arr && arr->num_dims == 2 && idx1 >= base && idx1 <= arr->bounds[0] && idx2 >= base && idx2 <= arr->bounds[1]) {
                int flat = (idx1 - base) * (arr->bounds[1] - base + 1) + (idx2 - base);
                return arr->elements[flat].as.number;
            }
            break;
        }
        case AST_NODE_UNARY_OP: {
            double v = eval_ast_eval_num(vm, node->left, err);
            if (node->op == TOK_MINUS || node->op == TOK_UNARY_MINUS) return -v;
            if (node->op == TOK_NOT) return (v == 1.0 || v == -1.0) ? 0.0 : (double)(~((int64_t)v));
            return v;
        }
        default:
            break;
    }
    BValue val = eval_ast_evaluate(vm, node, err);
    return (val.type == VAL_NUMBER || val.type == VAL_INTEGER) ? val.as.number : 0.0;
}

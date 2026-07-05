/* =====================================================================
 * BASIC++ Interpreter — AST Interpreter Implementation
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "ast_interpreter.h"
#include "vm.h"
#include "errors.h"
#include "funcreg.h"
#include "fileio.h"
#include "memmap.h"
#include "io/vfs.h"
#include "../console.h"
#include "../parser.h"

// Forward declaration of internal expression parser helper if needed
extern BValue parse_expression_bval_internal(Lexer *lex, RuntimeState *rt, int line_num, int precedence_level);

#define MAX_EVAL_FRAMES 256

typedef struct {
    AstExpr *expr;
    int step;
} EvalFrame;

// Helper to resolve user function name
static UserFunction *find_user_fn(RuntimeState *rt, char letter) {
    char name_buf[2];
    name_buf[0] = letter;
    name_buf[1] = '\0';
    return runtime_find_fn(rt, name_buf, 1);
}

BValue ast_call_function(RuntimeState *rt, AstFuncId func_id, BValue *args, int arg_count, char fn_letter, int builtin_kw, int line_num) {
    if (func_id == FUNC_FN_USER) {
        UserFunction *ufn = find_user_fn(rt, fn_letter);
        if (!ufn) {
            error_raise(ERR_HOW, line_num);
            return bval_int(0);
        }
        BValue saved[MAX_FN_PARAMS];
        int pi;
        int ac = (arg_count < ufn->param_count) ? arg_count : ufn->param_count;
        for (pi = 0; pi < ufn->param_count; pi++) {
            int vi = ufn->params[pi] - 'A';
            saved[pi] = rt->variables[vi];
            if (pi < ac) {
                rt->variables[vi] = args[pi];
            }
        }
        Lexer bl;
        lexer_init(&bl, ufn->body);
        BValue res = parse_expression_bval_internal(&bl, rt, line_num, 0);
        for (pi = 0; pi < ufn->param_count; pi++) {
            int vi = ufn->params[pi] - 'A';
            rt->variables[vi] = saved[pi];
        }
        return res;
    }

    if (func_id == FUNC_BUILTIN) {
        const FunctionEntry *fn = funcreg_find_by_keyword((KeywordId)builtin_kw);
        if (fn && fn->handler) {
            return fn->handler(args, arg_count, rt);
        }
        error_raise(ERR_HOW, line_num);
        return bval_int(0);
    }

    // Standard built-ins from vm_exec
    if (arg_count == 0) {
        BValue dummy = bval_int(0);
        return exec_func1((int)func_id, &dummy, line_num, rt);
    } else if (arg_count == 1) {
        return exec_func1((int)func_id, &args[0], line_num, rt);
    } else if (arg_count == 2) {
        return exec_func2((int)func_id, &args[0], &args[1], line_num, rt);
    } else if (arg_count == 3) {
        return exec_func3((int)func_id, &args[0], &args[1], &args[2], line_num, rt);
    }
    return bval_int(0);
}

int ast_eval_expr_nonrec(RuntimeState *rt, AstExpr *root, int line_num, BValue *out_val) {
    if (!root) {
        *out_val = bval_int(0);
        return 0;
    }
    EvalFrame stack[MAX_EVAL_FRAMES];
    int top = -1;

    top++;
    stack[top].expr = root;
    stack[top].step = 0;

    while (top >= 0) {
        AstExpr *curr = stack[top].expr;
        int step = stack[top].step;

        if (curr->type == EXPR_INT_LIT) {
            vm_eval_push(&rt->eval_stack, bval_int(curr->v.ival));
            top--;
            continue;
        }
        if (curr->type == EXPR_FLOAT_LIT) {
            vm_eval_push(&rt->eval_stack, bval_float(curr->v.fval));
            top--;
            continue;
        }
        if (curr->type == EXPR_STRING_LIT) {
            long slen = curr->v.sval.length;
            char *pooled = strpool_alloc(&rt->strpool, slen);
            if (pooled) {
                memcpy(pooled, curr->v.sval.data, (size_t)slen);
                vm_eval_push(&rt->eval_stack, bval_string(pooled, slen));
            } else {
                vm_eval_push(&rt->eval_stack, bval_string(NULL, 0));
            }
            top--;
            continue;
        }
        if (curr->type == EXPR_VAR) {
            BValue val;
            int idx = curr->v.var_name - 'A';
            if (idx >= 0 && idx < 26) {
                val = runtime_get_var_bval(rt, curr->v.var_name);
            } else {
                val = bval_int(0);
            }
            vm_eval_push(&rt->eval_stack, val);
            top--;
            continue;
        }
        if (curr->type == EXPR_STRING_VAR) {
            BValue val = runtime_get_string_var(rt, curr->v.var_name);
            vm_eval_push(&rt->eval_stack, val);
            top--;
            continue;
        }
        if (curr->type == EXPR_NAMED_VAR) {
            BValue val = runtime_get_named_var_bval(rt, curr->v.named.name, curr->v.named.name_len);
            vm_eval_push(&rt->eval_stack, val);
            top--;
            continue;
        }

        if (curr->type == EXPR_BINOP) {
            if (step == 0) {
                stack[top].step = 1;
                if (top >= MAX_EVAL_FRAMES - 1) { error_raise(ERR_SORRY, line_num); return -1; }
                top++;
                stack[top].expr = curr->v.binop.left;
                stack[top].step = 0;
            } else if (step == 1) {
                stack[top].step = 2;
                if (top >= MAX_EVAL_FRAMES - 1) { error_raise(ERR_SORRY, line_num); return -1; }
                top++;
                stack[top].expr = curr->v.binop.right;
                stack[top].step = 0;
            } else {
                BValue right = vm_eval_pop(&rt->eval_stack);
                BValue left = vm_eval_pop(&rt->eval_stack);
                BValue res;
                int cmp = 0;
                if (curr->v.binop.op >= BOP_EQ && curr->v.binop.op <= BOP_GE) {
                    cmp = bval_compare(&left, &right, line_num);
                }
                switch (curr->v.binop.op) {
                    case BOP_ADD: res = bval_add(&left, &right, line_num); break;
                    case BOP_SUB: res = bval_sub(&left, &right, line_num); break;
                    case BOP_MUL: res = bval_mul(&left, &right, line_num); break;
                    case BOP_DIV: res = bval_div(&left, &right, line_num); break;
                    case BOP_MOD: res = bval_mod(&left, &right, line_num); break;
                    case BOP_EQ:  res = bval_int(cmp == 0 ? -1 : 0); break;
                    case BOP_NE:  res = bval_int(cmp != 0 ? -1 : 0); break;
                    case BOP_LT:  res = bval_int(cmp < 0 ? -1 : 0); break;
                    case BOP_GT:  res = bval_int(cmp > 0 ? -1 : 0); break;
                    case BOP_LE:  res = bval_int(cmp <= 0 ? -1 : 0); break;
                    case BOP_GE:  res = bval_int(cmp >= 0 ? -1 : 0); break;
                    case BOP_AND: res = bval_int((bval_to_int(&left) != 0 && bval_to_int(&right) != 0) ? -1 : 0); break;
                    case BOP_OR:  res = bval_int((bval_to_int(&left) != 0 || bval_to_int(&right) != 0) ? -1 : 0); break;
                    case BOP_CONCAT: res = bval_concat(&left, &right, line_num, &rt->strpool); break;
                    default: res = bval_int(0); break;
                }
                vm_eval_push(&rt->eval_stack, res);
                top--;
            }
            continue;
        }

        if (curr->type == EXPR_UNOP) {
            if (step == 0) {
                stack[top].step = 1;
                if (top >= MAX_EVAL_FRAMES - 1) { error_raise(ERR_SORRY, line_num); return -1; }
                top++;
                stack[top].expr = curr->v.unop.operand;
                stack[top].step = 0;
            } else {
                BValue val = vm_eval_pop(&rt->eval_stack);
                BValue res;
                if (curr->v.unop.op == UOP_NEG) {
                    res = bval_neg(&val, line_num);
                } else if (curr->v.unop.op == UOP_NOT) {
                    res = bval_int(bval_to_int(&val) == 0 ? -1 : 0);
                } else {
                    res = bval_int(0);
                }
                vm_eval_push(&rt->eval_stack, res);
                top--;
            }
            continue;
        }

        if (curr->type == EXPR_ARRAY_AT) {
            if (step == 0) {
                stack[top].step = 1;
                if (top >= MAX_EVAL_FRAMES - 1) { error_raise(ERR_SORRY, line_num); return -1; }
                top++;
                stack[top].expr = curr->v.array_at.index;
                stack[top].step = 0;
            } else {
                BValue idx = vm_eval_pop(&rt->eval_stack);
                long index = bval_to_int(&idx);
                BValue val = bval_int(runtime_get_array(rt, index));
                vm_eval_push(&rt->eval_stack, val);
                top--;
            }
            continue;
        }

        if (curr->type == EXPR_DIM_ACCESS) {
            int total_args = curr->v.dim_access.idx2 ? 2 : 1;
            if (step < total_args) {
                stack[top].step = step + 1;
                if (top >= MAX_EVAL_FRAMES - 1) { error_raise(ERR_SORRY, line_num); return -1; }
                top++;
                stack[top].expr = (step == 0) ? curr->v.dim_access.idx1 : curr->v.dim_access.idx2;
                stack[top].step = 0;
            } else {
                long idx1 = 0, idx2 = 0;
                if (total_args == 2) {
                    BValue v2 = vm_eval_pop(&rt->eval_stack);
                    BValue v1 = vm_eval_pop(&rt->eval_stack);
                    idx1 = bval_to_int(&v1);
                    idx2 = bval_to_int(&v2);
                } else {
                    BValue v1 = vm_eval_pop(&rt->eval_stack);
                    idx1 = bval_to_int(&v1);
                }
                BValue val = runtime_get_dim(rt, curr->v.dim_access.name, curr->v.dim_access.name_len, (int)idx1, (int)idx2, 0, line_num);
                vm_eval_push(&rt->eval_stack, val);
                top--;
            }
            continue;
        }

        if (curr->type == EXPR_FUNC_CALL) {
            int arg_count = curr->v.func_call.arg_count;
            if (step < arg_count) {
                stack[top].step = step + 1;
                if (top >= MAX_EVAL_FRAMES - 1) { error_raise(ERR_SORRY, line_num); return -1; }
                top++;
                stack[top].expr = curr->v.func_call.args[step];
                stack[top].step = 0;
            } else {
                BValue args[3];
                int a;
                for (a = arg_count - 1; a >= 0; a--) {
                    args[a] = vm_eval_pop(&rt->eval_stack);
                }
                BValue res = ast_call_function(rt, curr->v.func_call.func, args, arg_count, curr->v.func_call.fn_letter, curr->v.func_call.builtin_kw, line_num);
                vm_eval_push(&rt->eval_stack, res);
                top--;
            }
            continue;
        }

        top--;
    }

    if (vm_eval_depth(&rt->eval_stack) > 0) {
        *out_val = vm_eval_pop(&rt->eval_stack);
    } else {
        *out_val = bval_int(0);
    }
    return 0;
}

int ast_interpret_stmt(RuntimeState *rt, AstStmt *stmt, int line_num) {
    if (!stmt) return 0;

    switch (stmt->type) {
    case STMT_PRINT: {
        int i;
        for (i = 0; i < stmt->v.print.item_count; i++) {
            AstPrintItem *item = &stmt->v.print.items[i];
            if (item->expr) {
                BValue val;
                ast_eval_expr_nonrec(rt, item->expr, line_num, &val);
                if (item->is_hash_width) {
                    // hash width formatting
                    long width = bval_to_int(&val);
                    (void)width; // placeholder or direct format
                } else {
                    if (bval_is_string(&val)) {
                        printf("%.*s", (int)val.v.sval.length, val.v.sval.data);
                    } else if (bval_is_float(&val)) {
                        printf("%g", val.v.fval);
                    } else {
                        printf("%ld", val.v.ival);
                    }
                }
            }
            if (item->suppress_space) {
                // do not print extra spacing
            } else {
                printf(" ");
            }
        }
        if (!stmt->v.print.trailing_comma) {
            printf("\n");
        }
        break;
    }
    case STMT_LET: {
        BValue val;
        ast_eval_expr_nonrec(rt, stmt->v.let.value, line_num, &val);
        runtime_set_var_bval(rt, stmt->v.let.var_name, val);
        break;
    }
    case STMT_LET_STRVAR: {
        BValue val;
        ast_eval_expr_nonrec(rt, stmt->v.let_strvar.value, line_num, &val);
        runtime_set_string_var(rt, stmt->v.let_strvar.var_name, val);
        break;
    }
    case STMT_LET_ARRAY_AT: {
        BValue idx_val, val;
        ast_eval_expr_nonrec(rt, stmt->v.let_array_at.index, line_num, &idx_val);
        ast_eval_expr_nonrec(rt, stmt->v.let_array_at.value, line_num, &val);
        runtime_set_array(rt, bval_to_int(&idx_val), bval_to_int(&val));
        break;
    }
    case STMT_LET_DIM: {
        BValue val, idx1_val, idx2_val;
        long idx1 = 0, idx2 = 0;
        int total_args = stmt->v.let_dim.idx2 ? 2 : 1;
        ast_eval_expr_nonrec(rt, stmt->v.let_dim.idx1, line_num, &idx1_val);
        idx1 = bval_to_int(&idx1_val);
        if (total_args == 2) {
            ast_eval_expr_nonrec(rt, stmt->v.let_dim.idx2, line_num, &idx2_val);
            idx2 = bval_to_int(&idx2_val);
        }
        ast_eval_expr_nonrec(rt, stmt->v.let_dim.value, line_num, &val);
        runtime_set_dim(rt, stmt->v.let_dim.name, stmt->v.let_dim.name_len, (int)idx1, (int)idx2, 0, val, line_num);
        break;
    }
    case STMT_IF: {
        BValue cond;
        ast_eval_expr_nonrec(rt, stmt->v.if_stmt.condition, line_num, &cond);
        if (bval_to_int(&cond) != 0) {
            AstStmt *curr = stmt->v.if_stmt.then_stmt;
            while (curr) {
                ast_interpret_stmt(rt, curr, line_num);
                curr = curr->next;
            }
        }
        break;
    }
    case STMT_GOTO: {
        BValue target;
        ast_eval_expr_nonrec(rt, stmt->v.goto_stmt.target, line_num, &target);
        vm_jump(rt, (int)bval_to_int(&target), line_num);
        break;
    }
    case STMT_GOSUB: {
        BValue target;
        ast_eval_expr_nonrec(rt, stmt->v.gosub.target, line_num, &target);
        vm_call(rt, (int)bval_to_int(&target), line_num);
        break;
    }
    case STMT_RETURN: {
        vm_return_sub(rt, line_num);
        break;
    }
    case STMT_END: {
        vm_set_state(rt, VM_HALTED);
        break;
    }
    case STMT_STOP: {
        vm_set_state(rt, VM_PAUSED);
        break;
    }
    case STMT_DIM: {
        long d1 = 0, d2 = 0;
        int dims = stmt->v.dim.dim2 ? 2 : 1;
        BValue v1, v2;
        ast_eval_expr_nonrec(rt, stmt->v.dim.dim1, line_num, &v1);
        d1 = bval_to_int(&v1);
        if (dims == 2) {
            ast_eval_expr_nonrec(rt, stmt->v.dim.dim2, line_num, &v2);
            d2 = bval_to_int(&v2);
        }
        runtime_dim(rt, stmt->v.dim.name, stmt->v.dim.name_len, (int)d1, (int)d2, 0, line_num);
        break;
    }
    case STMT_ON_GOTO: {
        BValue sel;
        ast_eval_expr_nonrec(rt, stmt->v.on_goto.selector, line_num, &sel);
        int idx = (int)bval_to_int(&sel) - 1;
        if (idx >= 0 && idx < stmt->v.on_goto.target_count) {
            if (stmt->v.on_goto.is_gosub) {
                vm_call(rt, stmt->v.on_goto.targets[idx], line_num);
            } else {
                vm_jump(rt, stmt->v.on_goto.targets[idx], line_num);
            }
        }
        break;
    }
    case STMT_RESTORE: {
        runtime_restore_data(rt);
        break;
    }
    case STMT_DIRECT_EXEC: {
        Lexer lex;
        lexer_init(&lex, stmt->v.direct_exec.text);
        parser_execute_line(&lex, rt, line_num);
        break;
    }
    default:
        break;
    }
    return 0;
}

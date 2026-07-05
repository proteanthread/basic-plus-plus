/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ast_core.c
 * Subsystem: Portable AST Tree Nodes Manager
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Defines node memory layout, constructor allocation, and deallocations.
 *
 * 2. WHAT TO EXPECT:
 *    Recursive AST walker walks top-down. Memory allocated on compiler workspace.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Maximum statement node parameters count.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    AST node struct memory alignments.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If compiler memory leaks, verify deallocation routines walk all union variants.
 * ===================================================================== */

#include "ast_core.h"
#include <stdlib.h>
#include <string.h>

AstExpr *ast_core_expr_new(AstExprType type)
{
    AstExpr *e = (AstExpr *)malloc(sizeof(AstExpr));
    if (e) {
        memset(e, 0, sizeof(AstExpr));
        e->type = type;
    }
    return e;
}

AstExpr *ast_core_expr_int(long val)
{
    AstExpr *e = ast_core_expr_new(EXPR_INT_LIT);
    if (e) e->v.ival = val;
    return e;
}

AstExpr *ast_core_expr_float(double val)
{
    AstExpr *e = ast_core_expr_new(EXPR_FLOAT_LIT);
    if (e) e->v.fval = val;
    return e;
}

AstExpr *ast_core_expr_string(const char *data, int length)
{
    AstExpr *e = ast_core_expr_new(EXPR_STRING_LIT);
    if (e) {
        e->v.sval.data = (char *)malloc((size_t)(length + 1));
        if (e->v.sval.data) {
            memcpy(e->v.sval.data, data, (size_t)length);
            e->v.sval.data[length] = '\0';
        }
        e->v.sval.length = length;
    }
    return e;
}

AstExpr *ast_core_expr_var(char name)
{
    AstExpr *e = ast_core_expr_new(EXPR_VAR);
    if (e) e->v.var_name = name;
    return e;
}

AstExpr *ast_core_expr_string_var(char name)
{
    AstExpr *e = ast_core_expr_new(EXPR_STRING_VAR);
    if (e) e->v.var_name = name;
    return e;
}

AstExpr *ast_core_expr_named_var(const char *name, int len)
{
    AstExpr *e = ast_core_expr_new(EXPR_NAMED_VAR);
    if (e) {
        int clen = len;
        if (clen > MAX_VAR_NAME_LEN) clen = MAX_VAR_NAME_LEN;
        memcpy(e->v.named.name, name, (size_t)clen);
        e->v.named.name[clen] = '\0';
        e->v.named.name_len = clen;
    }
    return e;
}

AstExpr *ast_core_expr_binop(AstBinOp op, AstExpr *left, AstExpr *right)
{
    AstExpr *e = ast_core_expr_new(EXPR_BINOP);
    if (e) {
        e->v.binop.op = op;
        e->v.binop.left = left;
        e->v.binop.right = right;
    }
    return e;
}

AstExpr *ast_core_expr_unop(AstUnOp op, AstExpr *operand)
{
    AstExpr *e = ast_core_expr_new(EXPR_UNOP);
    if (e) {
        e->v.unop.op = op;
        e->v.unop.operand = operand;
    }
    return e;
}

AstExpr *ast_core_expr_func(AstFuncId func, int argc, AstExpr *a0, AstExpr *a1, AstExpr *a2)
{
    AstExpr *e = ast_core_expr_new(EXPR_FUNC_CALL);
    if (e) {
        e->v.func_call.func = func;
        e->v.func_call.arg_count = argc;
        e->v.func_call.args[0] = a0;
        e->v.func_call.args[1] = a1;
        e->v.func_call.args[2] = a2;
        e->v.func_call.builtin_kw = 0;
    }
    return e;
}

AstExpr *ast_core_expr_array_at(AstExpr *index)
{
    AstExpr *e = ast_core_expr_new(EXPR_ARRAY_AT);
    if (e) e->v.array_at.index = index;
    return e;
}

AstExpr *ast_core_expr_dim_access(const char *name, int nlen, AstExpr *idx1, AstExpr *idx2)
{
    AstExpr *e = ast_core_expr_new(EXPR_DIM_ACCESS);
    if (e) {
        int clen = nlen;
        if (clen > MAX_VAR_NAME_LEN) clen = MAX_VAR_NAME_LEN;
        memcpy(e->v.dim_access.name, name, (size_t)clen);
        e->v.dim_access.name[clen] = '\0';
        e->v.dim_access.name_len = clen;
        e->v.dim_access.idx1 = idx1;
        e->v.dim_access.idx2 = idx2;
    }
    return e;
}

AstStmt *ast_core_stmt_new(AstStmtType type)
{
    AstStmt *s = (AstStmt *)malloc(sizeof(AstStmt));
    if (s) {
        memset(s, 0, sizeof(AstStmt));
        s->type = type;
    }
    return s;
}

void ast_core_free_expr(AstExpr *expr)
{
    if (!expr) return;
    switch (expr->type) {
    case EXPR_STRING_LIT:
        free(expr->v.sval.data);
        break;
    case EXPR_ARRAY_AT:
        ast_core_free_expr(expr->v.array_at.index);
        break;
    case EXPR_DIM_ACCESS:
        ast_core_free_expr(expr->v.dim_access.idx1);
        ast_core_free_expr(expr->v.dim_access.idx2);
        break;
    case EXPR_BINOP:
        ast_core_free_expr(expr->v.binop.left);
        ast_core_free_expr(expr->v.binop.right);
        break;
    case EXPR_UNOP:
        ast_core_free_expr(expr->v.unop.operand);
        break;
    case EXPR_FUNC_CALL:
        {
            int i;
            for (i = 0; i < expr->v.func_call.arg_count; i++) {
                ast_core_free_expr(expr->v.func_call.args[i]);
            }
        }
        break;
    default:
        break;
    }
    free(expr);
}

void ast_core_free_stmt(AstStmt *stmt)
{
    if (!stmt) return;
    switch (stmt->type) {
    case STMT_PRINT:
        {
            int i;
            for (i = 0; i < stmt->v.print.item_count; i++) {
                ast_core_free_expr(stmt->v.print.items[i].expr);
            }
            free(stmt->v.print.items);
        }
        break;
    case STMT_LET:
        ast_core_free_expr(stmt->v.let.value);
        break;
    case STMT_LET_STRVAR:
        ast_core_free_expr(stmt->v.let_strvar.value);
        break;
    case STMT_LET_ARRAY_AT:
        ast_core_free_expr(stmt->v.let_array_at.index);
        ast_core_free_expr(stmt->v.let_array_at.value);
        break;
    case STMT_LET_DIM:
        ast_core_free_expr(stmt->v.let_dim.idx1);
        ast_core_free_expr(stmt->v.let_dim.idx2);
        ast_core_free_expr(stmt->v.let_dim.value);
        break;
    case STMT_IF:
        ast_core_free_expr(stmt->v.if_stmt.condition);
        ast_core_free_stmt(stmt->v.if_stmt.then_stmt);
        break;
    case STMT_GOTO:
        ast_core_free_expr(stmt->v.goto_stmt.target);
        break;
    case STMT_GOSUB:
        ast_core_free_expr(stmt->v.gosub.target);
        break;
    case STMT_FOR:
        ast_core_free_expr(stmt->v.for_stmt.init);
        ast_core_free_expr(stmt->v.for_stmt.limit);
        ast_core_free_expr(stmt->v.for_stmt.step);
        break;
    case STMT_INPUT:
        {
            int vi;
            ast_core_free_expr(stmt->v.input.prompt);
            for (vi = 0; vi < stmt->v.input.var_count; vi++) {
                ast_core_free_expr(stmt->v.input.vars[vi]);
            }
        }
        break;
    case STMT_DIM:
        ast_core_free_expr(stmt->v.dim.dim1);
        ast_core_free_expr(stmt->v.dim.dim2);
        break;
    case STMT_WHILE:
        ast_core_free_expr(stmt->v.while_stmt.left);
        ast_core_free_expr(stmt->v.while_stmt.right);
        break;
    case STMT_DO:
        ast_core_free_expr(stmt->v.do_stmt.left);
        ast_core_free_expr(stmt->v.do_stmt.right);
        break;
    case STMT_LOOP:
        ast_core_free_expr(stmt->v.loop_stmt.left);
        ast_core_free_expr(stmt->v.loop_stmt.right);
        break;
    case STMT_READ:
        {
            int i;
            for (i = 0; i < stmt->v.read.var_count; i++) {
                ast_core_free_expr(stmt->v.read.var_indices[i]);
                ast_core_free_expr(stmt->v.read.var_indices2[i]);
            }
        }
        break;
    case STMT_ON_GOTO:
        ast_core_free_expr(stmt->v.on_goto.selector);
        break;
    case STMT_DEF_FN:
        ast_core_free_expr(stmt->v.def_fn.body);
        break;
    case STMT_INT:
        ast_core_free_expr(stmt->v.int_stmt.interrupt_number);
        break;
    case STMT_DIRECT_EXEC:
        free(stmt->v.direct_exec.text);
        break;
    default:
        break;
    }
    free(stmt);
}

void ast_core_free_line(AstStmt *stmts)
{
    AstStmt *curr = stmts;
    while (curr) {
        AstStmt *next = curr->next;
        ast_core_free_stmt(curr);
        curr = next;
    }
}

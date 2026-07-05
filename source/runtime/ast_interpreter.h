/* =====================================================================
 * BASIC++ Interpreter — AST Interpreter Header
 * ===================================================================== */

#ifndef BASICPP_AST_INTERPRETER_H
#define BASICPP_AST_INTERPRETER_H

#include "runtime.h"
#include "ast.h"

// Evaluates an AST expression non-recursively using the shared evaluation stack
int ast_eval_expr_nonrec(RuntimeState *rt, AstExpr *root, int line_num, BValue *out_val);

// Interprets a single AST statement
int ast_interpret_stmt(RuntimeState *rt, AstStmt *stmt, int line_num);

// Invokes a function call from the AST evaluator
BValue ast_call_function(RuntimeState *rt, AstFuncId func_id, BValue *args, int arg_count, char fn_letter, int builtin_kw, int line_num);

#endif // BASICPP_AST_INTERPRETER_H

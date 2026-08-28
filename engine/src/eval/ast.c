// FILENAME: ast.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast_internal.h, exec_internal.h, exec_interrupt.c)
// NEEDS: libengine (ast.h, ast_internal.h, eval.h, eval.c, lexer.h, lexer.c)
// NEEDS: libengine (vm.h)
// Implements AST parsing and evaluation structures for ast.
//
// ---- Includes ----

#include "eval/ast.h"
#include "eval/ast_internal.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

//
// ---- Top-Level AST Interface ----

// parses an expression from a token stream into an AST tree
EvalAstNode *eval_ast_parse_expr(LexerContext *lex) {
    if (!lex) return NULL;
    return parse_single_statement(lex);
}

// evaluates an AST expression tree against a VM context returning a BValue
BValue eval_ast_eval(VMContext *vm, EvalAstNode *node, BppError *err) {
    return eval_ast_evaluate(vm, node, err);
}

// executes an AST statement or block in the VM context
BppError eval_ast_exec(VMContext *vm, EvalAstNode *head) {
    return eval_ast_execute(vm, head);
}

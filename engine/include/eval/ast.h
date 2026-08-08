/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file eval_ast.h
 * @brief Evaluator AST Micro-Library Header.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares AST node allocation, creation, and recycling helpers.
 * - Why it exists: Decouples AST tree management out of monolithic expression evaluators.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: AST node types.
 * - What cannot be changed: Arena allocation invariants.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard C17.
 * - Portability concerns: None.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - AST optimization passes.
 */

#ifndef EVAL_EVAL_AST_H
#define EVAL_EVAL_AST_H

#include "types/types.h"
#include "memory/memory.h"
#include "lexer/lexer.h"

typedef enum {
    AST_NODE_LITERAL = 0,
    AST_NODE_VARIABLE,
    AST_NODE_BINARY_OP,
    AST_NODE_UNARY_OP,
    AST_NODE_CALL
} EvalAstNodeType;

typedef struct EvalAstNode {
    EvalAstNodeType type;
    BValue          val;
    BppTokenType    op;
    struct EvalAstNode *left;
    struct EvalAstNode *right;
} EvalAstNode;

EvalAstNode *eval_ast_create_literal(MemoryContext *mem, BValue val);
EvalAstNode *eval_ast_create_binary(MemoryContext *mem, BppTokenType op, EvalAstNode *left, EvalAstNode *right);
void         eval_ast_free(MemoryContext *mem, EvalAstNode *node);

#endif /* EVAL_EVAL_AST_H */

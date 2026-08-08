/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file ast.c
 * @brief Abstract Syntax Tree (AST) node constructor, destructor, and recursive tree evaluator implementation.
 *
 * 1. WHAT IT DOES:
 *    Implements AST node creation (`ast_node_create()`), tree deallocation (`ast_node_free()`), and expression evaluation (`ast_eval()`),
 *    supporting literal values, binary operators (`+`, `-`, `*`, `/`, `^`, `MOD`, `AND`, `OR`, `XOR`), unary operators (`-`, `NOT`),
 *    variable lookups, and function calls.
 *
 * 2. WHY IT EXISTS:
 *    Represents BASIC++ expressions in structured syntax trees, enabling operator precedence enforcement, short-circuit logical evaluation,
 *    and expression optimization prior to VM bytecode execution.
 *
 * 3. WHY IT WORKS THIS WAY:
 *    Allocates AST nodes via `calloc(1, sizeof(ASTNode))` to zero memory. Evaluates expressions recursively with bounded tree depth,
 *    returning tagged `BValue` structs containing results or `VAL_ERROR`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 *    - Required Headers: `eval/eval.h`, `eval/dispatch.h`, `types/types.h`, `types/errors.h`
 *    - CMake Target: Part of `eval_core` micro-library target in `engine/CMakeLists.txt`.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 *    - Included in all target executables (`baspp`, `bpp`, `bs`).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 *    - To add new binary operators (e.g. `<<`, `>>`, `<<<`): add node types to `ASTNodeType` enum in `eval.h` and branch in `ast_eval()`.
 *    - To optimize constant folding (e.g. `1 + 2` -> `3` at parse time): extend `ast_optimize()`.
 *
 * 7. WHAT CANNOT BE CHANGED:
 *    - Short-circuit logical evaluation rules for `AND` (fails fast on false) and `OR` (fails fast on true).
 *    - Automatic reference counting rules when returning `VAL_STRING` values from string expression nodes.
 *
 * 8. WHAT TO EXPECT:
 *    - Evaluation returns `BValue` by value.
 *    - Division by zero returns `val_make_error(ERR_DIVISION_BY_ZERO)`.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check for stack overflow on deeply nested expression trees.
 *    - Inspect AST node type tags (`node->type`) and left/right child non-NULL checks before dereferencing.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 *     - VM context (`vm`) and parser context (`pctx`) must be initialized.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 *     - Strict C17 compliance (`-std=c17`).
 *     - Double precision float calculations compliant with IEEE 754 standards.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 *     - Prerequisite C Source Files: `engine/src/eval/eval.c` (evaluator engine), `engine/src/eval/dispatch.c` (built-in function dispatcher), `engine/src/core/string.c` (string memory manager).
 *     - Prerequisite Header Surfaces: `engine/include/eval/eval.h`, `engine/include/eval/dispatch.h`, `engine/include/types/types.h`, `engine/include/types/errors.h`.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file eval_ast.c
 * @brief Evaluator AST Micro-Library Implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements AST node allocation and recycling using scratch memory arena.
 * - Why it exists: Isolated micro-library for expression AST nodes.
 * - Why it works this way: Allocates from scratch memory arena for high performance.
 *
 * SECTION 2: SAFE ZONES FOR EDITING
 * - Safe to add new AST node creation factory functions.
 *
 * SECTION 3: CRITICAL INVARIANTS
 * - Memory allocation must use mem_scratch_alloc.
 *
 * SECTION 4: EXPECTED SIDE-EFFECTS & BEHAVIORS
 * - Allocates zero-initialized AST nodes.
 *
 * SECTION 5: WHAT TO DO IF SOMETHING BREAKS
 * - Check scratch memory pool capacity.
 *
 * SECTION 6: ASSUMPTIONS
 * - Pointer inputs are non-NULL.
 *
 * SECTION 7: PORTABILITY CONCERNS
 * - Strictly portable C17.
 *
 * SECTION 8: FUTURE EXPANSIONS
 * - AST tree constant folding optimizer.
 */

#include "eval/ast.h"
#include <string.h>

EvalAstNode *eval_ast_create_literal(MemoryContext *mem, BValue val) {
    if (!mem) return NULL;
    EvalAstNode *node = (EvalAstNode *)mem_scratch_alloc(mem, sizeof(EvalAstNode));
    if (!node) return NULL;
    memset(node, 0, sizeof(EvalAstNode));
    node->type = AST_NODE_LITERAL;
    node->val = val;
    return node;
}

EvalAstNode *eval_ast_create_binary(MemoryContext *mem, BppTokenType op, EvalAstNode *left, EvalAstNode *right) {
    if (!mem) return NULL;
    EvalAstNode *node = (EvalAstNode *)mem_scratch_alloc(mem, sizeof(EvalAstNode));
    if (!node) return NULL;
    memset(node, 0, sizeof(EvalAstNode));
    node->type = AST_NODE_BINARY_OP;
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

void eval_ast_free(MemoryContext *mem, EvalAstNode *node) {
    (void)mem;
    (void)node;
    /* Scratch memory arena is reset as a unit at statement boundaries */
}

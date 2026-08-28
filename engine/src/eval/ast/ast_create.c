// FILENAME: ast_create.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ast_internal.h)
// Implements AST parsing and evaluation structures for ast_create.
//
// ---- Includes ----

#include "eval/ast_internal.h"
#include "runtime/strings.h"

//
// ---- Node Allocators & Constructors ----

static inline EvalAstNode *ast_alloc_node(void) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return NULL;
    EvalAstNode *node = (EvalAstNode *)hal->mem.alloc(sizeof(EvalAstNode));
    if (node) {
        runtime_memset(node, 0, sizeof(EvalAstNode));
    }
    return node;
}

static inline void ast_free_node(void *ptr) {
    HalContext *hal = hal_get();
    if (hal && hal->mem.free && ptr) {
        hal->mem.free(ptr);
    }
}

// creates a literal value AST node
EvalAstNode *eval_ast_create_literal(MemoryContext *mem, BValue val) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_LITERAL;
    node->val = val;
    return node;
}

// creates a variable access AST node
EvalAstNode *eval_ast_create_variable(MemoryContext *mem, const char *name) {
    (void)mem;
    if (!name || !*name) return NULL;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_VARIABLE;
    runtime_strncpy(node->var_name, name, sizeof(node->var_name) - 1);
    node->var_name[sizeof(node->var_name) - 1] = '\0';
    return node;
}


// creates a binary operation AST node with constant folding
EvalAstNode *eval_ast_create_binary(MemoryContext *mem, BppTokenType op, EvalAstNode *left, EvalAstNode *right) {
    (void)mem;
    if (left && right && left->type == AST_NODE_LITERAL && right->type == AST_NODE_LITERAL &&
        (left->val.type == VAL_NUMBER || left->val.type == VAL_INTEGER) &&
        (right->val.type == VAL_NUMBER || right->val.type == VAL_INTEGER)) {
        double n1 = left->val.as.number;
        double n2 = right->val.as.number;
        bool folded = false;
        double ans = 0.0;
        switch (op) {
            case TOK_PLUS:  ans = n1 + n2; folded = true; break;
            case TOK_MINUS: ans = n1 - n2; folded = true; break;
            case TOK_MUL:   ans = n1 * n2; folded = true; break;
            case TOK_DIV:   if (n2 != 0.0) { ans = n1 / n2; folded = true; } break;
            case TOK_EQ:    ans = (n1 == n2) ? -1.0 : 0.0; folded = true; break;
            case TOK_NE:    ans = (n1 != n2) ? -1.0 : 0.0; folded = true; break;
            case TOK_LT:    ans = (n1 < n2)  ? -1.0 : 0.0; folded = true; break;
            case TOK_GT:    ans = (n1 > n2)  ? -1.0 : 0.0; folded = true; break;
            case TOK_LE:    ans = (n1 <= n2) ? -1.0 : 0.0; folded = true; break;
            case TOK_GE:    ans = (n1 >= n2) ? -1.0 : 0.0; folded = true; break;
            case TOK_AND:   ans = (double)((int64_t)n1 & (int64_t)n2); folded = true; break;
            case TOK_OR:    ans = (double)((int64_t)n1 | (int64_t)n2); folded = true; break;
            case TOK_XOR:   ans = (double)((int64_t)n1 ^ (int64_t)n2); folded = true; break;
            case TOK_MOD:   if ((int64_t)n2 != 0) { ans = (double)((int64_t)n1 % (int64_t)n2); folded = true; } break;
            case TOK_POW:
                if (n2 == (double)(int)n2 && n2 >= -100 && n2 <= 100) {
                    ans = fast_int_pow(n1, (int)n2);
                    folded = true;
                } else if (n1 > 0.0 || (n1 == 0.0 && n2 > 0.0)) {
                    ans = runtime_pow(n1, n2);
                    folded = true;
                }
                break;
            default: break;
        }
        if (folded) {
            ast_free_node(left);
            ast_free_node(right);
            BValue res = { .type = VAL_NUMBER, .as.number = ans };
            return eval_ast_create_literal(mem, res);
        }
    }

    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_BINARY_OP;
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

// creates a unary operation AST node with constant folding
EvalAstNode *eval_ast_create_unary(MemoryContext *mem, BppTokenType op, EvalAstNode *operand) {
    (void)mem;
    if (operand && operand->type == AST_NODE_LITERAL &&
        (operand->val.type == VAL_NUMBER || operand->val.type == VAL_INTEGER)) {
        if (op == TOK_UNARY_MINUS) {
            operand->val.as.number = -operand->val.as.number;
            return operand;
        } else if (op == TOK_NOT) {
            operand->val.as.number = (double)(~(int64_t)operand->val.as.number);
            return operand;
        }
    }

    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_UNARY_OP;
    node->op = op;
    node->left = operand;
    return node;
}

// creates an assignment AST node
EvalAstNode *eval_ast_create_assignment(MemoryContext *mem, const char *name, EvalAstNode *expr) {
    (void)mem;
    if (!name || !*name) return NULL;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_ASSIGNMENT;
    runtime_strncpy(node->var_name, name, sizeof(node->var_name) - 1);
    node->var_name[sizeof(node->var_name) - 1] = '\0';
    node->right = expr;
    return node;
}

// creates an IF statement condition and branches AST node
EvalAstNode *eval_ast_create_if(MemoryContext *mem, EvalAstNode *cond, EvalAstNode *then_b, EvalAstNode *else_b) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_IF;
    node->condition = cond;
    node->then_branch = then_b;
    node->else_branch = else_b;
    return node;
}

// creates a built-in math function call AST node
EvalAstNode *eval_ast_create_math_func(MemoryContext *mem, AstMathFunc func, EvalAstNode *arg) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_MATH_FUNC;
    node->math_func = func;
    node->left = arg;
    return node;
}

// creates a 1D array read AST node
EvalAstNode *eval_ast_create_array_read(MemoryContext *mem, const char *name, EvalAstNode *index_expr) {
    (void)mem;
    if (!name || !*name) return NULL;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_ARRAY_READ;
    runtime_strncpy(node->var_name, name, sizeof(node->var_name) - 1);
    node->var_name[sizeof(node->var_name) - 1] = '\0';
    node->index_expr = index_expr;
    return node;
}

// creates a 1D array assignment AST node
EvalAstNode *eval_ast_create_array_assign(MemoryContext *mem, const char *name, EvalAstNode *index_expr, EvalAstNode *val_expr) {
    (void)mem;
    if (!name || !*name) return NULL;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_ARRAY_ASSIGN;
    runtime_strncpy(node->var_name, name, sizeof(node->var_name) - 1);
    node->var_name[sizeof(node->var_name) - 1] = '\0';
    node->index_expr = index_expr;
    node->right = val_expr;
    return node;
}

// creates a 2D array read AST node
EvalAstNode *eval_ast_create_array2d_read(MemoryContext *mem, const char *name, EvalAstNode *index_expr, EvalAstNode *index2_expr) {
    (void)mem;
    if (!name || !*name) return NULL;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_ARRAY2D_READ;
    runtime_strncpy(node->var_name, name, sizeof(node->var_name) - 1);
    node->var_name[sizeof(node->var_name) - 1] = '\0';
    node->index_expr = index_expr;
    node->index2_expr = index2_expr;
    return node;
}

// creates a 2D array assignment AST node
EvalAstNode *eval_ast_create_array2d_assign(MemoryContext *mem, const char *name, EvalAstNode *index_expr, EvalAstNode *index2_expr, EvalAstNode *val_expr) {
    (void)mem;
    if (!name || !*name) return NULL;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_ARRAY2D_ASSIGN;
    runtime_strncpy(node->var_name, name, sizeof(node->var_name) - 1);
    node->var_name[sizeof(node->var_name) - 1] = '\0';
    node->index_expr = index_expr;
    node->index2_expr = index2_expr;
    node->right = val_expr;
    return node;
}

// creates a GOTO jump AST node
EvalAstNode *eval_ast_create_goto(MemoryContext *mem, BppLineNumber line) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_GOTO;
    node->target_line = line;
    return node;
}

// creates a GOSUB subroutine jump AST node
EvalAstNode *eval_ast_create_gosub(MemoryContext *mem, BppLineNumber line) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_GOSUB;
    node->target_line = line;
    return node;
}

// creates a RETURN statement AST node
EvalAstNode *eval_ast_create_return(MemoryContext *mem) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_RETURN;
    return node;
}

// creates a FOR loop construct AST node
EvalAstNode *eval_ast_create_for_loop(MemoryContext *mem, const char *name, EvalAstNode *start_expr, EvalAstNode *end_expr, EvalAstNode *step_expr, EvalAstNode *body) {
    (void)mem;
    if (!name || !*name) return NULL;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_FOR_LOOP;
    runtime_strncpy(node->var_name, name, sizeof(node->var_name) - 1);
    node->var_name[sizeof(node->var_name) - 1] = '\0';
    node->left = start_expr;
    node->right = end_expr;
    node->condition = step_expr;
    node->then_branch = body;
    return node;
}

// creates a WHILE loop construct AST node
EvalAstNode *eval_ast_create_while_loop(MemoryContext *mem, EvalAstNode *cond, EvalAstNode *body) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_WHILE_LOOP;
    node->condition = cond;
    node->then_branch = body;
    return node;
}

// creates a PRINT statement AST node
EvalAstNode *eval_ast_create_print(MemoryContext *mem, EvalAstNode *expr) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_PRINT;
    node->left = expr;
    return node;
}

// creates a string built-in function AST node
EvalAstNode *eval_ast_create_string_func(MemoryContext *mem, AstStringFunc func, EvalAstNode *arg1, EvalAstNode *arg2, EvalAstNode *arg3) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_STRING_FUNC;
    node->str_func = func;
    node->left = arg1;
    node->right = arg2;
    node->condition = arg3;
    return node;
}

// creates a POKE direct memory write AST node
EvalAstNode *eval_ast_create_poke(MemoryContext *mem, EvalAstNode *addr_expr, EvalAstNode *val_expr) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_POKE;
    node->left = addr_expr;
    node->right = val_expr;
    return node;
}

// creates a PRINT # channel output AST node
EvalAstNode *eval_ast_create_file_print(MemoryContext *mem, int channel, EvalAstNode *expr) {
    (void)mem;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_FILE_PRINT;
    node->channel = channel;
    node->left = expr;
    return node;
}

// creates a LINE INPUT # channel input AST node
EvalAstNode *eval_ast_create_line_input(MemoryContext *mem, int channel, const char *var_name) {
    (void)mem;
    if (!var_name || !*var_name) return NULL;
    EvalAstNode *node = ast_alloc_node();
    if (!node) return NULL;
    node->type = AST_NODE_LINE_INPUT;
    node->channel = channel;
    runtime_strncpy(node->var_name, var_name, sizeof(node->var_name) - 1);
    node->var_name[sizeof(node->var_name) - 1] = '\0';
    return node;
}

// frees an AST node and its sub-trees with context
void eval_ast_free(MemoryContext *mem, EvalAstNode *node) {
    (void)mem;
    eval_ast_free_tree(node);
}

// recursively frees an AST node and all associated children
void eval_ast_free_tree(void *node_ptr) {
    if (!node_ptr) return;
    EvalAstNode *node = (EvalAstNode *)node_ptr;
    if (node->type == AST_NODE_LITERAL && node->val.type == VAL_STRING && node->val.as.string) {
        str_free_static(node->val.as.string);
    }
    if (node->left) eval_ast_free_tree(node->left);
    if (node->right) eval_ast_free_tree(node->right);
    if (node->condition) eval_ast_free_tree(node->condition);
    if (node->then_branch) eval_ast_free_tree(node->then_branch);
    if (node->else_branch) eval_ast_free_tree(node->else_branch);
    if (node->index_expr) eval_ast_free_tree(node->index_expr);
    if (node->index2_expr) eval_ast_free_tree(node->index2_expr);
    if (node->target_ast) eval_ast_free_tree(node->target_ast);
    if (node->next) eval_ast_free_tree(node->next);
    ast_free_node(node);
}


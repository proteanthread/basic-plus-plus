// FILENAME: ast.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast.c, ast_internal.h, exec_internal.h)
// NEEDED BY: libengine (exec_interrupt.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libengine (lexer.h, lexer.c)
// NEEDS: libkernel (errors.h, types.h)
// Implements AST parsing and evaluation structures for ast.
//
// ---- Includes ----

#ifndef EVAL_EVAL_AST_H
#define EVAL_EVAL_AST_H

#include "types/types.h"
#include "types/errors.h"
#include "memory/memory.h"
#include "lexer/lexer.h"

struct VMContext;

typedef enum {
    AST_NODE_LITERAL = 0,
    AST_NODE_VARIABLE,
    AST_NODE_BINARY_OP,
    AST_NODE_UNARY_OP,
    AST_NODE_ASSIGNMENT,
    AST_NODE_IF,
    AST_NODE_STATEMENT_LIST,
    AST_NODE_MATH_FUNC,
    AST_NODE_ARRAY_READ,
    AST_NODE_ARRAY_ASSIGN,
    AST_NODE_ARRAY2D_READ,
    AST_NODE_ARRAY2D_ASSIGN,
    AST_NODE_GOTO,
    AST_NODE_GOSUB,
    AST_NODE_RETURN,
    AST_NODE_FOR_LOOP,
    AST_NODE_WHILE_LOOP,
    AST_NODE_PRINT,
    AST_NODE_STRING_FUNC,
    AST_NODE_POKE,
    AST_NODE_FILE_PRINT,
    AST_NODE_LINE_INPUT
} EvalAstNodeType;

typedef enum {
    AST_MATH_SIN = 1,
    AST_MATH_COS,
    AST_MATH_TAN,
    AST_MATH_SQR,
    AST_MATH_ABS,
    AST_MATH_INT,
    AST_MATH_FIX,
    AST_MATH_EXP,
    AST_MATH_LOG,
    AST_MATH_ATN,
    AST_MATH_SGN,
    AST_MATH_RND,
    AST_MATH_EOF
} AstMathFunc;

typedef enum {
    AST_STR_STR = 1,
    AST_STR_UCASE,
    AST_STR_LCASE,
    AST_STR_MID,
    AST_STR_LEFT,
    AST_STR_RIGHT,
    AST_STR_LEN,
    AST_STR_CHR,
    AST_STR_ASC,
    AST_STR_SPACE,
    AST_STR_STRING,
    AST_STR_HEX,
    AST_STR_OCT,
    AST_STR_BIN,
    AST_STR_INSTR,
    AST_STR_SHA256,
    AST_STR_MD5,
    AST_STR_PEEK
} AstStringFunc;

typedef struct EvalAstNode {
    EvalAstNodeType type;
    BValue          val;
    BppTokenType    op;
    AstMathFunc     math_func;
    AstStringFunc   str_func;
    int             channel;
    char            var_name[64];
    BppLineNumber   target_line;
    const char     *source_pos;
    BValue         *cached_var_ptr;
    void           *cached_arr;
    struct EvalAstNode *left;
    struct EvalAstNode *right;
    struct EvalAstNode *condition;
    struct EvalAstNode *then_branch;
    struct EvalAstNode *else_branch;
    struct EvalAstNode *index_expr;
    struct EvalAstNode *index2_expr;
    struct EvalAstNode *target_ast;
    struct EvalAstNode *next;
} EvalAstNode;

EvalAstNode *eval_ast_create_goto(MemoryContext *mem, BppLineNumber line);
EvalAstNode *eval_ast_create_gosub(MemoryContext *mem, BppLineNumber line);
EvalAstNode *eval_ast_create_return(MemoryContext *mem);

EvalAstNode *eval_ast_create_literal(MemoryContext *mem, BValue val);
EvalAstNode *eval_ast_create_variable(MemoryContext *mem, const char *name);
EvalAstNode *eval_ast_create_binary(MemoryContext *mem, BppTokenType op, EvalAstNode *left, EvalAstNode *right);
EvalAstNode *eval_ast_create_unary(MemoryContext *mem, BppTokenType op, EvalAstNode *operand);
EvalAstNode *eval_ast_create_assignment(MemoryContext *mem, const char *name, EvalAstNode *expr);
EvalAstNode *eval_ast_create_if(MemoryContext *mem, EvalAstNode *cond, EvalAstNode *then_b, EvalAstNode *else_b);
EvalAstNode *eval_ast_create_math_func(MemoryContext *mem, AstMathFunc func, EvalAstNode *arg);
EvalAstNode *eval_ast_create_string_func(MemoryContext *mem, AstStringFunc func, EvalAstNode *arg1, EvalAstNode *arg2, EvalAstNode *arg3);
EvalAstNode *eval_ast_create_array_read(MemoryContext *mem, const char *name, EvalAstNode *index_expr);
EvalAstNode *eval_ast_create_array_assign(MemoryContext *mem, const char *name, EvalAstNode *index_expr, EvalAstNode *val_expr);
EvalAstNode *eval_ast_create_array2d_read(MemoryContext *mem, const char *name, EvalAstNode *index_expr, EvalAstNode *index2_expr);
EvalAstNode *eval_ast_create_array2d_assign(MemoryContext *mem, const char *name, EvalAstNode *index_expr, EvalAstNode *index2_expr, EvalAstNode *val_expr);
EvalAstNode *eval_ast_create_poke(MemoryContext *mem, EvalAstNode *addr_expr, EvalAstNode *val_expr);
EvalAstNode *eval_ast_create_file_print(MemoryContext *mem, int channel, EvalAstNode *expr);
EvalAstNode *eval_ast_create_line_input(MemoryContext *mem, int channel, const char *var_name);

EvalAstNode *eval_ast_create_for_loop(MemoryContext *mem, const char *name, EvalAstNode *start_expr, EvalAstNode *end_expr, EvalAstNode *step_expr, EvalAstNode *body);
EvalAstNode *eval_ast_create_while_loop(MemoryContext *mem, EvalAstNode *cond, EvalAstNode *body);
EvalAstNode *eval_ast_create_print(MemoryContext *mem, EvalAstNode *expr);

void         eval_ast_free(MemoryContext *mem, EvalAstNode *node);
void         eval_ast_free_tree(void *node_ptr);

BValue       eval_ast_evaluate(struct VMContext *vm, EvalAstNode *node, BppError *err);
BppError     eval_ast_execute(struct VMContext *vm, EvalAstNode *node);

EvalAstNode *eval_ast_try_parse_line(struct VMContext *vm, const char *source);
EvalAstNode *eval_ast_try_compile_multiline_block(struct VMContext *vm, const BppProgramLine *lines, size_t start_idx, size_t total_count, size_t *out_lines_skipped);

#endif // EVAL_EVAL_AST_H

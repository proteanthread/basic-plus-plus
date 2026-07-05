/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ast_core.h
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

#ifndef STANDALONE_AST_CORE_H
#define STANDALONE_AST_CORE_H

#include "../../lexer.h"
#include "../../config.h"

/* Expression Node Types */
typedef enum AstExprType {
    EXPR_INT_LIT,
    EXPR_FLOAT_LIT,
    EXPR_STRING_LIT,
    EXPR_VAR,
    EXPR_STRING_VAR,
    EXPR_NAMED_VAR,
    EXPR_ARRAY_AT,
    EXPR_DIM_ACCESS,
    EXPR_BINOP,
    EXPR_UNOP,
    EXPR_FUNC_CALL
} AstExprType;

/* Binary Operators */
typedef enum AstBinOp {
    BOP_ADD, BOP_SUB, BOP_MUL, BOP_DIV, BOP_MOD, BOP_POW,
    BOP_EQ, BOP_NE, BOP_LT, BOP_GT, BOP_LE, BOP_GE,
    BOP_AND, BOP_OR,
    BOP_CONCAT
} AstBinOp;

/* Unary Operators */
typedef enum AstUnOp {
    UOP_NEG,
    UOP_NOT
} AstUnOp;

/* Function Identifiers */
typedef enum AstFuncId {
    FUNC_ABS, FUNC_RND, FUNC_SIZE,
    FUNC_SIN, FUNC_COS, FUNC_TAN, FUNC_ATN,
    FUNC_SQR, FUNC_LOG, FUNC_EXP, FUNC_SGN, FUNC_INT,
    FUNC_LEN, FUNC_ASC, FUNC_VAL,
    FUNC_CHR, FUNC_STR,
    FUNC_LEFT, FUNC_RIGHT, FUNC_MID,
    FUNC_TAB,
    FUNC_FN_USER,
    FUNC_BUILTIN,
    FUNC_MEMMAP,
    FUNC_VPATH,
    FUNC_CWD,
    FUNC_PWD
} AstFuncId;

typedef struct AstExpr AstExpr;

struct AstExpr {
    AstExprType type;
    union {
        long ival;
        double fval;
        struct {
            char *data;
            int length;
        } sval;
        char var_name;
        struct {
            char name[MAX_VAR_NAME_LEN + 1];
            int name_len;
        } named;
        struct {
            AstExpr *index;
        } array_at;
        struct {
            char name[MAX_VAR_NAME_LEN + 1];
            int name_len;
            AstExpr *idx1;
            AstExpr *idx2;
        } dim_access;
        struct {
            AstBinOp op;
            AstExpr *left;
            AstExpr *right;
        } binop;
        struct {
            AstUnOp op;
            AstExpr *operand;
        } unop;
        struct {
            AstFuncId func;
            AstExpr *args[3];
            int arg_count;
            char fn_letter;
            int builtin_kw;
        } func_call;
    } v;
};

/* Statement Node Types */
typedef enum AstStmtType {
    STMT_PRINT,
    STMT_LET,
    STMT_LET_STRVAR,
    STMT_LET_ARRAY_AT,
    STMT_LET_DIM,
    STMT_IF,
    STMT_GOTO,
    STMT_GOSUB,
    STMT_RETURN,
    STMT_FOR,
    STMT_NEXT,
    STMT_INPUT,
    STMT_END,
    STMT_STOP,
    STMT_REM,
    STMT_DIM,
    STMT_DATA,
    STMT_READ,
    STMT_RESTORE,
    STMT_WHILE,
    STMT_WEND,
    STMT_DO,
    STMT_LOOP,
    STMT_ON_GOTO,
    STMT_DEF_FN,
    STMT_WHEN,
    STMT_USE,
    STMT_END_WHEN,
    STMT_RETRY,
    STMT_CONTINUE,
    STMT_INT,
    STMT_DIRECT_EXEC
} AstStmtType;

typedef struct AstStmt AstStmt;

typedef struct AstPrintItem {
    AstExpr *expr;
    int suppress_space;
    int is_hash_width;
} AstPrintItem;

struct AstStmt {
    AstStmtType type;
    AstStmt *next;
    union {
        struct {
            AstPrintItem *items;
            int item_count;
            int trailing_comma;
        } print;
        struct {
            char var_name;
            AstExpr *value;
        } let;
        struct {
            char var_name;
            AstExpr *value;
        } let_strvar;
        struct {
            AstExpr *index;
            AstExpr *value;
        } let_array_at;
        struct {
            char name[MAX_VAR_NAME_LEN + 1];
            int name_len;
            AstExpr *idx1;
            AstExpr *idx2;
            AstExpr *value;
        } let_dim;
        struct {
            AstExpr *condition;
            AstStmt *then_stmt;
        } if_stmt;
        struct {
            AstExpr *target;
        } goto_stmt;
        struct {
            AstExpr *target;
        } gosub;
        struct {
            char var_name;
            AstExpr *init;
            AstExpr *limit;
            AstExpr *step;
        } for_stmt;
        struct {
            char var_name;
        } next;
        struct {
            AstExpr *prompt;
            char var_names[26];
            int var_types[26];
            int var_count;
        } input;
        struct {
            char name[MAX_VAR_NAME_LEN + 1];
            int name_len;
            AstExpr *dim1;
            AstExpr *dim2;
        } dim;
        struct {
            const char *text;
        } rem;
        struct {
            AstExpr *left;
            AstExpr *right;
            TokenType relop;
        } while_stmt;
        struct {
            int has_condition;
            AstExpr *left;
            AstExpr *right;
            TokenType relop;
        } do_stmt;
        struct {
            int has_condition;
            AstExpr *left;
            AstExpr *right;
            TokenType relop;
        } loop_stmt;
        struct {
            int placeholder;
        } data;
        struct {
            char var_names[26];
            int var_types[26];
            int var_count;
            AstExpr *var_indices[26];
            AstExpr *var_indices2[26];
            char dim_names[26][MAX_VAR_NAME_LEN + 1];
        } read;
        struct {
            AstExpr *selector;
            double targets[64];
            int target_count;
            int is_gosub;
        } on_goto;
        struct {
            char func_name;
            char param_name;
            AstExpr *body;
        } def_fn;
        struct {
            AstExpr *interrupt_number;
        } int_stmt;
        struct {
            char *text;
        } direct_exec;
    } v;
};

typedef struct AstLine {
    double line_number;
    AstStmt *stmts;
} AstLine;

/* Node allocations and destructors */
AstExpr *ast_core_expr_new(AstExprType type);
AstExpr *ast_core_expr_int(long val);
AstExpr *ast_core_expr_float(double val);
AstExpr *ast_core_expr_string(const char *data, int length);
AstExpr *ast_core_expr_var(char name);
AstExpr *ast_core_expr_string_var(char name);
AstExpr *ast_core_expr_named_var(const char *name, int len);
AstExpr *ast_core_expr_binop(AstBinOp op, AstExpr *left, AstExpr *right);
AstExpr *ast_core_expr_unop(AstUnOp op, AstExpr *operand);
AstExpr *ast_core_expr_func(AstFuncId func, int argc, AstExpr *a0, AstExpr *a1, AstExpr *a2);
AstExpr *ast_core_expr_array_at(AstExpr *index);
AstExpr *ast_core_expr_dim_access(const char *name, int nlen, AstExpr *idx1, AstExpr *idx2);
AstStmt *ast_core_stmt_new(AstStmtType type);

void ast_core_free_expr(AstExpr *expr);
void ast_core_free_stmt(AstStmt *stmt);
void ast_core_free_line(AstStmt *stmts);

#endif /* STANDALONE_AST_CORE_H */

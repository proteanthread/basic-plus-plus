/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ast.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Variables lookup, DIM arrays, User-Defined Types (UDT), matrix operations (MAT), and abstract syntax trees (AST).
 *
 * 2. WHAT TO EXPECT:
 *    Static array bounds mapping. Variable lookup hashes utilize fast string indexing.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Maximum array dimension numbers, matrix operators, UDT array constraints.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Row-major layout logic, variable storage offset rules, AST node type enumerations.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check array index limits. Ensure REDIM operations do not corrupt memory (verify array bounds rules).
 * ===================================================================== */

 // ---
 // BASIC++ Compiler - ast.h
 // ---
 //
 // Abstract Syntax Tree (AST) for the BASIC++ compiler subsystem.
 //
 // PURPOSE:
 // The interpreter (parser.c) uses a "parse-and-execute" design -
 // it never builds a tree. The compiler needs a tree representation
 // so that the code generator can walk it multiple times and emit
 // target code. This module defines the AST node types and the
 // builder that constructs them from the token stream.
 //
 // DESIGN RATIONALE:
 // 1. Statements are represented as a flat linked list per line.
 // Each AstLine holds a line number and a chain of AstStmt nodes.
 // 2. Expressions are recursive trees (AstExpr). Binary operators,
 // unary operators, function calls, and literals are all nodes.
 // 3. Memory is allocated via malloc/free (not the interpreter's
 // pool system) because the AST is transient - built for one
 // compilation, then freed.
 // 4. The builder reuses the existing Lexer for tokenization.
 //
 // ---

#ifndef BASICPP_AST_H
#define BASICPP_AST_H

#include "lexer.h"
#include "config.h"

// --- Expression Node Types ---
typedef enum AstExprType {
 EXPR_INT_LIT, // integer literal
 EXPR_FLOAT_LIT, // float literal
 EXPR_STRING_LIT, // string literal
 EXPR_VAR, // single-letter variable A-Z
 EXPR_STRING_VAR, // string variable A$-Z$
 EXPR_NAMED_VAR, // named variable (extended)
 EXPR_ARRAY_AT, // @(expr) legacy array
 EXPR_DIM_ACCESS, // DIM array access: name(expr[,expr])
 EXPR_BINOP, // binary operation: left op right
 EXPR_UNOP, // unary operation: op operand
 EXPR_FUNC_CALL // function call: func(args...)
} AstExprType;

// Binary operator kinds
typedef enum AstBinOp {
 BOP_ADD, BOP_SUB, BOP_MUL, BOP_DIV, BOP_MOD, BOP_POW,
 BOP_EQ, BOP_NE, BOP_LT, BOP_GT, BOP_LE, BOP_GE,
 BOP_AND, BOP_OR,
 BOP_CONCAT // string concatenation via +
} AstBinOp;

// Unary operator kinds
typedef enum AstUnOp {
 UOP_NEG, // unary minus
 UOP_NOT // logical NOT
} AstUnOp;

// Function identifiers (maps to KeywordId for built-in functions)
typedef enum AstFuncId {
 FUNC_ABS, FUNC_RND, FUNC_SIZE,
 FUNC_SIN, FUNC_COS, FUNC_TAN, FUNC_ATN,
 FUNC_SQR, FUNC_LOG, FUNC_EXP, FUNC_SGN, FUNC_INT,
 FUNC_LEN, FUNC_ASC, FUNC_VAL,
 FUNC_CHR, FUNC_STR,
 FUNC_LEFT, FUNC_RIGHT, FUNC_MID,
 FUNC_TAB, // TAB(n) - PRINT column position
 FUNC_FN_USER // User-defined FN (DEF FN)
} AstFuncId;

// Forward declaration
typedef struct AstExpr AstExpr;

 // AstExpr - Expression node (recursive tree).
 //
 // Tagged union. The 'type' field determines which union member
 // is active. Expression trees are built bottom-up by the AST
 // builder and walked top-down by the code generator.
struct AstExpr {
 AstExprType type;
 union {
 long ival; // EXPR_INT_LIT
 double fval; // EXPR_FLOAT_LIT
 struct {
 char *data; // EXPR_STRING_LIT (malloc'd copy)
 int length;
 } sval;
 char var_name; // EXPR_VAR, EXPR_STRING_VAR
 struct {
 char name[MAX_VAR_NAME_LEN + 1];
 int name_len;
 } named; // EXPR_NAMED_VAR
 struct {
 AstExpr *index;
 } array_at; // EXPR_ARRAY_AT
 struct {
 char name[MAX_VAR_NAME_LEN + 1];
 int name_len;
 AstExpr *idx1;
 AstExpr *idx2; // NULL if 1D
 } dim_access; // EXPR_DIM_ACCESS
 struct {
 AstBinOp op;
 AstExpr *left;
 AstExpr *right;
 } binop; // EXPR_BINOP
 struct {
 AstUnOp op;
 AstExpr *operand;
 } unop; // EXPR_UNOP
 struct {
 AstFuncId func;
 AstExpr *args[3]; // up to 3 arguments
 int arg_count;
 char fn_letter; // For FUNC_FN_USER: A-Z
 } func_call; // EXPR_FUNC_CALL
 } v;
};

// --- Statement Node Types ---
typedef enum AstStmtType {
 STMT_PRINT,
 STMT_LET,
 STMT_LET_STRVAR, // A$ = expr
 STMT_LET_ARRAY_AT, // @(expr) = expr
 STMT_LET_DIM, // dim_array(i[,j]) = expr
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
 STMT_ON_GOTO, // ON expr GOTO line1,line2,...
 STMT_DEF_FN // DEF FNA(X) = expr
} AstStmtType;

// Forward declaration
typedef struct AstStmt AstStmt;

 // Print item - one element in a PRINT list.
 // Items can be expressions or format specifiers.
typedef struct AstPrintItem {
 AstExpr *expr; // expression to print (NULL for separator-only)
 int suppress_space; // 1 if followed by ;
 int is_hash_width; // 1 if this is a #width specifier
} AstPrintItem;

 // AstStmt - Statement node (linked list per line).
struct AstStmt {
 AstStmtType type;
 AstStmt *next; // next statement on same line (multi-stmt)
 union {
 // STMT_PRINT
 struct {
 AstPrintItem *items;
 int item_count;
 int trailing_comma; // suppress newline
 } print;

 // STMT_LET
 struct {
 char var_name;
 AstExpr *value;
 } let;

 // STMT_LET_STRVAR
 struct {
 char var_name;
 AstExpr *value;
 } let_strvar;

 // STMT_LET_ARRAY_AT
 struct {
 AstExpr *index;
 AstExpr *value;
 } let_array_at;

 // STMT_LET_DIM
 struct {
 char name[MAX_VAR_NAME_LEN + 1];
 int name_len;
 AstExpr *idx1;
 AstExpr *idx2; // NULL if 1D
 AstExpr *value;
 } let_dim;

 // STMT_IF
 struct {
 AstExpr *condition; // full boolean expression
 AstStmt *then_stmt; // statement to execute if true
 } if_stmt;

 // STMT_GOTO
 struct {
 AstExpr *target; // line number expression
 } goto_stmt;

 // STMT_GOSUB
 struct {
 AstExpr *target;
 } gosub;

 // STMT_FOR
 struct {
 char var_name;
 AstExpr *init;
 AstExpr *limit;
 AstExpr *step; // NULL = default step 1
 } for_stmt;

 // STMT_NEXT
 struct {
 char var_name;
 } next;

 // STMT_INPUT
 struct {
 AstExpr *prompt; // NULL or string literal
 char var_names[26]; // variables to read into
 int var_types[26]; // 0=int, 1=string
 int var_count;
 } input;

 // STMT_DIM
 struct {
 char name[MAX_VAR_NAME_LEN + 1];
 int name_len;
 AstExpr *dim1;
 AstExpr *dim2; // NULL if 1D
 } dim;

 // STMT_REM
 struct {
 const char *text; // comment text (borrowed)
 } rem;

 // STMT_WHILE
 struct {
 AstExpr *left;
 AstExpr *right;
 TokenType relop;
 } while_stmt;

 // STMT_DO
 struct {
 int has_condition; // 0=infinite, 1=WHILE, 2=UNTIL
 AstExpr *left;
 AstExpr *right;
 TokenType relop;
 } do_stmt;

 // STMT_LOOP
 struct {
 int has_condition;
 AstExpr *left;
 AstExpr *right;
 TokenType relop;
 } loop_stmt;

 // STMT_DATA
 struct {
 int placeholder; // DATA is collected separately
 } data;

 // STMT_READ
 struct {
 char var_names[26];
 int var_types[26]; // 0=numeric, 1=string, 2=array(1D), 3=array(2D)
 int var_count;
 AstExpr *var_indices[26]; // subscript 1 for array reads
 AstExpr *var_indices2[26]; // subscript 2 for 2D array reads
 char dim_names[26][MAX_VAR_NAME_LEN + 1]; // full dim name
 } read;

 // STMT_ON_GOTO
 struct {
 AstExpr *selector; // ON <selector> GOTO ...
 int targets[64]; // target line numbers
 int target_count;
 } on_goto;

 // STMT_DEF_FN
 struct {
 char func_name; // function letter (A for FNA)
 char param_name; // parameter variable
 AstExpr *body; // expression body
 } def_fn;

 } v;
};

// --- AstLine - One parsed program line. ---
typedef struct AstLine {
 int line_number;
 AstStmt *stmts; // linked list of statements
} AstLine;

// --- AST Builder Functions ---

 // ast_build_line - Parse a token stream into an AST statement list.
 //
 // The lexer should be positioned after the line number token.
 // Returns a linked list of AstStmt nodes, or NULL on error.
 // Sets error via error_raise on parse failure.
AstStmt *ast_build_line(Lexer *lex, int line_num);

 // ast_build_expr - Parse an expression into an AST tree.
 //
 // Used internally by ast_build_line and also available for
 // testing. Returns an AstExpr tree or NULL on error.
AstExpr *ast_build_expr(Lexer *lex, int line_num);

// --- AST Destructor Functions ---

// Free an expression tree recursively
void ast_free_expr(AstExpr *expr);

// Free a statement and its contained expressions
void ast_free_stmt(AstStmt *stmt);

// Free an entire line's statement list
void ast_free_line(AstStmt *stmts);

#endif // BASICPP_AST_H

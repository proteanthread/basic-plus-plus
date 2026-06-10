/*
 * ---
 * BASIC++ Compiler - ast.c
 * ---
 *
 * AST builder implementation.
 *
 * DESIGN RATIONALE:
 * This module mirrors the structure of parser.c's parse-and-execute
 * functions, but instead of calling runtime functions, it builds
 * AstExpr and AstStmt nodes. The grammar and precedence rules are
 * identical to the interpreter's parser.
 *
 * Memory management uses malloc/free. Each node is individually
 * allocated. The caller (compiler.c) is responsible for calling
 * ast_free_line() after code generation completes.
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "dialect.h"
#include "errors.h"

/* --- Internal forward declarations ---
 */
static AstExpr *build_term(Lexer *lex, int line_num);
static AstExpr *build_power(Lexer *lex, int line_num);
static AstExpr *build_factor(Lexer *lex, int line_num);
static AstExpr *build_additive(Lexer *lex, int line_num);
static AstExpr *build_comparison(Lexer *lex, int line_num);
static AstExpr *build_not_expr(Lexer *lex, int line_num);
static AstExpr *build_and_expr(Lexer *lex, int line_num);
static AstExpr *build_or_expr(Lexer *lex, int line_num);
static AstStmt *build_statement(Lexer *lex, int line_num);

/* --- Expression node constructors ---
 */

static AstExpr *expr_new(AstExprType type)
{
 AstExpr *e = (AstExpr *)malloc(sizeof(AstExpr));
 if (e) {
 memset(e, 0, sizeof(AstExpr));
 e->type = type;
 }
 return e;
}

static AstExpr *expr_int(long val)
{
 AstExpr *e = expr_new(EXPR_INT_LIT);
 if (e) e->v.ival = val;
 return e;
}

static AstExpr *expr_float(double val)
{
 AstExpr *e = expr_new(EXPR_FLOAT_LIT);
 if (e) e->v.fval = val;
 return e;
}

static AstExpr *expr_string(const char *data, int length)
{
 AstExpr *e = expr_new(EXPR_STRING_LIT);
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

static AstExpr *expr_var(char name)
{
 AstExpr *e = expr_new(EXPR_VAR);
 if (e) e->v.var_name = name;
 return e;
}

static AstExpr *expr_string_var(char name)
{
 AstExpr *e = expr_new(EXPR_STRING_VAR);
 if (e) e->v.var_name = name;
 return e;
}

static AstExpr *expr_named_var(const char *name, int len)
{
 AstExpr *e = expr_new(EXPR_NAMED_VAR);
 if (e) {
 int clen = len;
 if (clen > MAX_VAR_NAME_LEN) clen = MAX_VAR_NAME_LEN;
 memcpy(e->v.named.name, name, (size_t)clen);
 e->v.named.name[clen] = '\0';
 e->v.named.name_len = clen;
 }
 return e;
}

static AstExpr *expr_binop(AstBinOp op, AstExpr *left, AstExpr *right)
{
 AstExpr *e = expr_new(EXPR_BINOP);
 if (e) {
 e->v.binop.op = op;
 e->v.binop.left = left;
 e->v.binop.right = right;
 }
 return e;
}

static AstExpr *expr_unop(AstUnOp op, AstExpr *operand)
{
 AstExpr *e = expr_new(EXPR_UNOP);
 if (e) {
 e->v.unop.op = op;
 e->v.unop.operand = operand;
 }
 return e;
}

static AstExpr *expr_func(AstFuncId func, int argc, AstExpr *a0,
 AstExpr *a1, AstExpr *a2)
{
 AstExpr *e = expr_new(EXPR_FUNC_CALL);
 if (e) {
 e->v.func_call.func = func;
 e->v.func_call.arg_count = argc;
 e->v.func_call.args[0] = a0;
 e->v.func_call.args[1] = a1;
 e->v.func_call.args[2] = a2;
 }
 return e;
}

static AstExpr *expr_array_at(AstExpr *index)
{
 AstExpr *e = expr_new(EXPR_ARRAY_AT);
 if (e) e->v.array_at.index = index;
 return e;
}

static AstExpr *expr_dim_access(const char *name, int nlen,
 AstExpr *idx1, AstExpr *idx2)
{
 AstExpr *e = expr_new(EXPR_DIM_ACCESS);
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

/* --- Statement node constructor ---
 */
static AstStmt *stmt_new(AstStmtType type)
{
 AstStmt *s = (AstStmt *)malloc(sizeof(AstStmt));
 if (s) {
 memset(s, 0, sizeof(AstStmt));
 s->type = type;
 }
 return s;
}

/* --- Expression Builder - mirrors parse_expression_bval ---
 */

/*
 * build_factor - Parse atomic expression into AST node.
 */
static AstExpr *build_factor(Lexer *lex, int line_num)
{
 if (error_occurred()) return NULL;

 switch (lex->current.type) {
 case TOK_NUMBER:
 {
 long val = lex->current.value.num_value;
 lexer_next(lex);
 return expr_int(val);
 }

 case TOK_FLOAT_LIT:
 {
 double val = lex->current.value.fval;
 lexer_next(lex);
 return expr_float(val);
 }

 case TOK_STRING:
 {
 AstExpr *e = expr_string(lex->current.str_start,
 lex->current.str_length);
 lexer_next(lex);
 return e;
 }

 case TOK_VARIABLE:
 {
 char name = lex->current.value.var_name;
 lexer_next(lex);
 /* Check for DIM array access: A(...) */
 if (lex->current.type == TOK_LPAREN) {
 AstExpr *idx1, *idx2 = NULL;
 char nm[2];
 nm[0] = name; nm[1] = '\0';
 lexer_next(lex); /* consume ( */
 idx1 = ast_build_expr(lex, line_num);
 if (error_occurred()) return NULL;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1);
 return NULL;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx1);
 ast_free_expr(idx2);
 return NULL;
 }
 return expr_dim_access(nm, 1, idx1, idx2);
 }
 return expr_var(name);
 }

 case TOK_STRING_VAR:
 {
 char name = lex->current.value.var_name;
 lexer_next(lex);
 /* Check for string array subscript: A$(idx) */
 if (lex->current.type == TOK_LPAREN) {
 char nm[3];
 AstExpr *idx1, *idx2 = NULL;
 nm[0] = name; nm[1] = '$'; nm[2] = '\0';
 lexer_next(lex);
 idx1 = ast_build_expr(lex, line_num);
 if (error_occurred()) return NULL;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1); return NULL;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 return NULL;
 }
 return expr_dim_access(nm, 2, idx1, idx2);
 }
 return expr_string_var(name);
 }

 case TOK_NAMED_VAR:
 {
 char nm[MAX_VAR_NAME_LEN + 1];
 int nlen = lex->current.str_length;
 if (nlen > MAX_VAR_NAME_LEN) nlen = MAX_VAR_NAME_LEN;
 memcpy(nm, lex->current.str_start, (size_t)nlen);
 nm[nlen] = '\0';

 /* Check for FN* user-defined function call */
 if (nlen >= 3 &&
 (nm[0] == 'F' || nm[0] == 'f') &&
 (nm[1] == 'N' || nm[1] == 'n')) {
 char fc = nm[2];
 AstExpr *arg, *fn_expr;
 if (fc >= 'a' && fc <= 'z') fc = (char)(fc - 32);
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN)) return NULL;
 arg = ast_build_expr(lex, line_num);
 if (error_occurred()) return NULL;
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(arg);
 return NULL;
 }
 /* Create a special expression for FN call */
 fn_expr = (AstExpr *)calloc(1, sizeof(AstExpr));
 fn_expr->type = EXPR_FUNC_CALL;
 fn_expr->v.func_call.func = FUNC_FN_USER;
 fn_expr->v.func_call.args[0] = arg;
 fn_expr->v.func_call.fn_letter = fc;
 return fn_expr;
 }

 lexer_next(lex);
 /* Check for DIM array access */
 if (lex->current.type == TOK_LPAREN) {
 AstExpr *idx1, *idx2 = NULL;
 lexer_next(lex);
 idx1 = ast_build_expr(lex, line_num);
 if (error_occurred()) return NULL;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1);
 return NULL;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx1);
 ast_free_expr(idx2);
 return NULL;
 }
 return expr_dim_access(nm, nlen, idx1, idx2);
 }
 return expr_named_var(nm, nlen);
 }

 case TOK_AT:
 {
 AstExpr *idx;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN)) return NULL;
 idx = ast_build_expr(lex, line_num);
 if (error_occurred()) return NULL;
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx);
 return NULL;
 }
 return expr_array_at(idx);
 }

 case TOK_LPAREN:
 {
 AstExpr *e;
 lexer_next(lex);
 e = ast_build_expr(lex, line_num);
 if (error_occurred()) return NULL;
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(e);
 return NULL;
 }
 return e;
 }

 case TOK_KEYWORD:
 {
 KeywordId kw = lex->current.value.keyword;
 AstFuncId fid;
 int argc = 1;
 AstExpr *a0 = NULL, *a1 = NULL, *a2 = NULL;

 /* Map keyword to function ID */
 switch (kw) {
 case KW_ABS: fid = FUNC_ABS; break;
 case KW_RND: fid = FUNC_RND; break;
 case KW_SIZE:
 lexer_next(lex);
 return expr_func(FUNC_SIZE, 0, NULL, NULL, NULL);
 case KW_SIN: fid = FUNC_SIN; break;
 case KW_COS: fid = FUNC_COS; break;
 case KW_TAN: fid = FUNC_TAN; break;
 case KW_ATN: fid = FUNC_ATN; break;
 case KW_SQR: fid = FUNC_SQR; break;
 case KW_LOG_FUNC: fid = FUNC_LOG; break;
 case KW_EXP: fid = FUNC_EXP; break;
 case KW_SGN: fid = FUNC_SGN; break;
 case KW_INT_FUNC: fid = FUNC_INT; break;
 case KW_LEN: fid = FUNC_LEN; break;
 case KW_ASC: fid = FUNC_ASC; break;
 case KW_VAL_FUNC: fid = FUNC_VAL; break;
 case KW_CHR: fid = FUNC_CHR; break;
 case KW_STR_FUNC: fid = FUNC_STR; break;
 case KW_LEFT: fid = FUNC_LEFT; argc = 2; break;
 case KW_RIGHT: fid = FUNC_RIGHT; argc = 2; break;
 case KW_MID: fid = FUNC_MID; argc = 3; break;
 case KW_TAB_FUNC: fid = FUNC_TAB; break;
 default:
 error_raise(ERR_WHAT, line_num);
 return NULL;
 }

 lexer_next(lex); /* consume keyword */
 if (!lexer_expect(lex, TOK_LPAREN)) return NULL;

 a0 = ast_build_expr(lex, line_num);
 if (error_occurred()) return NULL;

 if (argc >= 2) {
 if (!lexer_expect(lex, TOK_COMMA)) {
 ast_free_expr(a0);
 return NULL;
 }
 a1 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(a0);
 return NULL;
 }
 }
 if (argc >= 3) {
 if (!lexer_expect(lex, TOK_COMMA)) {
 ast_free_expr(a0);
 ast_free_expr(a1);
 return NULL;
 }
 a2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(a0);
 ast_free_expr(a1);
 return NULL;
 }
 }

 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(a0);
 ast_free_expr(a1);
 ast_free_expr(a2);
 return NULL;
 }

 return expr_func(fid, argc, a0, a1, a2);
 }

 default:
 error_raise(ERR_WHAT, line_num);
 return NULL;
 }
}

/*
 * build_power - Parse exponentiation (right-associative).
 */
static AstExpr *build_power(Lexer *lex, int line_num)
{
 AstExpr *left;
 left = build_factor(lex, line_num);
 if (error_occurred()) return NULL;

 if (lex->current.type == TOK_CARET) {
 AstExpr *right;
 lexer_next(lex);
 right = build_power(lex, line_num); /* right-associative */
 if (error_occurred()) { ast_free_expr(left); return NULL; }
 left = expr_binop(BOP_POW, left, right);
 }

 return left;
}

/*
 * build_term - Parse multiplicative expression.
 */
static AstExpr *build_term(Lexer *lex, int line_num)
{
 AstExpr *left;
 left = build_power(lex, line_num);
 if (error_occurred()) return NULL;

 while (lex->current.type == TOK_STAR ||
 lex->current.type == TOK_SLASH) {
 AstBinOp op;
 AstExpr *right;
 op = (lex->current.type == TOK_STAR) ? BOP_MUL : BOP_DIV;
 lexer_next(lex);
 right = build_power(lex, line_num);
 if (error_occurred()) { ast_free_expr(left); return NULL; }
 left = expr_binop(op, left, right);
 }

 return left;
}

/*
 * build_additive - Parse additive expression (+ -).
 */
static AstExpr *build_additive(Lexer *lex, int line_num)
{
 AstExpr *left;
 int negate = 0;

 if (error_occurred()) return NULL;

 /* Optional leading sign */
 if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(lex);
 }

 left = build_term(lex, line_num);
 if (error_occurred()) return NULL;

 if (negate) {
 left = expr_unop(UOP_NEG, left);
 }

 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 AstBinOp op;
 AstExpr *right;
 op = (lex->current.type == TOK_PLUS) ? BOP_ADD : BOP_SUB;
 lexer_next(lex);
 right = build_term(lex, line_num);
 if (error_occurred()) { ast_free_expr(left); return NULL; }
 left = expr_binop(op, left, right);
 }

 return left;
}

/*
 * build_comparison - Parse comparison operators (=, <>, <, >, <=, >=).
 */
static AstExpr *build_comparison(Lexer *lex, int line_num)
{
 AstExpr *left;
 left = build_additive(lex, line_num);
 if (error_occurred()) return NULL;

 while (lex->current.type == TOK_EQUALS ||
 lex->current.type == TOK_NOT_EQ ||
 lex->current.type == TOK_LT ||
 lex->current.type == TOK_GT ||
 lex->current.type == TOK_LT_EQ ||
 lex->current.type == TOK_GT_EQ ||
 lex->current.type == TOK_HASH) {
 AstBinOp op;
 AstExpr *right;
 switch (lex->current.type) {
 case TOK_EQUALS: op = BOP_EQ; break;
 case TOK_NOT_EQ: op = BOP_NE; break;
 case TOK_LT: op = BOP_LT; break;
 case TOK_GT: op = BOP_GT; break;
 case TOK_LT_EQ: op = BOP_LE; break;
 case TOK_GT_EQ: op = BOP_GE; break;
 case TOK_HASH: op = BOP_NE; break;
 default: op = BOP_EQ; break;
 }
 lexer_next(lex);
 right = build_additive(lex, line_num);
 if (error_occurred()) { ast_free_expr(left); return NULL; }
 left = expr_binop(op, left, right);
 }

 return left;
}

/*
 * build_not_expr - Parse NOT prefix operator.
 */
static AstExpr *build_not_expr(Lexer *lex, int line_num)
{
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NOT) {
 AstExpr *operand;
 lexer_next(lex);
 operand = build_not_expr(lex, line_num);
 if (error_occurred()) return NULL;
 return expr_unop(UOP_NOT, operand);
 }
 return build_comparison(lex, line_num);
}

/*
 * build_and_expr - Parse AND logical operator.
 */
static AstExpr *build_and_expr(Lexer *lex, int line_num)
{
 AstExpr *left;
 left = build_not_expr(lex, line_num);
 if (error_occurred()) return NULL;

 while (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_AND) {
 AstExpr *right;
 lexer_next(lex);
 right = build_not_expr(lex, line_num);
 if (error_occurred()) { ast_free_expr(left); return NULL; }
 left = expr_binop(BOP_AND, left, right);
 }

 return left;
}

/*
 * build_or_expr - Parse OR logical operator.
 */
static AstExpr *build_or_expr(Lexer *lex, int line_num)
{
 AstExpr *left;
 left = build_and_expr(lex, line_num);
 if (error_occurred()) return NULL;

 while (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_OR) {
 AstExpr *right;
 lexer_next(lex);
 right = build_and_expr(lex, line_num);
 if (error_occurred()) { ast_free_expr(left); return NULL; }
 left = expr_binop(BOP_OR, left, right);
 }

 return left;
}

/*
 * ast_build_expr - Parse a full expression (top-level, includes logic).
 */
AstExpr *ast_build_expr(Lexer *lex, int line_num)
{
 return build_or_expr(lex, line_num);
}

/* --- Statement Builders ---
 */

/*
 * build_print - Parse PRINT statement into AST.
 */
static AstStmt *build_print(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_PRINT);
 AstPrintItem *items = NULL;
 int count = 0;
 int capacity = 8;
 char sep = dialect_get_separator();

 if (!s) return NULL;

 items = (AstPrintItem *)malloc(sizeof(AstPrintItem) * (size_t)capacity);
 if (!items) { free(s); return NULL; }

 s->v.print.trailing_comma = 0;

 /* Handle empty PRINT (newline only) */
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type == TOK_COLON && sep == ':') ||
 (lex->current.type == TOK_SEMICOLON && sep == ';')) {
 s->v.print.items = items;
 s->v.print.item_count = 0;
 return s;
 }

 while (!error_occurred()) {
 AstPrintItem item;
 memset(&item, 0, sizeof(item));

 /* Handle comma at current position (leading or double comma) */
 if (lex->current.type == TOK_COMMA) {
 /* Insert a NULL-expr item to represent tab advance */
 item.expr = NULL;
 item.suppress_space = 0;
 if (count >= capacity) {
 capacity *= 2;
 items = (AstPrintItem *)realloc(items,
 sizeof(AstPrintItem) * (size_t)capacity);
 }
 items[count++] = item;
 lexer_next(lex);
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type == TOK_COLON && sep == ':')) {
 s->v.print.trailing_comma = 1;
 break;
 }
 continue;
 }

 /* Check for #width */
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex);
 item.expr = ast_build_expr(lex, line_num);
 if (error_occurred()) break;
 item.is_hash_width = 1;
 } else {
 item.expr = ast_build_expr(lex, line_num);
 if (error_occurred()) break;
 }

 /* Store item */
 if (count >= capacity) {
 capacity *= 2;
 items = (AstPrintItem *)realloc(items,
 sizeof(AstPrintItem) * (size_t)capacity);
 }
 items[count++] = item;

 /* Check separator */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 items[count - 1].suppress_space = 0;
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type == TOK_COLON && sep == ':')) {
 s->v.print.trailing_comma = 1;
 break;
 }
 } else if (lex->current.type == TOK_SEMICOLON) {
 lexer_next(lex);
 items[count - 1].suppress_space = 1;
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type == TOK_COLON && sep == ':')) {
 s->v.print.trailing_comma = 1;
 break;
 }
 } else {
 break;
 }
 }

 if (error_occurred()) {
 int i;
 for (i = 0; i < count; i++) ast_free_expr(items[i].expr);
 free(items);
 free(s);
 return NULL;
 }

 s->v.print.items = items;
 s->v.print.item_count = count;
 return s;
}

/*
 * build_let - Parse LET/assignment into AST.
 */
static AstStmt *build_let(Lexer *lex, int line_num)
{
 /* @() array assignment */
 if (lex->current.type == TOK_AT) {
 AstStmt *s = stmt_new(STMT_LET_ARRAY_AT);
 AstExpr *idx, *val;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN)) { free(s); return NULL; }
 idx = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx); free(s); return NULL;
 }
 if (!lexer_expect(lex, TOK_EQUALS)) {
 ast_free_expr(idx); free(s); return NULL;
 }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { ast_free_expr(idx); free(s); return NULL; }
 s->v.let_array_at.index = idx;
 s->v.let_array_at.value = val;
 return s;
 }

 /* Standard variable */
 if (lex->current.type == TOK_VARIABLE) {
 char name = lex->current.value.var_name;
 lexer_next(lex);

 /* Check for DIM array assignment: A(i) = expr */
 if (lex->current.type == TOK_LPAREN) {
 AstStmt *s = stmt_new(STMT_LET_DIM);
 AstExpr *idx1, *idx2 = NULL, *val;
 lexer_next(lex);
 idx1 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1); free(s); return NULL;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 if (!lexer_expect(lex, TOK_EQUALS)) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 s->v.let_dim.name[0] = name;
 s->v.let_dim.name[1] = '\0';
 s->v.let_dim.name_len = 1;
 s->v.let_dim.idx1 = idx1;
 s->v.let_dim.idx2 = idx2;
 s->v.let_dim.value = val;
 return s;
 }

 /* Simple variable assignment */
 {
 AstStmt *s = stmt_new(STMT_LET);
 AstExpr *val;
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let.var_name = name;
 s->v.let.value = val;
 return s;
 }
 }

 /* Named variable */
 if (lex->current.type == TOK_NAMED_VAR) {
 char nm[MAX_VAR_NAME_LEN + 1];
 int nlen = lex->current.str_length;
 if (nlen > MAX_VAR_NAME_LEN) nlen = MAX_VAR_NAME_LEN;
 memcpy(nm, lex->current.str_start, (size_t)nlen);
 nm[nlen] = '\0';
 lexer_next(lex);

 /* Check for DIM array assignment: A1(i) = expr or T1(G1) = expr */
 if (lex->current.type == TOK_LPAREN) {
 AstStmt *s = stmt_new(STMT_LET_DIM);
 AstExpr *idx1, *idx2 = NULL, *val;
 lexer_next(lex);
 idx1 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1); free(s); return NULL;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 if (!lexer_expect(lex, TOK_EQUALS)) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 memcpy(s->v.let_dim.name, nm, (size_t)(nlen + 1));
 s->v.let_dim.name_len = nlen;
 s->v.let_dim.idx1 = idx1;
 s->v.let_dim.idx2 = idx2;
 s->v.let_dim.value = val;
 return s;
 }

 /* Simple named variable assignment */
 {
 int is_str = (nlen > 0 && nm[nlen - 1] == '$');
 if (is_str) {
 /* String named var: X0$ = expr */
 AstStmt *s = stmt_new(STMT_LET_STRVAR);
 AstExpr *val;
 s->v.let_strvar.var_name = nm[0];
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let_strvar.value = val;
 return s;
 } else {
 AstStmt *s = stmt_new(STMT_LET);
 AstExpr *val;
 s->v.let.var_name = nm[0];
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let.value = val;
 return s;
 }
 }
 }

 /* String variable */
 if (lex->current.type == TOK_STRING_VAR) {
 char sv_name = lex->current.value.var_name;
 lexer_next(lex);
 /* Check for string array subscript: A$(idx) = expr */
 if (lex->current.type == TOK_LPAREN) {
 AstStmt *s = stmt_new(STMT_LET_DIM);
 AstExpr *val;
 s->v.let_dim.name[0] = sv_name;
 s->v.let_dim.name[1] = '$';
 s->v.let_dim.name[2] = '\0';
 lexer_next(lex); /* consume ( */
 s->v.let_dim.idx1 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let_dim.idx2 = NULL;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 s->v.let_dim.idx2 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) { free(s); return NULL; }
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let_dim.value = val;
 return s;
 }
 /* Simple string variable: A$ = expr */
 {
 AstStmt *s = stmt_new(STMT_LET_STRVAR);
 AstExpr *val;
 s->v.let_strvar.var_name = sv_name;
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let_strvar.value = val;
 return s;
 }
 }

 error_raise(ERR_WHAT, line_num);
 return NULL;
}

/*
 * build_if - Parse IF statement into AST.
 *
 * Now uses a full boolean expression for the condition.
 * Comparisons (=, <>, <, >, <=, >=) and logical operators
 * (AND, OR, NOT) are handled by the expression parser.
 */
static AstStmt *build_if(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_IF);

 if (!s) return NULL;

 /* Parse the full condition expression */
 s->v.if_stmt.condition = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }

 /* Optional THEN keyword */
 if (dialect_get_config()->has_then_keyword &&
 lexer_match_keyword(lex, KW_THEN)) {
 lexer_next(lex);
 }

 /* IF cond THEN <linenumber> - synthesize GOTO */
 if (lex->current.type == TOK_NUMBER) {
 AstStmt *g = stmt_new(STMT_GOTO);
 g->v.goto_stmt.target = ast_build_expr(lex, line_num);
 s->v.if_stmt.then_stmt = g;
 return s;
 }

 s->v.if_stmt.then_stmt = build_statement(lex, line_num);
 return s;
}

/*
 * build_for - Parse FOR statement.
 */
static AstStmt *build_for(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_FOR);
 if (!s) return NULL;

 if (lex->current.type == TOK_VARIABLE) {
 s->v.for_stmt.var_name = lex->current.value.var_name;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR) {
 /* Multi-char var (R1, Q4, DELAY) - use first letter */
 s->v.for_stmt.var_name = (char)(lex->current.str_start[0]);
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 free(s); return NULL;
 }

 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }

 s->v.for_stmt.init = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }

 /* Expect TO keyword */
 if (!lexer_match_keyword(lex, KW_TO)) {
 error_raise(ERR_WHAT, line_num);
 ast_free_expr(s->v.for_stmt.init); free(s);
 return NULL;
 }
 lexer_next(lex);

 s->v.for_stmt.limit = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(s->v.for_stmt.init); free(s);
 return NULL;
 }

 /* Optional STEP */
 s->v.for_stmt.step = NULL;
 if (lexer_match_keyword(lex, KW_STEP)) {
 lexer_next(lex);
 s->v.for_stmt.step = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(s->v.for_stmt.init);
 ast_free_expr(s->v.for_stmt.limit);
 free(s); return NULL;
 }
 }

 return s;
}

/*
 * build_input - Parse INPUT statement.
 */
static AstStmt *build_input(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_INPUT);
 (void)line_num; /* used by ast_build_expr below */
 if (!s) return NULL;

 s->v.input.prompt = NULL;
 s->v.input.var_count = 0;

 /* Optional prompt string */
 if (lex->current.type == TOK_STRING) {
 s->v.input.prompt = expr_string(lex->current.str_start,
 lex->current.str_length);
 lexer_next(lex);
 if (lex->current.type == TOK_COMMA) lexer_next(lex);
 }

 /* Variable list */
 while (!error_occurred()) {
 int idx = s->v.input.var_count;
 if (idx >= 26) break;

 if (lex->current.type == TOK_VARIABLE) {
 s->v.input.var_names[idx] = lex->current.value.var_name;
 s->v.input.var_types[idx] = 0;
 s->v.input.var_count++;
 lexer_next(lex);
 } else if (lex->current.type == TOK_STRING_VAR) {
 s->v.input.var_names[idx] = lex->current.value.var_name;
 s->v.input.var_types[idx] = 1;
 s->v.input.var_count++;
 lexer_next(lex);
 } else {
 break;
 }

 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 } else {
 break;
 }
 }

 return s;
}

/*
 * build_dim - Parse DIM statement.
 */
static AstStmt *build_dim(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_DIM);
 if (!s) return NULL;

 if (lex->current.type == TOK_VARIABLE) {
 s->v.dim.name[0] = lex->current.value.var_name;
 s->v.dim.name[1] = '\0';
 s->v.dim.name_len = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_STRING_VAR) {
 /* String array: DIM A$(20) */
 s->v.dim.name[0] = lex->current.value.var_name;
 s->v.dim.name[1] = '$';
 s->v.dim.name[2] = '\0';
 s->v.dim.name_len = 2;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR) {
 int nlen = lex->current.str_length;
 if (nlen > MAX_VAR_NAME_LEN) nlen = MAX_VAR_NAME_LEN;
 memcpy(s->v.dim.name, lex->current.str_start, (size_t)nlen);
 s->v.dim.name[nlen] = '\0';
 s->v.dim.name_len = nlen;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 free(s); return NULL;
 }

 if (!lexer_expect(lex, TOK_LPAREN)) { free(s); return NULL; }

 s->v.dim.dim1 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }

 s->v.dim.dim2 = NULL;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 s->v.dim.dim2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(s->v.dim.dim1);
 free(s); return NULL;
 }
 }

 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(s->v.dim.dim1);
 ast_free_expr(s->v.dim.dim2);
 free(s); return NULL;
 }

 /* Handle comma-separated multiple DIMs: DIM A$(20),B(10),C(5) */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 s->next = build_dim(lex, line_num);
 }

 return s;
}

/*
 * build_statement - Parse one statement into AST node.
 */
static AstStmt *build_statement(Lexer *lex, int line_num)
{
 if (error_occurred()) return NULL;

 if (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;

 /* Handle REM before lexer_next - the text after REM
 * is raw comment, not valid tokens */
 if (kw == KW_REM) {
 AstStmt *s = stmt_new(STMT_REM);
 /* Position past "REM" keyword */
 s->v.rem.text = lex->source + lex->pos;
 lexer_skip_to_end(lex);
 return s;
 }
 }

 /* Handle identifiers starting with REM (e.g. REMARKABLE, REMEMBER)
 * GW-BASIC greedily matches REM and treats rest as comment */
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length >= 3) {
 char c0 = lex->current.str_start[0];
 char c1 = lex->current.str_start[1];
 char c2 = lex->current.str_start[2];
 if ((c0 == 'R' || c0 == 'r') &&
 (c1 == 'E' || c1 == 'e') &&
 (c2 == 'M' || c2 == 'm')) {
 AstStmt *s = stmt_new(STMT_REM);
 /* Include everything from after REM to end of line */
 s->v.rem.text = lex->current.str_start + 3;
 lexer_skip_to_end(lex);
 return s;
 }
 }

 if (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;

 lexer_next(lex);

 switch (kw) {
 case KW_PRINT:
 return build_print(lex, line_num);
 case KW_LET:
 return build_let(lex, line_num);
 case KW_INPUT:
 return build_input(lex, line_num);
 case KW_IF:
 return build_if(lex, line_num);
 case KW_GOTO:
 {
 AstStmt *s = stmt_new(STMT_GOTO);
 s->v.goto_stmt.target = ast_build_expr(lex, line_num);
 return s;
 }
 case KW_GOSUB:
 {
 AstStmt *s = stmt_new(STMT_GOSUB);
 s->v.gosub.target = ast_build_expr(lex, line_num);
 return s;
 }
 case KW_RETURN:
 return stmt_new(STMT_RETURN);
 case KW_FOR:
 return build_for(lex, line_num);
 case KW_NEXT:
 {
 AstStmt *s = stmt_new(STMT_NEXT);
 if (lex->current.type == TOK_VARIABLE) {
 s->v.next.var_name = lex->current.value.var_name;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR) {
 s->v.next.var_name = (char)(lex->current.str_start[0]);
 lexer_next(lex);
 } else {
 s->v.next.var_name = '\0';
 }
 return s;
 }
 case KW_END:
 return stmt_new(STMT_END);
 case KW_STOP:
 return stmt_new(STMT_STOP);
 case KW_REM:
 /* Handled before lexer_next above; unreachable */
 break;
 case KW_DIM:
 return build_dim(lex, line_num);
 case KW_DATA:
 {
 AstStmt *s = stmt_new(STMT_DATA);
 /* Skip DATA values - collected separately */
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 lexer_next(lex);
 }
 return s;
 }
 case KW_READ:
 {
 AstStmt *s = stmt_new(STMT_READ);
 int ri;
 s->v.read.var_count = 0;
 for (ri = 0; ri < 26; ri++) {
 s->v.read.var_indices[ri] = NULL;
 s->v.read.var_indices2[ri] = NULL;
 s->v.read.dim_names[ri][0] = '\0';
 }
 while (!error_occurred()) {
 int idx = s->v.read.var_count;
 if (idx >= 26) break;
 if (lex->current.type == TOK_VARIABLE) {
 char vn = lex->current.value.var_name;
 s->v.read.var_names[idx] = vn;
 s->v.read.dim_names[idx][0] = vn;
 s->v.read.dim_names[idx][1] = '\0';
 lexer_next(lex);
 /* Check for array subscript: READ A(expr) or A(e1,e2) */
 if (lex->current.type == TOK_LPAREN) {
 AstExpr *subscr, *subscr2 = NULL;
 lexer_next(lex);
 subscr = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 subscr2 = ast_build_expr(lex, line_num);
 if (error_occurred()) { ast_free_expr(subscr); free(s); return NULL; }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(subscr);
 ast_free_expr(subscr2);
 free(s); return NULL;
 }
 s->v.read.var_indices[idx] = subscr;
 s->v.read.var_indices2[idx] = subscr2;
 s->v.read.var_types[idx] = subscr2 ? 3 : 2;
 } else {
 s->v.read.var_types[idx] = 0;
 }
 s->v.read.var_count++;
 } else if (lex->current.type == TOK_NAMED_VAR) {
 /* Named var like M, B, etc. with possible subscript */
 int nlen = lex->current.str_length;
 char vn = lex->current.str_start[0];
 s->v.read.var_names[idx] = vn;
 if (nlen > MAX_VAR_NAME_LEN) nlen = MAX_VAR_NAME_LEN;
 memcpy(s->v.read.dim_names[idx],
 lex->current.str_start, (size_t)nlen);
 s->v.read.dim_names[idx][nlen] = '\0';
 lexer_next(lex);
 if (lex->current.type == TOK_LPAREN) {
 AstExpr *subscr, *subscr2 = NULL;
 lexer_next(lex);
 subscr = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 subscr2 = ast_build_expr(lex, line_num);
 if (error_occurred()) { ast_free_expr(subscr); free(s); return NULL; }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(subscr);
 ast_free_expr(subscr2);
 free(s); return NULL;
 }
 s->v.read.var_indices[idx] = subscr;
 s->v.read.var_indices2[idx] = subscr2;
 s->v.read.var_types[idx] = subscr2 ? 3 : 2;
 } else {
 s->v.read.var_types[idx] = 0;
 }
 s->v.read.var_count++;
 } else if (lex->current.type == TOK_STRING_VAR) {
 s->v.read.var_names[idx] =
 lex->current.value.var_name;
 s->v.read.var_types[idx] = 1;
 s->v.read.var_count++;
 lexer_next(lex);
 } else {
 break;
 }
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 else
 break;
 }
 return s;
 }
 case KW_RESTORE:
 return stmt_new(STMT_RESTORE);
 case KW_WHILE:
 {
 AstStmt *s = stmt_new(STMT_WHILE);
 s->v.while_stmt.left =
 ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.while_stmt.relop = lex->current.type;
 lexer_next(lex);
 s->v.while_stmt.right =
 ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(s->v.while_stmt.left);
 free(s); return NULL;
 }
 return s;
 }
 case KW_WEND:
 return stmt_new(STMT_WEND);
 case KW_DO:
 return stmt_new(STMT_DO);
 case KW_LOOP:
 return stmt_new(STMT_LOOP);
 case KW_ON:
 {
 AstStmt *s = stmt_new(STMT_ON_GOTO);
 s->v.on_goto.selector =
 ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 /* Expect GOTO or GOSUB */
 if (!lexer_match_keyword(lex, KW_GOTO)) {
 error_raise(ERR_WHAT, line_num);
 ast_free_expr(s->v.on_goto.selector);
 free(s); return NULL;
 }
 lexer_next(lex);
 s->v.on_goto.target_count = 0;
 while (!error_occurred()) {
 int ti = s->v.on_goto.target_count;
 if (ti >= 64) break;
 if (lex->current.type != TOK_NUMBER) break;
 s->v.on_goto.targets[ti] =
 (int)lex->current.value.num_value;
 s->v.on_goto.target_count++;
 lexer_next(lex);
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 else
 break;
 }
 return s;
 }
 case KW_DEF:
 {
 AstStmt *s = stmt_new(STMT_DEF_FN);
 /* Handle both "DEF FN A(X)" and "DEF FNA(X)" patterns */
 if (lexer_match_keyword(lex, KW_FN)) {
 /* Pattern: DEF FN A(X) */
 lexer_next(lex);
 if (lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 free(s); return NULL;
 }
 s->v.def_fn.func_name = lex->current.value.var_name;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length >= 3 &&
 (lex->current.str_start[0] == 'F' ||
 lex->current.str_start[0] == 'f') &&
 (lex->current.str_start[1] == 'N' ||
 lex->current.str_start[1] == 'n')) {
 /* Pattern: DEF FNA(X) - "FNA" as named var */
 char fc = lex->current.str_start[2];
 if (fc >= 'a' && fc <= 'z') fc = (char)(fc - 32);
 s->v.def_fn.func_name = fc;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 free(s); return NULL;
 }
 /* Expect (param) */
 if (!lexer_expect(lex, TOK_LPAREN)) {
 free(s); return NULL;
 }
 if (lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 free(s); return NULL;
 }
 s->v.def_fn.param_name = lex->current.value.var_name;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_RPAREN)) {
 free(s); return NULL;
 }
 /* Expect = */
 if (!lexer_expect(lex, TOK_EQUALS)) {
 free(s); return NULL;
 }
 s->v.def_fn.body = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 return s;
 }
 case KW_RUN:
 {
 /* RUN "MENU" or RUN - skip arguments, emit as comment */
 AstStmt *s = stmt_new(STMT_REM);
 s->v.rem.text = "RUN (compiled: restart)";
 /* Skip any arguments */
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 lexer_next(lex);
 }
 return s;
 }
 default:
 error_raise(ERR_WHAT, line_num);
 return NULL;
 }
 }

 /* Bare assignment (LET optional) */
 if ((lex->current.type == TOK_VARIABLE ||
 lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_STRING_VAR ||
 lex->current.type == TOK_AT) &&
 dialect_get_config()->has_let_optional) {
 return build_let(lex, line_num);
 }

 if (lex->current.type != TOK_EOF && lex->current.type != TOK_CR) {
 error_raise(ERR_WHAT, line_num);
 }
 return NULL;
}

/* --- Public API ---
 */

/*
 * ast_build_line - Parse a full line into a statement chain.
 */
AstStmt *ast_build_line(Lexer *lex, int line_num)
{
 AstStmt *head = NULL;
 AstStmt *tail = NULL;
 char sep = dialect_get_separator();

 while (!error_occurred() &&
 lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {

 AstStmt *s = build_statement(lex, line_num);
 if (!s) break;

 if (head == NULL) {
 head = s;
 tail = s;
 } else {
 tail->next = s;
 tail = s;
 }

 /* Check for statement separator */
 if (lex->current.type == TOK_SEMICOLON && sep == ';') {
 lexer_next(lex);
 } else if (lex->current.type == TOK_COLON && sep == ':') {
 lexer_next(lex);
 } else {
 break;
 }
 }

 if (error_occurred()) {
 ast_free_line(head);
 return NULL;
 }

 return head;
}

/* --- Destructor Functions ---
 */

void ast_free_expr(AstExpr *expr)
{
 if (!expr) return;

 switch (expr->type) {
 case EXPR_STRING_LIT:
 free(expr->v.sval.data);
 break;
 case EXPR_ARRAY_AT:
 ast_free_expr(expr->v.array_at.index);
 break;
 case EXPR_DIM_ACCESS:
 ast_free_expr(expr->v.dim_access.idx1);
 ast_free_expr(expr->v.dim_access.idx2);
 break;
 case EXPR_BINOP:
 ast_free_expr(expr->v.binop.left);
 ast_free_expr(expr->v.binop.right);
 break;
 case EXPR_UNOP:
 ast_free_expr(expr->v.unop.operand);
 break;
 case EXPR_FUNC_CALL:
 {
 int i;
 for (i = 0; i < expr->v.func_call.arg_count; i++) {
 ast_free_expr(expr->v.func_call.args[i]);
 }
 }
 break;
 default:
 break;
 }
 free(expr);
}

void ast_free_stmt(AstStmt *stmt)
{
 if (!stmt) return;

 switch (stmt->type) {
 case STMT_PRINT:
 {
 int i;
 for (i = 0; i < stmt->v.print.item_count; i++) {
 ast_free_expr(stmt->v.print.items[i].expr);
 }
 free(stmt->v.print.items);
 }
 break;
 case STMT_LET:
 ast_free_expr(stmt->v.let.value);
 break;
 case STMT_LET_STRVAR:
 ast_free_expr(stmt->v.let_strvar.value);
 break;
 case STMT_LET_ARRAY_AT:
 ast_free_expr(stmt->v.let_array_at.index);
 ast_free_expr(stmt->v.let_array_at.value);
 break;
 case STMT_LET_DIM:
 ast_free_expr(stmt->v.let_dim.idx1);
 ast_free_expr(stmt->v.let_dim.idx2);
 ast_free_expr(stmt->v.let_dim.value);
 break;
 case STMT_IF:
 ast_free_expr(stmt->v.if_stmt.condition);
 ast_free_stmt(stmt->v.if_stmt.then_stmt);
 break;
 case STMT_GOTO:
 ast_free_expr(stmt->v.goto_stmt.target);
 break;
 case STMT_GOSUB:
 ast_free_expr(stmt->v.gosub.target);
 break;
 case STMT_FOR:
 ast_free_expr(stmt->v.for_stmt.init);
 ast_free_expr(stmt->v.for_stmt.limit);
 ast_free_expr(stmt->v.for_stmt.step);
 break;
 case STMT_INPUT:
 ast_free_expr(stmt->v.input.prompt);
 break;
 case STMT_DIM:
 ast_free_expr(stmt->v.dim.dim1);
 ast_free_expr(stmt->v.dim.dim2);
 break;
 case STMT_WHILE:
 ast_free_expr(stmt->v.while_stmt.left);
 ast_free_expr(stmt->v.while_stmt.right);
 break;
 case STMT_ON_GOTO:
 ast_free_expr(stmt->v.on_goto.selector);
 break;
 case STMT_DEF_FN:
 ast_free_expr(stmt->v.def_fn.body);
 break;
 case STMT_READ:
 {
 int ri;
 for (ri = 0; ri < stmt->v.read.var_count; ri++) {
 if (stmt->v.read.var_indices[ri])
 ast_free_expr(stmt->v.read.var_indices[ri]);
 }
 }
 break;
 default:
 break;
 }
 free(stmt);
}

void ast_free_line(AstStmt *stmts)
{
 while (stmts) {
 AstStmt *next = stmts->next;
 ast_free_stmt(stmts);
 stmts = next;
 }
}

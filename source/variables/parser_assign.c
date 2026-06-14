/*
 * ---
 * BASIC++ Interpreter - parser_assign.c
 * ---
 *
 * Assignment and variable declaration handlers: LET, DIM.
 *
 * Handles explicit and implicit variable assignment, string
 * variable assignment, array element assignment, and DIM
 * array declarations.
 *
 * ---
 */

#include "parser_internal.h"

/*
 * parse_let - Parse and execute LET (or bare assignment).
 *
 * Syntax:
 * LET var = expr
 * var = expr (LET is optional in PATB)
 * LET @(expr) = expr
 * @(expr) = expr
 *
 * The 'has_let' parameter indicates whether the LET keyword was
 * already consumed by the caller.
 */
void pi_parse_let(Lexer *lex, RuntimeState *rt, int line_num,
 int has_let)
{
 /* Check for @() array assignment */
 if (lex->current.type == TOK_AT) {
 long index, value;
 lexer_next(lex); /* consume @ */
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 index = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;
 value = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_array(rt, index, value);
 return;
 }

 /* Standard variable assignment (or DIM array assignment) */
 if (lex->current.type == TOK_VARIABLE) {
 char var_name;
 var_name = lex->current.value.var_name;
 lexer_next(lex); /* consume variable */

 /* Check for DIM array assignment: A(i) = expr */
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays &&
 runtime_find_dim(rt, &var_name, 1) != NULL) {
 int idx1, idx2 = 0, idx3 = 0;
 BValue val;
 lexer_next(lex); /* consume ( */
 idx1 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx3 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;

 /*
  * ASC() array unpack: X(n) = ASC(string$)
  * For 1D arrays, unpack each character's ASCII
  * value into consecutive slots starting at idx1.
  * Truncates if string exceeds remaining slots.
  */
 if (idx2 == 0 && idx3 == 0 &&
 lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ASC) {
 BValue str_val;
 lexer_next(lex); /* consume ASC */
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 str_val = parse_expression_bval(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (bval_is_string(&str_val) &&
 str_val.v.sval.length > 0 &&
 str_val.v.sval.data != NULL) {
 DimArray *arr = runtime_find_dim(rt,
 &var_name, 1);
 if (arr != NULL) {
 int i, max_fill;
 int slen = str_val.v.sval.length;
 max_fill = arr->size[0] - idx1;
 if (max_fill > slen) max_fill = slen;
 if (max_fill < 0) max_fill = 0;
 for (i = 0; i < max_fill; i++) {
 BValue cv = bval_int((long)
 (unsigned char)
 str_val.v.sval.data[i]);
 runtime_set_dim(rt,
 &var_name, 1,
 idx1 + i, 0, 0,
 cv, line_num);
 if (error_occurred()) return;
 }
 }
 }
 return;
 }

 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_dim(rt, &var_name, 1, idx1, idx2, idx3, val, line_num);
 return;
 }

 if (!lexer_expect(lex, TOK_EQUALS)) return;

 {
 BValue bv;
 bv = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;

 /*
 * FUNCTION / multi-line DEF FN return value:
 * if inside a function, check if var_name
 * matches the function name (or FN suffix).
 */
 if (rt->in_sub_index >= 0 &&
 rt->in_sub_index < rt->sub_count &&
 rt->subs[rt->in_sub_index].is_function) {
 SubDef *cur =
 &rt->subs[rt->in_sub_index];
 char uc = (char)(var_name >= 'a' &&
 var_name <= 'z' ?
 var_name - 32 :
 var_name);
 /* Direct match: FUNCTION S -> S = v */
 if (cur->name_len == 1 &&
 cur->name[0] == uc) {
 rt->fn_return_value = bv;
 return;
 }
 /* DEF FN match: FNS -> S = v */
 if (cur->name_len == 3 &&
 cur->name[0] == 'F' &&
 cur->name[1] == 'N' &&
 cur->name[2] == uc) {
 rt->fn_return_value = bv;
 return;
 }
 }

 runtime_set_var_bval(rt, var_name, bv);
 }
 } else if (lex->current.type == TOK_NAMED_VAR) {
 /* Extended variable assignment */
 const char *name = lex->current.str_start;
 int name_len = lex->current.str_length;
 BValue bv;

 lexer_next(lex); /* consume named variable */

 /* Check for DIM array assignment: Arr(i) = expr */
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays &&
 runtime_find_dim(rt, name, name_len) != NULL) {
 int idx1, idx2 = 0, idx3 = 0;
 BValue dval;
 lexer_next(lex); /* consume ( */
 idx1 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx3 = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;

 /*
  * ASC() array unpack: Arr(n) = ASC(string$)
  * Same as single-letter path above.
  */
 if (idx2 == 0 && idx3 == 0 &&
 lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ASC) {
 BValue str_val;
 lexer_next(lex); /* consume ASC */
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 str_val = parse_expression_bval(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (bval_is_string(&str_val) &&
 str_val.v.sval.length > 0 &&
 str_val.v.sval.data != NULL) {
 DimArray *arr = runtime_find_dim(rt,
 name, name_len);
 if (arr != NULL) {
 int i, max_fill;
 int slen = str_val.v.sval.length;
 max_fill = arr->size[0] - idx1;
 if (max_fill > slen) max_fill = slen;
 if (max_fill < 0) max_fill = 0;
 for (i = 0; i < max_fill; i++) {
 BValue cv = bval_int((long)
 (unsigned char)
 str_val.v.sval.data[i]);
 runtime_set_dim(rt,
 name, name_len,
 idx1 + i, 0, 0,
 cv, line_num);
 if (error_occurred()) return;
 }
 }
 }
 return;
 }

 dval = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_dim(rt, name, name_len,
 idx1, idx2, idx3, dval, line_num);
 return;
 }

 if (!lexer_expect(lex, TOK_EQUALS)) return;

 bv = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;

 /*
 * FUNCTION return value: if we are inside a
 * FUNCTION and the LHS matches the function name,
 * set fn_return_value instead of a variable.
 */
 if (rt->in_sub_index >= 0 &&
 rt->in_sub_index < rt->sub_count &&
 rt->subs[rt->in_sub_index].is_function) {
 SubDef *cur = &rt->subs[rt->in_sub_index];
 int fn_match = 0;
 if (cur->name_len == name_len) {
 /* Case-insensitive compare */
 int j, match = 1;
 for (j = 0; j < name_len; j++) {
 char a = name[j];
 char b = cur->name[j];
 if (a >= 'a' && a <= 'z')
 a = (char)(a - 32);
 if (b >= 'a' && b <= 'z')
 b = (char)(b - 32);
 if (a != b) { match = 0; break; }
 }
 if (match) fn_match = 1;
 }
 /*
 * Multi-line DEF FN: function name is
 * "FN<x>", but LET uses just "<x>".
 * Match suffix after "FN" prefix.
 */
 if (!fn_match && cur->name_len > 2 &&
 cur->name[0] == 'F' &&
 cur->name[1] == 'N' &&
 name_len == cur->name_len - 2) {
 int j, match = 1;
 for (j = 0; j < name_len; j++) {
 char a = name[j];
 char b = cur->name[j + 2];
 if (a >= 'a' && a <= 'z')
 a = (char)(a - 32);
 if (b >= 'a' && b <= 'z')
 b = (char)(b - 32);
 if (a != b) { match = 0; break; }
 }
 if (match) fn_match = 1;
 }
 if (fn_match) {
 rt->fn_return_value = bv;
 return;
 }
 }

 runtime_set_named_var_bval(rt, name, name_len, bv);
 } else if (lex->current.type == TOK_STRING_VAR) {
 /* String variable or string array assignment */
 char var_name = lex->current.value.var_name;
 BValue val;
 lexer_next(lex); /* consume string variable */

 /*
 * Check for DIM string array assignment: A$(idx) = expr
 * The DIM name is "A$" (2 chars).
 */
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays) {
 char sname[3];
 sname[0] = var_name;
 sname[1] = '$';
 sname[2] = '\0';
 {
 DimArray *arr = runtime_find_dim(rt, sname, 2);
 if (arr != NULL) {
 int idx1, idx2 = 0, idx3 = 0;
 lexer_next(lex); /* consume ( */
 val = parse_expression_bval(lex, rt, line_num);
 idx1 = (int)bval_to_subscript(&val);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 val = parse_expression_bval(lex, rt,
 line_num);
 idx2 = (int)bval_to_subscript(&val);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 val = parse_expression_bval(lex, rt,
 line_num);
 idx3 = (int)bval_to_subscript(&val);
 if (error_occurred()) return;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_dim(rt, sname, 2, idx1, idx2, idx3,
 val, line_num);
 return;
 }
 }
 }

 if (!lexer_expect(lex, TOK_EQUALS)) return;
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_string_var(rt, var_name, val);
 } else {
 error_raise(ERR_WHAT, line_num);
 }

 (void)has_let; /* suppress unused parameter warning */
}

/* --- DIM Statement Handler ---
 * Syntax:
 * DIM name(size) - 1D array
 * DIM name(size1,size2) - 2D array
 */
void pi_parse_dim(Lexer *lex, RuntimeState *rt, int line_num)
{
 if (!dialect_get_config()->has_dim_arrays) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /*
 * DIM supports comma-separated declarations:
 * DIM A(10), B(20), RM$(255)
 *
 * Each declaration is: name[(subscripts)]
 * Name can be TOK_VARIABLE (A), TOK_NAMED_VAR (RM, RM$),
 * or TOK_STRING_VAR (A$).
 */
 do {
 char name[MAX_VAR_NAME_LEN + 1];
 int name_len;
 int dim1;
 int dim2 = 0;
 int dim3 = 0;

 /* Get array name */
 if (lex->current.type == TOK_VARIABLE) {
 name[0] = lex->current.value.var_name;
 name[1] = '\0';
 name_len = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_STRING_VAR) {
 /*
 * Single-letter string array: DIM A$(10)
 * Store as "A$" so it's distinct from numeric A().
 */
 name[0] = lex->current.value.var_name;
 name[1] = '$';
 name[2] = '\0';
 name_len = 2;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR) {
 /*
 * Multi-char variable or string var: DIM RM(10), RM$(255)
 * The lexer already includes trailing $ in the name
 * for string vars (e.g., str_start="RM$", str_length=3).
 */
 name_len = lex->current.str_length;
 if (name_len > MAX_VAR_NAME_LEN)
 name_len = MAX_VAR_NAME_LEN;
 memcpy(name, lex->current.str_start, (size_t)name_len);
 name[name_len] = '\0';
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 if (!lexer_expect(lex, TOK_LPAREN)) return;

 dim1 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume comma */
 dim2 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume comma */
 dim3 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 }
 }

 if (!lexer_expect(lex, TOK_RPAREN)) return;

 runtime_dim(rt, name, name_len, dim1, dim2, dim3, line_num);
 if (error_occurred()) return;

 /* Check for comma -> more arrays to DIM */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume comma, loop */
 } else {
 break;
 }
 } while (1);
}


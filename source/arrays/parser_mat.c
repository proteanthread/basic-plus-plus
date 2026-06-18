 // ---
 // BASIC++ Interpreter - parser_mat.c
 // ---
 //
 // Matrix operation handler: MAT statement.
 //
 // Implements ECMA-116 matrix operations: MAT READ, MAT PRINT,
 // MAT assignment, MAT ZER, MAT CON, MAT IDN, MAT TRN,
 // MAT INV, and matrix arithmetic (+, -, *).
 //
//
// HOW TO EXTEND:
//   To add a new statement or sub-command:
//   1. Add the keyword to lexer.h (KeywordId enum).
//   2. Add it to the keyword table in lexer.c.
//   3. Add a handler function in this file.
//   4. Wire it into parser.c's dispatch switch.
//
// TROUBLESHOOTING:
//   - 'WHAT?' on valid syntax: check dialect feature flags.
//   - Crash in expression: ensure error_occurred() is checked
//     after every parse_expression call.
 // ---

#include "parser_internal.h"

int pi_mat_get_array_name(Lexer *lex, char *name, int *name_len)
{
 if (lex->current.type == TOK_VARIABLE) {
 name[0] = lex->current.value.var_name;
 name[1] = '\0';
 *name_len = 1;
 lexer_next(lex);
 return 1;
 }
 if (lex->current.type == TOK_NAMED_VAR) {
 int len = lex->current.str_length;
 if (len > MAX_VAR_NAME_LEN) len = MAX_VAR_NAME_LEN;
 memcpy(name, lex->current.str_start, (size_t)len);
 name[len] = '\0';
 *name_len = len;
 lexer_next(lex);
 return 1;
 }
 return 0;
}

 // mat_match_ident - Check if current token matches a specific
 // identifier string (case-insensitive). Used for ZER, CON, IDN,
 // TRN, INV which are not keywords but context-specific identifiers.
int pi_mat_match_ident(Lexer *lex, const char *target)
{
 const char *src;
 int len, i;

 if (lex->current.type == TOK_NAMED_VAR) {
 src = lex->current.str_start;
 len = lex->current.str_length;
 } else if (lex->current.type == TOK_VARIABLE) {
 // Single-letter - only matches single-letter targets
 return 0;
 } else {
 return 0;
 }

 i = 0;
 while (target[i] != '\0') i++;
 if (len != i) return 0;

 for (i = 0; i < len; i++) {
 char a = src[i];
 char b = target[i];
 if (a >= 'a' && a <= 'z') a = (char)(a - 32);
 if (b >= 'a' && b <= 'z') b = (char)(b - 32);
 if (a != b) return 0;
 }
 return 1;
}

 // parse_mat_cmd - Parse and execute a MAT statement.
 //
 // This is the main dispatcher for all MAT operations.
 // The MAT keyword has already been consumed by the statement
 // dispatcher.
void pi_parse_mat_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char name_a[MAX_VAR_NAME_LEN + 1];
 int name_a_len;

 // MAT requires a dialect with DIM array support
 if (!dialect_get_config()->has_dim_arrays) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 // MAT PRINT <name> [;|,] or MAT PRINT USING fmt$; <name>
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_PRINT) {
 DimArray *arr;
 int r, c;
 int use_tab = 1; // default: tabbed
 int use_compact = 0;
 char fmt_buf[64];
 int has_using = 0;

 lexer_next(lex); // consume PRINT

 // Check for USING
 if (lex->current.type == TOK_KEYWORD &&
  lex->current.value.keyword == KW_USING) {
  BValue fv;
  lexer_next(lex); // consume USING
  fv = parse_expression_bval(lex, rt, line_num);
  if (error_occurred()) return;
  {
   int fl = fv.v.sval.length;
   if (fl > 63) fl = 63;
   memcpy(fmt_buf, fv.v.sval.data,
    (size_t)fl);
   fmt_buf[fl] = '\0';
  }
  has_using = 1;
  // Expect semicolon separator
  if (lex->current.type == TOK_SEMICOLON)
   lexer_next(lex);
 }

 if (!pi_mat_get_array_name(lex, name_a, &name_a_len)) {
  error_raise(ERR_WHAT, line_num);
  return;
 }

 // Check trailing delimiter: ; = compact, , = tabbed
 if (lex->current.type == TOK_SEMICOLON) {
  use_compact = 1;
  use_tab = 0;
  lexer_next(lex);
 } else if (lex->current.type == TOK_COMMA) {
  use_tab = 1;
  use_compact = 0;
  lexer_next(lex);
 }

 arr = runtime_find_dim(rt, name_a, name_a_len);
 if (arr == NULL) {
  error_raise(ERR_HOW, line_num);
  return;
 }

 if (arr->dims == 1) {
  // 1D: print elements on one line
  int base = rt->option_base;
  for (c = base; c < arr->size[0]; c++) {
   BValue v = arr->elements[c - base];
   if (has_using) {
    printf(fmt_buf, bval_to_float(&v));
   } else if (v.type == VAL_FLOAT) {
    printf("%G", v.v.fval);
   } else {
    printf("%ld", bval_to_int(&v));
   }
   if (c < arr->size[0] - 1) {
    if (use_compact)
     printf(" ");
    else
     printf("\t");
   }
  }
  printf("\n");
 } else {
  // 2D: print as matrix rows
  for (r = 1; r < arr->size[0]; r++) {
   for (c = 1; c < arr->size[1]; c++) {
    int off = r * arr->size[1] + c;
    BValue v = arr->elements[off];
    if (has_using) {
     printf(fmt_buf, bval_to_float(&v));
    } else if (v.type == VAL_FLOAT) {
     printf("%G", v.v.fval);
    } else {
     printf("%6ld", bval_to_int(&v));
    }
    if (c < arr->size[1] - 1) {
     if (use_compact)
      printf(" ");
     else
      printf("\t");
    }
   }
   printf("\n");
  }
 }
 return;
 }

 // MAT READ <name>
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_READ) {
 DimArray *arr;
 int r, c;

 lexer_next(lex); // consume READ
 if (!pi_mat_get_array_name(lex, name_a, &name_a_len)) {
  error_raise(ERR_WHAT, line_num);
  return;
 }

 arr = runtime_find_dim(rt, name_a, name_a_len);
 if (arr == NULL) {
  error_raise(ERR_HOW, line_num);
  return;
 }

 if (arr->dims == 1) {
  // 1D: read into elements
  for (c = 0; c < arr->total; c++) {
   arr->elements[c] = runtime_read_data_bval(
    rt, line_num);
   if (error_occurred()) return;
  }
 } else {
  // 2D: read row-major, 1-based
  for (r = 1; r < arr->size[0]; r++) {
   for (c = 1; c < arr->size[1]; c++) {
    int off = r * arr->size[1] + c;
    arr->elements[off] = runtime_read_data_bval(
     rt, line_num);
    if (error_occurred()) return;
   }
  }
 }
 return;
 }

 // MAT INPUT <name>
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_INPUT) {
 DimArray *arr;
 int r, c;
 char ibuf[256];

 lexer_next(lex); // consume INPUT
 if (!pi_mat_get_array_name(lex, name_a, &name_a_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 arr = runtime_find_dim(rt, name_a, name_a_len);
 if (arr == NULL || arr->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 // Input values into matrix, 1-based
 for (r = 1; r < arr->size[0]; r++) {
 for (c = 1; c < arr->size[1]; c++) {
 int off = r * arr->size[1] + c;
 double val;
 printf("(%d,%d)? ", r, c);
 fflush(stdout);
 if (fgets(ibuf, sizeof(ibuf), stdin) == NULL) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 val = atof(ibuf);
 arr->elements[off] = bval_float(val);
 }
 }
 return;
 }

 if (!pi_mat_get_array_name(lex, name_a, &name_a_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 if (lex->current.type != TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume '='

 // --- MAT A = ZER / CON / IDN ---
 if (pi_mat_match_ident(lex, "ZER")) {
 DimArray *arr = runtime_find_dim(rt, name_a, name_a_len);
 int i;
 if (arr == NULL) {
  error_raise(ERR_HOW, line_num);
  return;
 }
 // Works on 1D, 2D, and 3D arrays
 for (i = 0; i < arr->total; i++) {
  arr->elements[i] = bval_int(0);
 }
 lexer_next(lex);
 return;
 }

 if (pi_mat_match_ident(lex, "CON")) {
 DimArray *arr = runtime_find_dim(rt, name_a, name_a_len);
 int i;
 if (arr == NULL) {
  error_raise(ERR_HOW, line_num);
  return;
 }
 // Works on 1D, 2D, and 3D arrays
 for (i = 0; i < arr->total; i++) {
  arr->elements[i] = bval_int(1);
 }
 lexer_next(lex);
 return;
 }

 if (pi_mat_match_ident(lex, "IDN")) {
 DimArray *arr = runtime_find_dim(rt, name_a, name_a_len);
 int r, c;
 if (arr == NULL || arr->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 // rows and cols must match for identity
 if (arr->size[0] != arr->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 for (r = 0; r < arr->size[0]; r++) {
 for (c = 0; c < arr->size[1]; c++) {
 arr->elements[r * arr->size[1] + c] =
 (r == c) ? bval_float(1.0) : bval_float(0.0);
 }
 }
 lexer_next(lex);
 return;
 }

 // --- MAT A = TRN(B) ---
 if (pi_mat_match_ident(lex, "TRN")) {
 char name_b[MAX_VAR_NAME_LEN + 1];
 int name_b_len;
 DimArray *a, *b;
 int r, c;

 lexer_next(lex); // consume TRN
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 if (!pi_mat_get_array_name(lex, name_b, &name_b_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;

 b = runtime_find_dim(rt, name_b, name_b_len);
 if (b == NULL || b->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 // A must be DIMmed with transposed dimensions
 a = runtime_find_dim(rt, name_a, name_a_len);
 if (a == NULL || a->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 if (a->size[0] != b->size[1] || a->size[1] != b->size[0]) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 // A(r,c) = B(c,r) - using 1-based indexing
 for (r = 1; r < a->size[0]; r++) {
 for (c = 1; c < a->size[1]; c++) {
 int a_off = r * a->size[1] + c;
 int b_off = c * b->size[1] + r;
 a->elements[a_off] = b->elements[b_off];
 }
 }
 return;
 }

 // --- MAT A = INV(B) ---
 if (pi_mat_match_ident(lex, "INV")) {
 char name_b[MAX_VAR_NAME_LEN + 1];
 int name_b_len;
 DimArray *a, *b;
 int n, r, c, p;
 double work[16][32]; // max 15x15 matrix for inverse
 double pivot, factor;

 lexer_next(lex); // consume INV
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 if (!pi_mat_get_array_name(lex, name_b, &name_b_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;

 b = runtime_find_dim(rt, name_b, name_b_len);
 if (b == NULL || b->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 // Must be square
 if (b->size[0] != b->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 n = b->size[0] - 1; // 1-based size
 if (n > 15 || n < 1) {
 error_raise(ERR_SORRY, line_num); // too large
 return;
 }

 a = runtime_find_dim(rt, name_a, name_a_len);
 if (a == NULL || a->dims != 2 ||
 a->size[0] != b->size[0] ||
 a->size[1] != b->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 // Build augmented matrix [B | I]
 for (r = 0; r < n; r++) {
 for (c = 0; c < n; c++) {
 BValue v = b->elements[(r+1) * b->size[1] + (c+1)];
 work[r][c] = bval_to_float(&v);
 work[r][c + n] = (r == c) ? 1.0 : 0.0;
 }
 }

 // Gauss-Jordan elimination
 for (p = 0; p < n; p++) {
 // Find pivot
 int max_row = p;
 double max_val = work[p][p];
 if (max_val < 0) max_val = -max_val;

 for (r = p + 1; r < n; r++) {
 double v = work[r][p];
 if (v < 0) v = -v;
 if (v > max_val) {
 max_val = v;
 max_row = r;
 }
 }

 // Swap rows if needed
 if (max_row != p) {
 for (c = 0; c < 2 * n; c++) {
 double tmp = work[p][c];
 work[p][c] = work[max_row][c];
 work[max_row][c] = tmp;
 }
 }

 pivot = work[p][p];
 if (pivot > -1e-12 && pivot < 1e-12) {
 // Singular matrix
 error_raise(ERR_HOW, line_num);
 return;
 }

 // Scale pivot row
 for (c = 0; c < 2 * n; c++) {
 work[p][c] /= pivot;
 }

 // Eliminate column
 for (r = 0; r < n; r++) {
 if (r == p) continue;
 factor = work[r][p];
 for (c = 0; c < 2 * n; c++) {
 work[r][c] -= factor * work[p][c];
 }
 }
 }

 // Extract inverse from right half
 for (r = 0; r < n; r++) {
 for (c = 0; c < n; c++) {
 a->elements[(r+1) * a->size[1] + (c+1)] =
 bval_float(work[r][c + n]);
 }
 }
 return;
 }

 // --- MAT A = (k) * B - scalar multiplication ---
 if (lex->current.type == TOK_LPAREN) {
 BValue scalar;
 double k;
 char name_b[MAX_VAR_NAME_LEN + 1];
 int name_b_len;
 DimArray *a, *b;
 int r, c;

 lexer_next(lex); // consume '('
 scalar = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 k = bval_to_float(&scalar);
 if (!lexer_expect(lex, TOK_RPAREN)) return;

 // Expect '*'
 if (lex->current.type != TOK_STAR) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume '*'

 if (!pi_mat_get_array_name(lex, name_b, &name_b_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 b = runtime_find_dim(rt, name_b, name_b_len);
 if (b == NULL || b->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 a = runtime_find_dim(rt, name_a, name_a_len);
 if (a == NULL || a->dims != 2 ||
 a->size[0] != b->size[0] ||
 a->size[1] != b->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 for (r = 1; r < a->size[0]; r++) {
 for (c = 1; c < a->size[1]; c++) {
 int off = r * a->size[1] + c;
 BValue v = b->elements[off];
 a->elements[off] = bval_float(
 bval_to_float(&v) * k);
 }
 }
 return;
 }

 // --- MAT A = B, MAT A = B + C, MAT A = B - C, MAT A = B * C ---
 {
 char name_b[MAX_VAR_NAME_LEN + 1];
 int name_b_len;
 DimArray *a, *b;

 if (!pi_mat_get_array_name(lex, name_b, &name_b_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 b = runtime_find_dim(rt, name_b, name_b_len);
 if (b == NULL || b->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 a = runtime_find_dim(rt, name_a, name_a_len);
 if (a == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 // Check for operator: +, -, *
 if (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 // MAT A = B + C or MAT A = B - C
 TokenType op = lex->current.type;
 char name_c[MAX_VAR_NAME_LEN + 1];
 int name_c_len;
 DimArray *cc;
 int r, c_idx;

 lexer_next(lex); // consume +/-
 if (!pi_mat_get_array_name(lex, name_c, &name_c_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 cc = runtime_find_dim(rt, name_c, name_c_len);
 if (cc == NULL || cc->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 // Dimensions must match
 if (b->size[0] != cc->size[0] ||
 b->size[1] != cc->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 if (a->size[0] != b->size[0] ||
 a->size[1] != b->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 for (r = 1; r < a->size[0]; r++) {
 for (c_idx = 1; c_idx < a->size[1]; c_idx++) {
 int off = r * a->size[1] + c_idx;
 double bv = bval_to_float(
 &b->elements[off]);
 double cv = bval_to_float(
 &cc->elements[off]);
 if (op == TOK_PLUS) {
 a->elements[off] =
 bval_float(bv + cv);
 } else {
 a->elements[off] =
 bval_float(bv - cv);
 }
 }
 }
 return;
 }

 if (lex->current.type == TOK_STAR) {
 // MAT A = B * C
 char name_c[MAX_VAR_NAME_LEN + 1];
 int name_c_len;
 DimArray *cc;
 int r, c_idx, k;
 static BValue temp_b[4096];
 static BValue temp_c[4096];
 BValue *bp, *cp;

 lexer_next(lex); // consume '*'
 if (!pi_mat_get_array_name(lex, name_c, &name_c_len)) {
  error_raise(ERR_WHAT, line_num);
  return;
 }

 cc = runtime_find_dim(rt, name_c, name_c_len);
 if (cc == NULL || cc->dims != 2) {
  error_raise(ERR_HOW, line_num);
  return;
 }

 // B cols must match C rows
 if (b->size[1] != cc->size[0]) {
  error_raise(ERR_HOW, line_num);
  return;
 }
 // A must be B rows x C cols
 if (a->size[0] != b->size[0] ||
     a->size[1] != cc->size[1]) {
  error_raise(ERR_HOW, line_num);
  return;
 }

 // In-place safety: copy source if it overlaps target
 bp = b->elements;
 cp = cc->elements;
 if (a == b) {
  memcpy(temp_b, b->elements,
   (size_t)b->total * sizeof(BValue));
  bp = temp_b;
 }
 if (a == cc) {
  memcpy(temp_c, cc->elements,
   (size_t)cc->total * sizeof(BValue));
  cp = temp_c;
 }

 // Matrix multiply: A(r,c) = sum B(r,k)*C(k,c)
 for (r = 1; r < a->size[0]; r++) {
  for (c_idx = 1; c_idx < a->size[1]; c_idx++) {
   double sum = 0.0;
   for (k = 1; k < b->size[1]; k++) {
    double bv = bval_to_float(
     &bp[r * b->size[1] + k]);
    double cv = bval_to_float(
     &cp[k * cc->size[1] + c_idx]);
    sum += bv * cv;
   }
   a->elements[r * a->size[1] + c_idx] =
    bval_float(sum);
  }
 }
 return;
 }

 // MAT A = B (copy)
 if (a->dims != 2 ||
 a->size[0] != b->size[0] ||
 a->size[1] != b->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 {
 int i;
 for (i = 0; i < a->total; i++) {
 a->elements[i] = b->elements[i];
 }
 }
 }
}

// --- File I/O - OPEN / CLOSE ---
 //
 // SYNTAX:
 // OPEN "filename" FOR INPUT AS #n
 // OPEN "filename" FOR OUTPUT AS #n
 // OPEN "filename" FOR APPEND AS #n
 // CLOSE #n
 //
 // The FOR keyword is detected as TOK_KEYWORD KW_FOR (reused from
 // loops). INPUT/OUTPUT/APPEND are detected as identifiers.



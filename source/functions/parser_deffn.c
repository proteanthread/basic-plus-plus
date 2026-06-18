/*
 * ---
 * BASIC++ Interpreter - parser_deffn.c
 * ---
 *
 * User-defined function handlers: DEF FN, FN evaluation.
 *
 * Supports single-line DEF FN (PATB/ECMA-55), multi-line
 * DEF FN (ECMA-116), and evaluation of user functions.
 *
 * ---
 */

#include "parser_internal.h"

/*
 * parse_def_fn - Parse a DEF FN statement.
 *
 * DEF FN<name>(<params>) = <expression>
 *
 * Stores the function definition in the runtime's user_funcs table.
 */
void pi_parse_def_fn(Lexer *lex, RuntimeState *rt, int line_num)
{
 char fn_name[MAX_VAR_NAME_LEN + 1];
 int fn_name_len = 0;
 char params[MAX_FN_PARAMS];
 int param_count = 0;
 const char *body_start;
 int body_len;

 /*
 * DEF SEG [= address]
 * Set virtual memory segment base.
 */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_SEG) {
 lexer_next(lex); /* consume SEG */
 if (lex->current.type == TOK_EQUALS) {
 lexer_next(lex); /* consume = */
 rt->mem_seg_base = (int)parse_expression(
 lex, rt, line_num);
 if (rt->mem_seg_base < 0)
 rt->mem_seg_base = 0;
 if (rt->mem_seg_base >= MAX_MEM_SEGMENT)
 rt->mem_seg_base = 0;
 } else {
 rt->mem_seg_base = 0; /* DEF SEG alone resets */
 }
 return;
 }

 /*
 * DEF USR [= address]
 * Define user machine-language function address.
 * No-op: BASIC++ cannot call machine language.
 * Accept and consume for GW-BASIC compatibility.
 */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_DEFUSR) {
  lexer_next(lex);
  lexer_skip_to_end(lex);
  return;
 }
 /*
 * Also handle plain identifier USR after DEF.
 * In extended-vars mode, USR becomes TOK_NAMED_VAR.
 * In single-letter mode, USR becomes TOK_VARIABLE U.
 */
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 3 &&
 lex->current.str_start != NULL &&
 (lex->current.str_start[0] == 'U' ||
 lex->current.str_start[0] == 'u')) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 if (lex->current.type == TOK_VARIABLE &&
 (lex->current.value.var_name == 'U' ||
 lex->current.value.var_name == 'u')) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /*
 * After DEF, expect FN keyword or FN<name> as
 * a single named var (extended-vars mode).
 *
 * In extended-vars mode (GW-BASIC, QBasic, E116),
 * the lexer tokenizes "FNA" as TOK_NAMED_VAR
 * instead of KW_FN + TOK_VARIABLE. Handle both.
 */
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_start != NULL &&
 lex->current.str_length >= 3 &&
 (lex->current.str_start[0] == 'F' ||
 lex->current.str_start[0] == 'f') &&
 (lex->current.str_start[1] == 'N' ||
 lex->current.str_start[1] == 'n')) {
 /* Extract name after "FN" prefix */
 int nlen = lex->current.str_length - 2;
 int k;
 if (nlen > MAX_VAR_NAME_LEN)
 nlen = MAX_VAR_NAME_LEN;
 for (k = 0; k < nlen; k++) {
 char c = lex->current.str_start[k + 2];
 if (c >= 'a' && c <= 'z')
 c = (char)(c - 32);
 fn_name[k] = c;
 }
 fn_name[nlen] = '\0';
 fn_name_len = nlen;
 lexer_next(lex); /* consume FN<name> */
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_FN) {
 lexer_next(lex); /* consume FN */

 /*
 * Parse function name - letter(s) after FN.
 * Classic: DEF FN A(X) -> name = "A"
 */
 if (lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 fn_name[0] = lex->current.value.var_name;
 fn_name_len = 1;
 fn_name[fn_name_len] = '\0';
 lexer_next(lex); /* consume variable name */
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /* Parse parameter list: (X) or (X,Y) or (X,Y,Z) */
 if (!lexer_expect(lex, TOK_LPAREN)) return;

 if (lex->current.type == TOK_VARIABLE) {
 params[param_count++] = lex->current.value.var_name;
 lexer_next(lex);

 while (lex->current.type == TOK_COMMA &&
 param_count < MAX_FN_PARAMS) {
 lexer_next(lex); /* consume comma */
 if (lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 params[param_count++] = lex->current.value.var_name;
 lexer_next(lex);
 }
 }

 if (!lexer_expect(lex, TOK_RPAREN)) return;

 /*
 * If no '=', this is a multi-line DEF FN (ECMA-116):
 *
 * DEF FN Square(X)
 * LET FN Square = X * X
 * END DEF
 *
 * Register as a FUNCTION in the subs table (reusing
 * the QBasic FUNCTION infrastructure) and skip to
 * END DEF. The return value is set via assignment
 * to FN Name inside the body.
 */
 if (lex->current.type != TOK_EQUALS) {
 SubDef *sd;
 char full_name[MAX_VAR_NAME_LEN + 3];
 int full_len;
 int idx;
 ProgramStore *pgm = rt->program;

 /* Build "FN" + name for the subs table */
 full_name[0] = 'F';
 full_name[1] = 'N';
 full_len = 2 + fn_name_len;
 if (full_len > MAX_VAR_NAME_LEN)
 full_len = MAX_VAR_NAME_LEN;
 memcpy(full_name + 2, fn_name,
 (size_t)(full_len - 2));
 full_name[full_len] = '\0';

 /* Check if already registered */
 if (runtime_find_sub(rt, full_name, full_len)
 != NULL) {
 /* Re-run: skip to END DEF */
 } else {
 if (rt->sub_count >= MAX_SUBS) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 sd = &rt->subs[rt->sub_count];
 memcpy(sd->name, full_name,
 (size_t)(full_len + 1));
 sd->name_len = full_len;
 sd->is_function = 1;
 sd->body_index = rt->current_index + 1;
 sd->param_count = 0;

 /* Map DEF FN params (single-letter A-Z) to
 * SUB-style named params */
 {
 int pi;
 for (pi = 0; pi < param_count &&
 pi < MAX_SUB_PARAMS; pi++) {
 sd->params[pi][0] = params[pi];
 sd->params[pi][1] = '\0';
 sd->param_is_string[pi] = 0;
 sd->param_count++;
 }
 }

 rt->sub_count++;
 }

 /* Skip forward to END DEF */
 idx = rt->current_index + 1;
 while (idx < pgm->count) {
 Lexer cl;
 lexer_init(&cl, pgm->lines[idx].text);
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD &&
 cl.current.value.keyword == KW_END) {
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD &&
 cl.current.value.keyword == KW_DEF) {
 rt->next_index = idx + 1;
 return;
 }
 }
 idx++;
 }
 error_raise(ERR_HOW, line_num);
 return;
 }

 /*
 * Single-line DEF FN: DEF FNA(X) = X*X+1
 * Capture the body expression text after '='.
 */
 {
 int eq_pos = lex->pos; /* pos is right after the '=' */
 /* Skip whitespace after '=' */
 while (eq_pos < lex->length &&
 lex->source[eq_pos] == ' ') {
 eq_pos++;
 }
 body_start = lex->source + eq_pos;
 body_len = lex->length - eq_pos;
 }

 /* Trim trailing whitespace */
 while (body_len > 0 &&
 (body_start[body_len - 1] == ' ' ||
 body_start[body_len - 1] == '\r' ||
 body_start[body_len - 1] == '\n')) {
 body_len--;
 }

 if (body_len <= 0) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /* Store the function definition */
 if (runtime_def_fn(rt, fn_name, fn_name_len,
 params, param_count,
 body_start, body_len) != 0) {
 error_raise(ERR_SORRY, line_num);
 return;
 }

 /* Skip to end of line (body consumed) */
 lex->current.type = TOK_EOF;
}

/*
 * eval_user_fn - Evaluate a user-defined function call.
 *
 * This is called from parse_factor_bval when FN<name>(...) is
 * encountered. It:
 * 1. Looks up the function definition
 * 2. Evaluates argument expressions
 * 3. Saves parameter variables
 * 4. Binds parameters to argument values
 * 5. Re-parses and evaluates the body expression
 * 6. Restores parameter variables
 * 7. Returns the result
 *
 * LOCAL SCOPING:
 * This save/bind/eval/restore pattern provides transparent
 * local scoping that matches classic BASIC behavior. The
 * parameter variables are temporarily overwritten and then
 * restored, so they appear local to the function body.
 */
BValue pi_eval_user_fn(Lexer *outer_lex, RuntimeState *rt,
 int line_num)
{
 char fn_name;
 UserFunction *fn;
 BValue saved_vars[MAX_FN_PARAMS];
 BValue args[MAX_FN_PARAMS];
 int argc = 0;
 int i;
 Lexer body_lex;
 BValue result;

 /* Parse function name (letter after FN) */
 if (outer_lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 fn_name = outer_lex->current.value.var_name;
 lexer_next(outer_lex); /* consume name */

 /* Look up the function */
 {
 char name_buf[2];
 name_buf[0] = fn_name;
 name_buf[1] = '\0';
 fn = runtime_find_fn(rt, name_buf, 1);
 }

 /*
 * If no single-line DEF FN found, check for
 * multi-line DEF FN in the subs table (registered
 * as "FN<name>").
 */
 if (fn == NULL) {
 char sub_name[4];
 SubDef *sd;
 sub_name[0] = 'F';
 sub_name[1] = 'N';
 sub_name[2] = (char)(fn_name >= 'a' &&
 fn_name <= 'z' ?
 fn_name - 32 : fn_name);
 sub_name[3] = '\0';
 sd = runtime_find_sub(rt, sub_name, 3);
 if (sd != NULL && sd->is_function &&
 sd->body_index >= 0) {
 /* Parse arguments */
 StackFrame frame;
 int save_idx, save_next;

 if (!lexer_expect(outer_lex, TOK_LPAREN))
 return bval_int(0);
 for (i = 0; i < sd->param_count; i++) {
 BValue av;
 if (i > 0) {
 if (outer_lex->current.type !=
 TOK_COMMA) break;
 lexer_next(outer_lex);
 }
 av = parse_expression_bval(
 outer_lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 args[i] = av;
 argc++;
 }
 if (!lexer_expect(outer_lex, TOK_RPAREN))
 return bval_int(0);

 /* Push FRAME_SUB */
 frame.type = FRAME_SUB;
 frame.data.sub_call.return_index =
 rt->current_index;
 frame.data.sub_call.sub_index =
 (int)(sd - rt->subs);
 for (i = 0; i < MAX_VARIABLES; i++)
 frame.data.sub_call.saved_vars[i] =
 rt->variables[i];
 for (i = 0; i < MAX_STRING_VARS; i++)
 frame.data.sub_call.saved_strvars[i] =
 rt->string_vars[i];
 if (runtime_push(rt, &frame) != 0)
 return bval_int(0);

 /* Push scope stack */
 scope_stack_push(
  &rt->scope_stack, rt,
  SCOPE_FULL,
  (int)(sd - rt->subs),
  rt->current_index);

 /* Bind params */
 for (i = 0; i < argc && i < sd->param_count;
 i++) {
 pi_set_param_by_name(rt, sd->params[i],
 args[i]);
 }

 /* Execute inline */
 rt->fn_return_value = bval_int(0);
 rt->in_sub_index = (int)(sd - rt->subs);
 save_idx = rt->current_index;
 save_next = rt->next_index;

 {
 int fi = sd->body_index;
 ProgramStore *pgm = rt->program;
 while (fi < pgm->count &&
 !error_occurred()) {
 Lexer fl;
 int fln;
 ProgramLine *fline =
 &pgm->lines[fi];
 fln = fline->line_number;
 lexer_init(&fl, fline->text);
 if (fl.current.type == TOK_NUMBER)
 lexer_next(&fl);

 rt->current_index = fi;
 rt->next_index = -1;

 parser_execute_line(
 &fl, rt, fln);

 if (error_occurred())
 return bval_int(0);

 if (rt->in_sub_index < 0)
 break;

 if (rt->next_index >= 0)
 fi = rt->next_index;
 else
 fi++;
 }
 }

 rt->current_index = save_idx;
 rt->next_index = save_next;
 return rt->fn_return_value;
 }
 }

 if (fn == NULL) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }

 /* Parse argument list */
 if (!lexer_expect(outer_lex, TOK_LPAREN)) return bval_int(0);

 if (fn->param_count > 0) {
 args[argc] = parse_expression_bval(outer_lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 argc++;

 while (argc < fn->param_count &&
 outer_lex->current.type == TOK_COMMA) {
 lexer_next(outer_lex); /* consume comma */
 args[argc] = parse_expression_bval(outer_lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 argc++;
 }
 }

 if (!lexer_expect(outer_lex, TOK_RPAREN)) return bval_int(0);

 /* Validate argument count */
 if (argc != fn->param_count) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }

 /* Save parameter variables (local scoping) */
 for (i = 0; i < fn->param_count; i++) {
 int var_idx = fn->params[i] - 'A';
 saved_vars[i] = rt->variables[var_idx];
 rt->variables[var_idx] = args[i];
 }

 /* Parse and evaluate the body expression */
 lexer_init(&body_lex, fn->body);
 result = parse_expression_bval(&body_lex, rt, line_num);

 /* Restore parameter variables */
 for (i = 0; i < fn->param_count; i++) {
 int var_idx = fn->params[i] - 'A';
 rt->variables[var_idx] = saved_vars[i];
 }

 return result;
}

/* --- MAT - Matrix Operations ---
 *
 * SYNTAX (Dartmouth BASIC compatible):
 * MAT PRINT A - print matrix formatted
 * MAT READ A - read matrix from DATA
 * MAT A = ZER - zero matrix
 * MAT A = CON - all ones matrix
 * MAT A = IDN - identity matrix
 * MAT A = TRN(B) - transpose of B into A
 * MAT A = INV(B) - inverse of B into A
 * MAT A = B + C - matrix addition
 * MAT A = B - C - matrix subtraction
 * MAT A = B * C - matrix multiplication
 * MAT A = (k) * B - scalar multiplication
 *
 * PREREQUISITES:
 * Matrices must be pre-DIMmed as 2D arrays.
 * DIM A(3,3) creates a 4x4 matrix (0..3 x 0..3).
 * MAT operations use 1-based indexing internally
 * (rows 1..size[0]-1, cols 1..size[1]-1), matching
 * Dartmouth BASIC convention.
 *
 * DESIGN:
 * All operations are parsed in parse_mat_cmd and dispatched
 * to inline code. No separate module needed - MAT operations
 * are statement-level and don't need expression integration.
 */

/*
 * mat_get_array_name - Parse a matrix name from the lexer.
 * Fills name[] and *name_len. Returns 1 on success, 0 on error.
 */


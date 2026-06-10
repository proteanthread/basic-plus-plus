/*
 * ---
 * BASIC++ Interpreter - parser.c
 * ---
 *
 * Statement and expression parser with direct execution dispatch.
 *
 * ARCHITECTURE:
 * This parser uses direct dispatch: parsing and execution happen
 * simultaneously. When the parser recognizes a PRINT statement,
 * it immediately evaluates the expression list and prints the
 * output. No intermediate AST is constructed.
 *
 * This is faithful to the original Palo Alto Tiny BASIC
 * architecture. The parsing layers are:
 *
 * parser_execute_line() -> handles ; separator, dispatches stmts
 * parse_statement() -> identifies keyword, calls handler
 * parse_print() -> PRINT expr-list
 * parse_let() -> LET var = expr (or bare assignment)
 * parse_input() -> INPUT var-list
 * parse_if() -> IF expr relop expr stmt
 * parse_goto() -> GOTO expr
 * parse_gosub() -> GOSUB expr
 * parse_return() -> RETURN
 * parse_end() -> END
 * parse_rem() -> REM (skip to EOL)
 * parse_list_cmd() -> LIST [-n | n | n- | n-m | n,n,n-m,...]
 * parse_run_cmd() -> RUN
 * parse_new_cmd() -> NEW
 * parse_save_cmd() -> SAVE "filename"
 * parse_load_cmd() -> LOAD "filename"
 * parse_stop() -> STOP
 *
 * parse_expression() -> [+|-] term ((+|-) term)*
 * parse_term() -> factor ((*|/) factor)*
 * parse_factor() -> num | var | @(expr) | (expr) |
 * ABS(expr) | RND(expr) | SIZE
 *
 * PATB-SPECIFIC BEHAVIOR:
 * - IF does NOT use THEN: "IF A=5 PRINT A"
 * - Semicolon is the statement separator: "10 A=1; PRINT A"
 * - GOTO/GOSUB/RETURN/STOP must be last on a line
 * - LET is optional: "A=5" is the same as "LET A=5"
 * - # is the not-equal operator and PRINT format specifier
 * - Relational operators return 1 (true) or 0 (false)
 * - Relational operators can appear in expressions
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#ifdef _WIN32
#include <direct.h> /* _chdir */
#else
#include <unistd.h> /* chdir */
#endif
#include "parser.h"
#include "exec.h"
#include "dialect.h"
#include "fileio.h"
#include "errors.h"
#include "compiler.h"
#include "vdev.h"
#include "funcreg.h"
#include "builtins.h"
#include "vm.h"
#include "bytecode.h"
#include "module.h"
#include "security.h"
#include "platform.h"
#include "selftest.h"
#include "help.h"
#include "detok.h"
#include "gfxbuf.h"
#include "memmap.h"

/* --- Forward Declarations ---
 * Internal parsing functions. These are not exposed in the header
 * because they are implementation details of the parse-and-execute
 * architecture.
 */
static void parse_statement(Lexer *lex, RuntimeState *rt, int line_num);
static long parse_term(Lexer *lex, RuntimeState *rt, int line_num);
static long parse_factor(Lexer *lex, RuntimeState *rt, int line_num);
static long parse_power(Lexer *lex, RuntimeState *rt, int line_num);
/* BValue expression parsing */
static BValue parse_factor_bval(Lexer *lex, RuntimeState *rt, int line_num);
static BValue parse_power_bval(Lexer *lex, RuntimeState *rt, int line_num);
static BValue parse_term_bval(Lexer *lex, RuntimeState *rt, int line_num);
static void parse_dim(Lexer *lex, RuntimeState *rt, int line_num);

/* Case-insensitive string comparison (C89-safe) */
static int str_case_equal(const char *a, const char *b)
{
 if (!a || !b) return 0;
 while (*a && *b) {
 if (toupper((unsigned char)*a) !=
 toupper((unsigned char)*b))
 return 0;
 a++; b++;
 }
 return (*a == '\0' && *b == '\0');
}

/*
 * set_param_by_name - Set a SUB/FUNCTION parameter.
 *
 * If the name is a single letter A-Z, sets the corresponding
 * single-letter variable (A-Z). Otherwise sets a named variable.
 * This is needed because the lexer treats single letters as
 * TOK_VARIABLE (using rt->variables[]), not named vars.
 */
static void set_param_by_name(RuntimeState *rt,
 const char *name, BValue val)
{
 int len = (int)strlen(name);
 if (len == 1 && name[0] >= 'A' && name[0] <= 'Z') {
 runtime_set_var_bval(rt, name[0], val);
 } else {
 runtime_set_named_var_bval(rt, name, len, val);
 }
}

/* --- Statement Parsing ---
 */

/*
 * parse_print - Parse and execute PRINT statement.
 *
 * PATB PRINT syntax:
 * PRINT expr-list
 * PRINT -> prints newline only
 * PRINT "string" -> prints string
 * PRINT expr -> prints number
 * PRINT #n -> sets field width to n
 * PRINT expr, expr -> prints with separator
 * PRINT expr; -> suppresses newline (but ; is
 * the statement separator in PATB,
 * so trailing , suppresses newline)
 *
 * The comma between items acts as a separator. A trailing comma
 * suppresses the newline at the end of the PRINT statement.
 */
/*
 * print_margin_check - Auto-wrap when cursor exceeds margin.
 *
 * Both ECMA-55 and GW-BASIC/QBasic wrap output to the next
 * line when the cursor position exceeds the defined margin
 * (screen_width). This is a no-op for file channel output.
 */
static void print_margin_check(RuntimeState *rt)
{
 if (rt->print_col > rt->screen_width) {
 printf("\n");
 rt->print_col = 1;
 rt->cursor_row++;
 }
}

static void parse_print(Lexer *lex, RuntimeState *rt, int line_num)
{
 int need_newline = 1; /* print newline unless trailing comma */
 char sep;
 int file_chan = 0; /* 0=stdout, 1-8=file channel */

 sep = dialect_get_separator();

 /*
 * PRINT #n, ...
 * If the first token is #, parse channel number then comma.
 * All output redirects to the file channel.
 */
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex); /* consume # */
 file_chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 /* Expect comma after channel number */
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume comma */
 }

 /*
 * PRINT > "file", expr-list (redirect, overwrite)
 * PRINT >> "file", expr-list (redirect, append)
 * PRINT | "command", expr-list (pipe to command)
 *
 * These redirect PRINT output to a file or pipe.
 * The file/command is opened, all PRINT output goes
 * there, and it is closed after the statement.
 */
 if (file_chan == 0 && (lex->current.type == TOK_GT ||
 lex->current.type == TOK_APPEND ||
 lex->current.type == TOK_PIPE)) {
 int redir_mode = lex->current.type;
 char rname[256];
 int rnlen;
 FILE *rfp = NULL;

 lexer_next(lex);
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 rnlen = lex->current.str_length;
 if (rnlen > 254) rnlen = 254;
 memcpy(rname, lex->current.str_start,
 (size_t)rnlen);
 rname[rnlen] = '\0';
 lexer_next(lex);

 /* Consume comma before expression list */
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);

 if (redir_mode == TOK_PIPE) {
#ifdef _WIN32
 rfp = _popen(rname, "w");
#else
 rfp = popen(rname, "w");
#endif
 } else if (redir_mode == TOK_APPEND) {
 rfp = fopen(rname, "a");
 } else {
 rfp = fopen(rname, "w");
 }

 if (rfp == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Print all expressions to the file/pipe */
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 BValue val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) break;
 if (bval_is_string(&val)) {
 if (val.v.sval.data)
 fwrite(val.v.sval.data, 1,
 (size_t)val.v.sval.length, rfp);
 } else {
 char buf[32];
 int blen = bval_to_string_buf(
 &val, buf, 32);
 fwrite(buf, 1, (size_t)blen, rfp);
 }
 if (lex->current.type == TOK_SEMICOLON) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_COMMA) {
 fprintf(rfp, "\t");
 lexer_next(lex);
 }
 }
 fprintf(rfp, "\n");

 if (redir_mode == TOK_PIPE) {
#ifdef _WIN32
 _pclose(rfp);
#else
 pclose(rfp);
#endif
 } else {
 fclose(rfp);
 }
 return;
 }

 /*
 * PRINT AT (col, row), expr-list
 * Relative cursor positioning then print.
 * Moves cursor to column col, row row.
 */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_AT) {
 int at_col, at_row;
 lexer_next(lex); /* consume AT */

 /* Expect ( col , row ) */
 if (lex->current.type == TOK_LPAREN)
 lexer_next(lex);
 at_col = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 at_row = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_RPAREN)
 lexer_next(lex);

 /* Clamp to valid range */
 if (at_row < 1) at_row = 1;
 if (at_col < 1) at_col = 1;

 /* Move cursor */
 printf("\033[%d;%dH", at_row, at_col);
 fflush(stdout);
 rt->cursor_row = at_row;
 rt->cursor_col = at_col;

 /* Consume separator before print-list */
 if (lex->current.type == TOK_COMMA ||
 lex->current.type == TOK_SEMICOLON)
 lexer_next(lex);
 }

 /*
 * PRINT @pos, expr-list
 * TRS-80 absolute cursor positioning.
 * Position = row * screen_width + col (0-based).
 * Converts to 1-based row,col for ANSI output.
 */
 if (lex->current.type == TOK_AT) {
 int abs_pos, at_row, at_col;
 int sw = rt->screen_width;
 lexer_next(lex); /* consume @ */

 if (sw < 1) sw = 64; /* TRS-80 default */

 abs_pos = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 if (abs_pos < 0) abs_pos = 0;
 at_row = (abs_pos / sw) + 1;
 at_col = (abs_pos % sw) + 1;

 /* Move cursor */
 printf("\033[%d;%dH", at_row, at_col);
 fflush(stdout);
 rt->cursor_row = at_row;
 rt->cursor_col = at_col;

 /* Consume separator before print-list */
 if (lex->current.type == TOK_COMMA ||
 lex->current.type == TOK_SEMICOLON)
 lexer_next(lex);
 }

 /*
 * PRINT USING "format"; expr [; expr...]
 * Format patterns:
 * # - digit placeholder
 * . - decimal point
 * \ \ - string field (width = # of chars between)
 */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_USING) {
 const char *fmt;
 int flen, fi;
 lexer_next(lex); /* consume USING */

 /* Parse format string */
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 fmt = lex->current.str_start;
 flen = lex->current.str_length;
 lexer_next(lex);

 /* Expect semicolon after format */
 if (lex->current.type == TOK_SEMICOLON)
 lexer_next(lex);

 /* Process format string, consume values */
 fi = 0;
 while (fi < flen) {
 if (fmt[fi] == '#' || fmt[fi] == '.') {
 /* Numeric field: count # before and after . */
 int whole = 0, frac = 0, has_dot = 0;
 int fw;
 BValue v;
 char nbuf[64];

 while (fi < flen && fmt[fi] == '#') {
 whole++; fi++;
 }
 if (fi < flen && fmt[fi] == '.') {
 has_dot = 1; fi++;
 while (fi < flen && fmt[fi] == '#') {
 frac++; fi++;
 }
 }

 /* Parse next value */
 v = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;

 fw = whole + (has_dot ? 1 + frac : 0);
 if (has_dot) {
 sprintf(nbuf, "%*.*f",
 fw, frac, bval_to_float(&v));
 } else {
 sprintf(nbuf, "%*ld",
 fw, bval_to_int(&v));
 }
 printf("%s", nbuf);

 /* Consume separator */
 if (lex->current.type == TOK_SEMICOLON)
 lexer_next(lex);
 else if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 } else if (fmt[fi] == '\\') {
 /* String field: \ \ = width chars */
 int width = 2;
 BValue v;
 fi++; /* skip first \ */
 while (fi < flen && fmt[fi] != '\\') {
 width++; fi++;
 }
 if (fi < flen) fi++; /* skip closing \ */

 v = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;

 if (bval_is_string(&v) &&
 v.v.sval.data != NULL) {
 int slen = v.v.sval.length;
 int pad = 0;
 if (slen > width) slen = width;
 printf("%.*s", slen, v.v.sval.data);
 pad = width - slen;
 while (pad-- > 0) printf(" ");
 }

 if (lex->current.type == TOK_SEMICOLON)
 lexer_next(lex);
 else if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 } else {
 /* Literal character in format string */
 printf("%c", fmt[fi]);
 fi++;
 }
 }
 printf("\n");
 return;
 }

 /* Handle empty PRINT (just prints a newline) */
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type == TOK_SEMICOLON && sep == ';') ||
 (lex->current.type == TOK_COLON && sep == ':')) {
 if (file_chan > 0) {
 fileio_print_newline(file_chan, line_num);
 } else {
 printf("\n");
 }
 return;
 }

 while (!error_occurred()) {
 need_newline = 1;

 /* Check for # format specifier (only for stdout) */
 if (lex->current.type == TOK_HASH && file_chan == 0) {
 long width;
 lexer_next(lex); /* consume # */
 width = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 rt->print_width = (int)width;
 /* Continue to next item */
 }
 /*
 * TAB(n) - Move to column n.
 * Emits spaces until the cursor reaches column n.
 * Vintage BASIC games use: PRINT TAB(26);"TITLE"
 */
 else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_TAB_FUNC) {
 int col;
 lexer_next(lex); /* consume TAB */
 if (lex->current.type != TOK_LPAREN) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume ( */
 col = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type != TOK_RPAREN) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume ) */
 if (col > 0 && file_chan == 0) {
 /* ECMA-55: move to absolute column col */
 if (rt->print_col > col) {
 /* Past target: advance to next line */
 printf("\n");
 rt->print_col = 1;
 }
 while (rt->print_col < col) {
 putchar(' ');
 rt->print_col++;
 }
 }
 }
 /*
 * SPC(n) - Print n spaces.
 * Vintage BASIC: PRINT SPC(5);"TEXT"
 */
 else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_SPC_FUNC) {
 int n;
 lexer_next(lex); /* consume SPC */
 if (lex->current.type != TOK_LPAREN) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume ( */
 n = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type != TOK_RPAREN) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume ) */
 if (n > 0 && file_chan == 0) {
 int i;
 for (i = 0; i < n; i++)
 putchar(' ');
 }
 }
 /* Expression (numeric or string value via BValue) */
 else {
 BValue val;
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;

 if (file_chan > 0) {
 /* Write to file channel */
 char buf[64];
 if (bval_is_string(&val)) {
 char sbuf[MAX_LINE_LENGTH + 1];
 int slen = val.v.sval.length;
 if (slen > MAX_LINE_LENGTH)
 slen = MAX_LINE_LENGTH;
 memcpy(sbuf, val.v.sval.data, (size_t)slen);
 sbuf[slen] = '\0';
 fileio_print(file_chan, sbuf, line_num);
 } else if (bval_is_float(&val)) {
 sprintf(buf, "%G", val.v.fval);
 fileio_print(file_chan, buf, line_num);
 } else {
 sprintf(buf, "%ld", val.v.ival);
 fileio_print(file_chan, buf, line_num);
 }
 } else {
 /* Print to stdout (original behavior) */
 if (bval_is_string(&val)) {
 int si;
 for (si = 0; si < val.v.sval.length; si++) {
 putchar(val.v.sval.data[si]);
 rt->print_col++;
 print_margin_check(rt);
 }
 } else if (bval_is_float(&val)) {
 int nc;
 /* ECMA-55: leading space for positive */
 if (val.v.fval >= 0.0) {
 putchar(' ');
 rt->print_col++;
 }
 nc = printf("%G", val.v.fval);
 rt->print_col += nc;
 /* ECMA-55: trailing space */
 putchar(' ');
 rt->print_col++;
 print_margin_check(rt);
 } else {
 int nc;
 /* ECMA-55: leading space for positive */
 if (val.v.ival >= 0) {
 putchar(' ');
 rt->print_col++;
 }
 nc = printf("%ld", val.v.ival);
 rt->print_col += nc;
 /* ECMA-55: trailing space */
 putchar(' ');
 rt->print_col++;
 print_margin_check(rt);
 }
 }
 }

 /* Check for comma or semicolon separator between items */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume comma */
 need_newline = 0; /* trailing comma = no newline */
 /* ECMA-55: advance to next print zone */
 if (file_chan == 0) {
 int zone = dialect_get_config()->print_zone_width;
 if (zone < 1) zone = 14;
 while ((rt->print_col - 1) % zone != 0) {
 putchar(' ');
 rt->print_col++;
 }
 }
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR) {
 break;
 }
 } else if (lex->current.type == TOK_SEMICOLON) {
 lexer_next(lex); /* consume semicolon */
 need_newline = 0;
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR) {
 break;
 }
 } else if (lex->current.type == TOK_COLON && sep == ':') {
 break;
 } else {
 break;
 }
 }

 if (need_newline && !error_occurred()) {
 if (file_chan > 0) {
 fileio_print_newline(file_chan, line_num);
 } else {
 printf("\n");
 rt->print_col = 1;
 rt->cursor_row++;
 rt->cursor_col = 1;
 }
 }
}

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
static void parse_let(Lexer *lex, RuntimeState *rt, int line_num,
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
 int idx1, idx2 = 0;
 BValue val;
 lexer_next(lex); /* consume ( */
 idx1 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_dim(rt, &var_name, 1, idx1, idx2, val, line_num);
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
 int idx1, idx2 = 0;
 BValue dval;
 lexer_next(lex); /* consume ( */
 idx1 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;
 dval = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_dim(rt, name, name_len,
 idx1, idx2, dval, line_num);
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
 int idx1, idx2 = 0;
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
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_dim(rt, sname, 2, idx1, idx2,
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

/*
 * parse_input - Parse and execute INPUT statement.
 *
 * Syntax:
 * INPUT var-list
 * INPUT "prompt", var-list
 * INPUT "prompt" var-list (PATB: no comma after prompt)
 *
 * Reads integer values from stdin. For each variable in the list,
 * prints "?" prompt (or custom prompt) and reads a number.
 */
static void parse_input(Lexer *lex, RuntimeState *rt, int line_num)
{
 int has_custom_prompt = 0;
 char input_buf[INPUT_BUFFER_SIZE];
 int file_chan = 0; /* 0=stdin, 1-8=file channel */
 /* For file INPUT#: read whole line, split on commas */
 char file_buf[INPUT_BUFFER_SIZE];
 int file_buf_valid = 0;
 int file_buf_pos = 0;

 /*
 * INPUT #n, var
 * If first token is #, read from file channel.
 */
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex); /* consume # */
 file_chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume comma */
 }

 /* Check for custom prompt string (only for stdin) */
 if (file_chan == 0 && lex->current.type == TOK_STRING) {
 int i;
 for (i = 0; i < lex->current.str_length; i++) {
 putchar(lex->current.str_start[i]);
 }
 lexer_next(lex); /* consume string */
 has_custom_prompt = 1;

 /* Skip optional comma or semicolon after prompt */
 if (lex->current.type == TOK_COMMA ||
 lex->current.type == TOK_SEMICOLON) {
 lexer_next(lex);
 }
 }

 /* Read each variable */
 while (!error_occurred()) {
 char var_name;
 long value;
 char *endptr;

 if (lex->current.type == TOK_VARIABLE) {
 var_name = lex->current.value.var_name;
 lexer_next(lex); /* consume variable */
 } else if (lex->current.type == TOK_AT) {
 /* @(expr) = input */
 long index;
 lexer_next(lex); /* consume @ */
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 index = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN)) return;

 if (file_chan > 0) {
 if (!file_buf_valid) {
 if (fileio_input_line(file_chan,
 file_buf, INPUT_BUFFER_SIZE,
 line_num) != 0) {
 return;
 }
 file_buf_valid = 1;
 file_buf_pos = 0;
 }
 {
 int di = 0;
 while (file_buf[file_buf_pos] != '\0'
 && file_buf[file_buf_pos] != ','
 && file_buf[file_buf_pos] != '\n'
 && file_buf[file_buf_pos] != '\r'
 && di < INPUT_BUFFER_SIZE - 1) {
 input_buf[di++] =
 file_buf[file_buf_pos++];
 }
 input_buf[di] = '\0';
 if (file_buf[file_buf_pos] == ',')
 file_buf_pos++;
 }
 } else {
 if (!has_custom_prompt) {
 printf("? ");
 }
 fflush(stdout);
 if (fgets(input_buf, INPUT_BUFFER_SIZE, stdin)
 == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 }

 value = strtol(input_buf, &endptr, 10);
 while (endptr == input_buf) {
 if (file_chan > 0) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 printf("?Redo from start\n? ");
 fflush(stdout);
 if (fgets(input_buf, INPUT_BUFFER_SIZE,
 stdin) == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 value = strtol(input_buf, &endptr, 10);
 }

 runtime_set_array(rt, index, value);
 goto check_more_vars;
 } else if (lex->current.type == TOK_STRING_VAR) {
 /* String variable INPUT: reads entire line as string */
 char svar_name = lex->current.value.var_name;
 int slen;
 char *ptr;
 lexer_next(lex);

 if (file_chan > 0) {
 /* Use shared buffer, split on commas */
 if (!file_buf_valid) {
 if (fileio_input_line(file_chan,
 file_buf, INPUT_BUFFER_SIZE,
 line_num) != 0) {
 return;
 }
 file_buf_valid = 1;
 file_buf_pos = 0;
 }
 {
 int di = 0;
 while (file_buf[file_buf_pos] != '\0'
 && file_buf[file_buf_pos] != ','
 && file_buf[file_buf_pos] != '\n'
 && file_buf[file_buf_pos] != '\r'
 && di < INPUT_BUFFER_SIZE - 1) {
 input_buf[di++] =
 file_buf[file_buf_pos++];
 }
 input_buf[di] = '\0';
 if (file_buf[file_buf_pos] == ',')
 file_buf_pos++;
 }
 } else {
 if (!has_custom_prompt) {
 printf("? ");
 }
 fflush(stdout);
 if (fgets(input_buf, INPUT_BUFFER_SIZE, stdin)
 == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 }

 /* Strip trailing newline */
 slen = (int)strlen(input_buf);
 while (slen > 0 && (input_buf[slen-1] == '\n' ||
 input_buf[slen-1] == '\r')) {
 slen--;
 }

 ptr = strpool_store(&rt->strpool, input_buf, slen);
 if (ptr == NULL) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 runtime_set_string_var(rt, svar_name,
 bval_string(ptr, slen));
 goto check_more_vars;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 if (file_chan > 0) {
 /* Multi-var file input: read line once,
 * split on commas for subsequent vars */
 if (!file_buf_valid) {
 if (fileio_input_line(file_chan,
 file_buf, INPUT_BUFFER_SIZE,
 line_num) != 0) {
 return;
 }
 file_buf_valid = 1;
 file_buf_pos = 0;
 }
 /* Extract next comma-delimited value */
 {
 int di = 0;
 while (file_buf[file_buf_pos] != '\0' &&
 file_buf[file_buf_pos] != ',' &&
 file_buf[file_buf_pos] != '\n' &&
 file_buf[file_buf_pos] != '\r' &&
 di < INPUT_BUFFER_SIZE - 1) {
 input_buf[di++] =
 file_buf[file_buf_pos++];
 }
 input_buf[di] = '\0';
 /* Skip comma */
 if (file_buf[file_buf_pos] == ',')
 file_buf_pos++;
 }
 } else {
 /* Print prompt if no custom prompt */
 if (!has_custom_prompt) {
 printf("? ");
 }
 fflush(stdout);

 /* Read input line */
 if (fgets(input_buf, INPUT_BUFFER_SIZE, stdin) == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 }

 /* Parse integer */
 value = strtol(input_buf, &endptr, 10);
 while (endptr == input_buf) {
 /*
 * Not a number. If reading from stdin,
 * reprompt (GW-BASIC/ECMA-55 behavior).
 * If from file, it's a hard error.
 */
 if (file_chan > 0) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 printf("?Redo from start\n? ");
 fflush(stdout);
 if (fgets(input_buf, INPUT_BUFFER_SIZE,
 stdin) == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 value = strtol(input_buf, &endptr, 10);
 }

 runtime_set_var(rt, var_name, value);

check_more_vars:
 /* Check for comma (more variables) */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume comma */
 has_custom_prompt = 0; /* reset prompt for next var */
 } else {
 break;
 }
 }
}

/*
 * parse_if - Parse and execute IF statement.
 *
 * Supports TWO forms:
 *
 * 1. SINGLE-LINE IF (original):
 * IF condition THEN statement [ELSE statement]
 *
 * 2. BLOCK IF (ECMA-116 / QBasic):
 * IF condition THEN
 * ...statements...
 * [ELSEIF condition THEN
 * ...statements...]
 * [ELSE
 * ...statements...]
 * END IF
 *
 * Detection: If THEN is followed by end-of-line (no statement),
 * it's a block IF. Otherwise, it's single-line.
 *
 * PATB syntax (NO THEN keyword):
 * IF expression relop expression statement
 */

/*
 * block_if_skip_to_end - Skip forward from current position to END IF.
 *
 * Used when the TRUE branch of a block IF has been executed and
 * we encounter ELSEIF or ELSE - we need to skip all remaining
 * branches to END IF.
 *
 * Tracks nesting depth to handle nested block IFs correctly.
 */
static void block_if_skip_to_end(RuntimeState *rt, int line_num)
{
 ProgramStore *pgm = rt->program;
 int idx = rt->current_index + 1;
 int depth = 0;

 while (idx < pgm->count) {
 Lexer cl;
 const char *cline = pgm->lines[idx].text;
 lexer_init(&cl, cline);

 /* Skip line number */
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);

 if (cl.current.type == TOK_KEYWORD) {
 KeywordId kk = cl.current.value.keyword;

 /* Check for nested IF (block form) */
 if (kk == KW_IF) {
 /* Scan to see if this IF has THEN at EOL */
 lexer_next(&cl);
 /* Skip condition tokens */
 while (cl.current.type != TOK_EOF &&
 cl.current.type != TOK_CR) {
 if (cl.current.type == TOK_KEYWORD &&
 cl.current.value.keyword == KW_THEN) {
 lexer_next(&cl);
 /* THEN at EOL = block IF */
 if (cl.current.type == TOK_EOF ||
 cl.current.type == TOK_CR) {
 depth++;
 }
 break;
 }
 lexer_next(&cl);
 }
 idx++;
 continue;
 }

 /* END IF at our depth */
 if (kk == KW_END) {
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD &&
 cl.current.value.keyword == KW_IF) {
 if (depth > 0) {
 depth--;
 } else {
 /* Found our END IF */
 rt->current_index = idx;
 rt->next_index = idx;
 return;
 }
 }
 }

 /* ENDIF (single word) at our depth */
 if (kk == KW_ENDIF) {
 if (depth > 0) {
 depth--;
 } else {
 rt->current_index = idx;
 rt->next_index = idx;
 return;
 }
 }
 }
 idx++;
 }

 /* No END IF found */
 error_raise(ERR_WHAT, line_num);
}

/*
 * block_if_scan - Scan forward for ELSEIF, ELSE, or END IF.
 *
 * Used when a block IF condition is FALSE. Scans forward to find
 * the next ELSEIF (to try another condition), ELSE (to execute
 * the default), or END IF (to skip everything).
 *
 * Sets rt->current_index and rt->next_index to the found line.
 * Returns the keyword found: KW_ELSEIF, KW_ELSE, KW_ENDIF, or KW_END.
 */
static KeywordId block_if_scan(RuntimeState *rt, int line_num)
{
 ProgramStore *pgm = rt->program;
 int idx = rt->current_index + 1;
 int depth = 0;

 while (idx < pgm->count) {
 Lexer cl;
 const char *cline = pgm->lines[idx].text;
 lexer_init(&cl, cline);

 /* Skip line number */
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);

 if (cl.current.type == TOK_KEYWORD) {
 KeywordId kk = cl.current.value.keyword;

 /* Nested block IF */
 if (kk == KW_IF) {
 lexer_next(&cl);
 while (cl.current.type != TOK_EOF &&
 cl.current.type != TOK_CR) {
 if (cl.current.type == TOK_KEYWORD &&
 cl.current.value.keyword == KW_THEN) {
 lexer_next(&cl);
 if (cl.current.type == TOK_EOF ||
 cl.current.type == TOK_CR) {
 depth++;
 }
 break;
 }
 lexer_next(&cl);
 }
 idx++;
 continue;
 }

 /* END IF */
 if (kk == KW_END) {
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD &&
 cl.current.value.keyword == KW_IF) {
 if (depth > 0) {
 depth--;
 } else {
 rt->current_index = idx;
 rt->next_index = idx;
 return KW_ENDIF;
 }
 }
 }

 /* ENDIF (single word) */
 if (kk == KW_ENDIF) {
 if (depth > 0) {
 depth--;
 } else {
 rt->current_index = idx;
 rt->next_index = idx;
 return KW_ENDIF;
 }
 }

 /* ELSEIF at our depth */
 if (kk == KW_ELSEIF && depth == 0) {
 rt->current_index = idx;
 rt->next_index = idx;
 return KW_ELSEIF;
 }

 /* ELSE at our depth */
 if (kk == KW_ELSE && depth == 0) {
 rt->current_index = idx;
 rt->next_index = idx;
 return KW_ELSE;
 }
 }
 idx++;
 }

 /* No matching END IF */
 error_raise(ERR_WHAT, line_num);
 return KW_END; /* unreachable */
}

/*
 * when_exception_scan - Scan forward for USE and END WHEN.
 *
 * Starting from the WHEN EXCEPTION IN line, find the matching
 * USE (handler) and END WHEN (block end) at the same depth.
 *
 * Returns 1 on success, 0 on error. Populates use_idx and
 * end_when_idx with ProgramStore indices.
 */
static int when_exception_scan(RuntimeState *rt, int when_idx,
 int *use_idx, int *end_when_idx,
 int line_num)
{
 ProgramStore *pgm = rt->program;
 int idx = when_idx + 1;
 int depth = 0;
 int found_use = -1;

 while (idx < pgm->count) {
 Lexer cl;
 const char *cline = pgm->lines[idx].text;
 lexer_init(&cl, cline);

 /* Skip line number */
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);

 if (cl.current.type == TOK_KEYWORD) {
 KeywordId kk = cl.current.value.keyword;

 /* Nested WHEN EXCEPTION IN */
 if (kk == KW_WHEN) {
 depth++;
 idx++;
 continue;
 }

 /* USE at our depth */
 if (kk == KW_USE && depth == 0) {
 found_use = idx;
 }

 /* END WHEN */
 if (kk == KW_END) {
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD &&
 cl.current.value.keyword == KW_WHEN) {
 if (depth > 0) {
 depth--;
 } else {
 if (found_use < 0) {
 error_raise(ERR_WHAT, line_num);
 return 0;
 }
 *use_idx = found_use;
 *end_when_idx = idx;
 return 1;
 }
 }
 }
 }
 idx++;
 }

 /* No matching END WHEN */
 error_raise(ERR_WHAT, line_num);
 return 0;
}

static void parse_if(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue cond_val;
 int condition;

 /*
 * Parse condition as a full expression.
 * Comparisons (=, <, >, etc.) and logical operators
 * (AND, OR, NOT) are handled by parse_expression_bval.
 * Result: non-zero = true, zero = false.
 */
 cond_val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 condition = (bval_to_int(&cond_val) != 0);

 /*
 * Consume optional THEN keyword (GW-BASIC uses THEN,
 * PATB does not).
 */
 if (dialect_get_config()->has_then_keyword) {
 if (lexer_match_keyword(lex, KW_THEN)) {
 lexer_next(lex); /* consume THEN */
 }
 }

 /*
 * Block IF detection: if THEN is followed by end-of-line,
 * this is a multi-line block IF.
 */
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR) {
 /* ========== BLOCK IF ========== */
 if (condition) {
 /*
 * TRUE: increment block_if_depth so that
 * ELSEIF/ELSE handlers know to skip to END IF.
 * Execution continues on the next line naturally.
 */
 rt->block_if_depth++;
 return;
 } else {
 /*
 * FALSE: scan forward for ELSEIF/ELSE/END IF.
 */
 KeywordId found = block_if_scan(rt, line_num);

 if (found == KW_ELSEIF) {
 /*
 * Found ELSEIF - the exec loop will execute
 * this line, which triggers the ELSEIF handler.
 * The ELSEIF handler evaluates its condition.
 */
 return;
 }
 if (found == KW_ELSE) {
 /*
 * Found ELSE - execution continues on the
 * line AFTER the ELSE.
 */
 return;
 }
 /* KW_ENDIF - skip past it, done */
 return;
 }
 }

 /* ========== SINGLE-LINE IF (original behavior) ========== */

 if (condition) {
 /*
 * TRUE path: execute THEN clause.
 *
 * Special case: IF...THEN linenum
 * In GW-BASIC, "IF A=5 THEN 100" means
 * "IF A=5 THEN GOTO 100". If the next token
 * is a number, treat as implicit GOTO.
 */
 if (lex->current.type == TOK_NUMBER) {
 int target = (int)lex->current.value.num_value;
 lexer_next(lex);
 vm_jump(rt, target, line_num);
 lexer_skip_to_end(lex);
 return;
 }
 parse_statement(lex, rt, line_num);
 if (error_occurred()) return;

 /*
 * Check if there's an ELSE to skip.
 * The multi-statement handler may have stopped
 * at a colon. We need to scan forward through
 * remaining tokens looking for ELSE.
 */
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ELSE) {
 /* Found ELSE - skip rest of line */
 lexer_skip_to_end(lex);
 return;
 }
 /* Skip colon separators and continue */
 if (lex->current.type == TOK_COLON) {
 lexer_next(lex);
 /* Check for ELSE after colon */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword ==
 KW_ELSE) {
 lexer_skip_to_end(lex);
 return;
 }
 /* Not ELSE - execute this statement
 * (part of THEN clause) */
 parse_statement(lex, rt, line_num);
 if (error_occurred()) return;
 } else {
 break;
 }
 }
 } else {
 /*
 * FALSE path: skip THEN clause, find ELSE.
 *
 * Scan the raw source string for " ELSE "
 * boundary. We must skip over quoted strings
 * to avoid matching ELSE inside a string
 * literal like PRINT "ELSE".
 */
 const char *src = lex->source;
 int len = lex->length;
 int p = lex->pos;
 int found_else = -1;

 while (p < len) {
 /* Skip string literals */
 if (src[p] == '"') {
 p++;
 while (p < len && src[p] != '"') p++;
 if (p < len) p++; /* skip closing quote */
 continue;
 }
 /* Look for ELSE keyword boundary */
 if ((src[p] == 'E' || src[p] == 'e') &&
 p + 4 <= len) {
 int ok = 1;
 /* Check "ELSE" case-insensitive */
 if ((src[p+1] != 'L' && src[p+1] != 'l')
 || (src[p+2] != 'S' && src[p+2] != 's')
 || (src[p+3] != 'E' &&
 src[p+3] != 'e')) {
 ok = 0;
 }
 /* Check word boundary before */
 if (ok && p > 0 &&
 ((src[p-1] >= 'A' && src[p-1] <= 'Z')
 || (src[p-1] >= 'a' &&
 src[p-1] <= 'z'))) {
 ok = 0;
 }
 /* Check word boundary after */
 if (ok && p + 4 < len &&
 ((src[p+4] >= 'A' && src[p+4] <= 'Z')
 || (src[p+4] >= 'a' &&
 src[p+4] <= 'z'))) {
 ok = 0;
 }
 if (ok) {
 found_else = p + 4;
 break;
 }
 }
 p++;
 }

 if (found_else >= 0) {
 /*
 * Reposition the lexer after "ELSE"
 * and execute the ELSE clause.
 */
 lex->pos = found_else;
 lexer_next(lex); /* prime first token */

 /* Skip optional whitespace already handled
 * by lexer_next */
 parse_statement(lex, rt, line_num);
 } else {
 /* No ELSE - skip entire line */
 lexer_skip_to_end(lex);
 }
 }
}

/*
 * parse_goto - Parse and execute GOTO.
 *
 * Syntax: GOTO expression
 *
 * Evaluates the expression to get a target line number, then
 * sets the runtime's next_index to the target line. If the
 * target line doesn't exist, raises ERR_HOW.
 *
 * PATB rule: GOTO must be the last command on a line.
 */
static void parse_goto(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * Check for label-style GOTO (e.g., GOTO MyLabel).
 * If the current token is an identifier (not a number),
 * try resolving it as a label first.
 */
 if (lex->current.type == TOK_NAMED_VAR ||
 (lex->current.type == TOK_KEYWORD &&
 lex->current.str_start != NULL)) {
 const char *nm = lex->current.str_start;
 int nlen = lex->current.str_length;
 int idx;

 if (nm != NULL && nlen > 0) {
 idx = runtime_find_label(rt, nm, nlen);
 if (idx >= 0) {
 lexer_next(lex);
 rt->next_index = idx;
 lexer_skip_to_end(lex);
 return;
 }
 }
 }

 {
 long target;
 target = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /* Use VM control flow primitive */
 vm_jump(rt, (int)target, line_num);

 /*
 * ECMA-55 s10.4: GOTO must not transfer control
 * into the body of a FOR..NEXT loop. In strict
 * mode, check active FOR frames on the stack.
 * Warn if the target falls within a FOR body
 * that was entered from a different context.
 * Warning only - execution continues normally.
 */
 if (!error_occurred() && dialect_is_strict()) {
 int fi;
 int tgt_idx = rt->next_index;
 for (fi = 0; fi < rt->stack_top; fi++) {
 if (rt->stack[fi].type == FRAME_FOR) {
 int body = rt->stack[fi].data
 .for_loop.body_index;
 /*
 * If jumping into the middle of a
 * loop body (after the FOR but the
 * jump didn't come from within the
 * loop), warn. A rough check: if
 * the target is >= body_index and
 * the jump source is outside, warn.
 */
 if (tgt_idx >= body &&
 (rt->current_index < body - 1 ||
 rt->current_index > tgt_idx)) {
 printf("Warning: GOTO into FOR"
 " body (ECMA-55)\n");
 break;
 }
 }
 }
 }
 }

 /* GOTO must be last - skip to end of line */
 lexer_skip_to_end(lex);
}

/*
 * parse_gosub - Parse and execute GOSUB.
 *
 * Syntax: GOSUB expression
 *
 * Pushes the return address (next line after current) onto the
 * stack, then transfers to the target line.
 *
 * PATB rule: GOSUB must be the last command on a line.
 */
static void parse_gosub(Lexer *lex, RuntimeState *rt, int line_num)
{
 /* Check for label-style GOSUB */
 if (lex->current.type == TOK_NAMED_VAR ||
 (lex->current.type == TOK_KEYWORD &&
 lex->current.str_start != NULL)) {
 const char *nm = lex->current.str_start;
 int nlen = lex->current.str_length;
 int idx;

 if (nm != NULL && nlen > 0) {
 idx = runtime_find_label(rt, nm, nlen);
 if (idx >= 0) {
 StackFrame frame;
 lexer_next(lex);
 frame.type = FRAME_GOSUB;
 frame.data.gosub.return_index =
 rt->current_index + 1;
 if (runtime_push(rt, &frame) != 0)
 return;
 rt->next_index = idx;
 lexer_skip_to_end(lex);
 return;
 }
 }
 }

 {
 long target;
 target = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /* Use VM control flow primitive */
 vm_call(rt, (int)target, line_num);
 }

 /* GOSUB must be last - skip to end of line */
 lexer_skip_to_end(lex);
}

/*
 * parse_return - Parse and execute RETURN.
 *
 * Pops a GOSUB frame from the stack and transfers to the
 * return address.
 *
 * PATB rule: RETURN must be the last command on a line.
 */
static void parse_return(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)lex;

 /* Use VM control flow primitive */
 vm_return_sub(rt, line_num);

 /* RETURN must be last - skip to end */
 lexer_skip_to_end(lex);
}

/*
 * parse_end - Parse END statement.
 *
 * Stops program execution and returns to the READY prompt.
 */
static void parse_end(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)line_num;

 /*
 * END IF - end of block IF.
 * Decrement block_if_depth if inside a true block.
 */
 if (lexer_match_keyword(lex, KW_IF)) {
 lexer_next(lex); /* consume IF */
 if (rt->block_if_depth > 0)
 rt->block_if_depth--;
 return;
 }

 /*
 * END WHEN - end of WHEN EXCEPTION block.
 * Pop the exception frame if present.
 * (May already be popped by USE fall-through or
 * CONTINUE - safe to check.)
 */
 if (lexer_match_keyword(lex, KW_WHEN)) {
 int i;
 lexer_next(lex); /* consume WHEN */
 /* Find and pop innermost exception frame */
 for (i = rt->stack_top - 1; i >= 0; i--) {
 if (rt->stack[i].type == FRAME_EXCEPTION) {
 rt->stack_top = i;
 break;
 }
 }
 return;
 }

 /*
 * END SELECT is a no-op when reached by fall-through.
 * The CASE handler already jumped past it.
 */
 if (lexer_match_keyword(lex, KW_SELECT)) {
 lexer_next(lex); /* consume SELECT */
 return;
 }

 /*
 * END SUB - return from subprogram.
 * Pop FRAME_SUB frame, restore saved variables.
 */
 if (lexer_match_keyword(lex, KW_SUB)) {
 StackFrame frame;
 int i;
 lexer_next(lex); /* consume SUB */

 if (runtime_pop(rt, FRAME_SUB, &frame) != 0) {
 return; /* error already raised */
 }

 /* Restore saved variables */
 for (i = 0; i < MAX_VARIABLES; i++) {
 rt->variables[i] = frame.data.sub_call
 .saved_vars[i];
 }
 for (i = 0; i < MAX_STRING_VARS; i++) {
 rt->string_vars[i] = frame.data.sub_call
 .saved_strvars[i];
 }

 rt->in_sub_index = -1;
 rt->next_index = frame.data.sub_call.return_index;
 return;
 }

 /*
 * END FUNCTION - same as END SUB but for functions.
 */
 if (lexer_match_keyword(lex, KW_FUNCTION)) {
 StackFrame frame;
 int i;
 lexer_next(lex); /* consume FUNCTION */

 if (runtime_pop(rt, FRAME_SUB, &frame) != 0) {
 return;
 }

 /* Restore saved variables */
 for (i = 0; i < MAX_VARIABLES; i++) {
 rt->variables[i] = frame.data.sub_call
 .saved_vars[i];
 }
 for (i = 0; i < MAX_STRING_VARS; i++) {
 rt->string_vars[i] = frame.data.sub_call
 .saved_strvars[i];
 }

 rt->in_sub_index = -1;
 rt->next_index = frame.data.sub_call.return_index;
 return;
 }

 /*
 * END DEF - end of multi-line DEF FN (ECMA-116).
 * Same as END FUNCTION - pop FRAME_SUB, restore vars.
 */
 if (lexer_match_keyword(lex, KW_DEF)) {
 StackFrame frame;
 int i;
 lexer_next(lex); /* consume DEF */

 if (runtime_pop(rt, FRAME_SUB, &frame) != 0) {
 return;
 }

 /* Restore saved variables */
 for (i = 0; i < MAX_VARIABLES; i++) {
 rt->variables[i] = frame.data.sub_call
 .saved_vars[i];
 }
 for (i = 0; i < MAX_STRING_VARS; i++) {
 rt->string_vars[i] = frame.data.sub_call
 .saved_strvars[i];
 }

 rt->in_sub_index = -1;
 rt->next_index = frame.data.sub_call.return_index;
 return;
 }

 /* Use VM state machine */
 vm_halt(rt);

 lexer_skip_to_end(lex);
}

/*
 * parse_stop - Parse STOP statement.
 *
 * Same as END but may print a message. For now, identical to END.
 * PATB rule: STOP must be the last command on a line.
 */
static void parse_stop(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)line_num;

 printf("[STOP at line %d]\n", line_num);

 /* Use VM_PAUSED so CONT can resume */
 vm_set_state(rt, VM_PAUSED);

 lexer_skip_to_end(lex);
}

/*
 * parse_rem - Parse REM (remark/comment).
 *
 * Skips the entire rest of the line. No execution effect.
 */
static void parse_rem(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)rt;
 (void)line_num;

 lexer_skip_to_end(lex);
}

/*
 * parse_list_cmd - Parse LIST command.
 *
 * GW-BASIC compatible syntax with BASIC++ extensions:
 * LIST -> list all lines
 * LIST n -> list single line n
 * LIST n- -> list from line n to end
 * LIST -n -> list from start to line n
 * LIST n-m -> list lines n through m
 * LIST n,m-p -> list line n, then lines m through p
 * LIST n,m,p -> list lines n, m, and p individually
 *
 * Comma separates independent segments. Each segment is
 * either a single line number or a range (with '-').
 */
static void parse_list_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)line_num;

 /* No arguments: list everything */
 if (lex->current.type != TOK_NUMBER &&
 lex->current.type != TOK_MINUS) {
 program_list(&rt->memory->program, 0, 0);
 return;
 }

 /*
 * Parse comma-separated segments.
 * Each segment is one of:
 * n -> single line (from=n, to=n)
 * n- -> from n to end (from=n, to=0)
 * n-m -> range (from=n, to=m)
 * -n -> from start to n (from=0, to=n)
 */
 for (;;) {
 int from = 0;
 int to = 0;

 /* Case: -n (start to line n) */
 if (lex->current.type == TOK_MINUS) {
 lexer_next(lex); /* consume '-' */
 if (lex->current.type == TOK_NUMBER) {
 to = (int)lex->current.value.num_value;
 lexer_next(lex);
 }
 program_list(&rt->memory->program, from, to);
 }
 /* Case: starts with a number */
 else if (lex->current.type == TOK_NUMBER) {
 from = (int)lex->current.value.num_value;
 lexer_next(lex);

 if (lex->current.type == TOK_MINUS) {
 /* n- or n-m */
 lexer_next(lex); /* consume '-' */
 if (lex->current.type == TOK_NUMBER) {
 to = (int)lex->current.value.num_value;
 lexer_next(lex);
 }
 /* else to=0 means "to end" */
 program_list(&rt->memory->program, from, to);
 } else {
 /* Single line: n */
 to = from;
 program_list(&rt->memory->program, from, to);
 }
 } else {
 /* Unexpected token, stop */
 break;
 }

 /* Check for comma separator to continue */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume ',' */
 } else {
 break;
 }
 }
}

/*
 * parse_run_cmd - Parse RUN command.
 *
 * Resets the runtime state and begins execution from the first line.
 */
static void parse_run_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)lex;
 (void)line_num;

 if (rt->memory->program.count == 0) {
 /* No program to run */
 return;
 }

 /*
 * ECMA-55: END must be the last line of the program.
 * Check in strict mode only. Warn but still execute.
 */
 if (dialect_is_strict()) {
 ProgramStore *ps = &rt->memory->program;
 int last = ps->count - 1;
 if (last >= 0) {
 Lexer chk;
 lexer_init(&chk, ps->lines[last].text);
 if (chk.current.type == TOK_NUMBER)
 lexer_next(&chk);
 if (!(chk.current.type == TOK_KEYWORD &&
 chk.current.value.keyword == KW_END)) {
 printf("Warning: END is not the "
 "last statement (ECMA-55)\n");
 }
 }
 }

 exec_run(rt);
}

/*
 * parse_new_cmd - Parse NEW command.
 *
 * Clears the program store and resets runtime state.
 */
static void parse_new_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)lex;
 (void)line_num;

 program_clear(&rt->memory->program);
 runtime_reset(rt);
}

/*
 * parse_save_cmd - Parse SAVE command.
 *
 * Syntax: SAVE "filename"
 */
static void parse_save_cmd(Lexer *lex, RuntimeState *rt, int line_num);

/*
 * parse_load_cmd - Parse LOAD command.
 *
 * Syntax: LOAD "filename"
 */
static void parse_load_cmd(Lexer *lex, RuntimeState *rt, int line_num);

/* --- Loop Statement Handlers ---
 * All loops use the unified stack frame system. Each loop type
 * pushes a typed frame when entering the loop and pops it when
 * exiting. The type-checking in runtime_pop() prevents mismatched
 * NEXT/WEND/LOOP from corrupting execution.
 *
 * Loop execution model:
 * FOR: push frame, body executes, NEXT checks/increments/jumps
 * WHILE: evaluate condition, push frame if true, skip to WEND if false
 * DO: push frame (optionally evaluate pre-condition), LOOP checks
 */

/*
 * parse_for - Parse and execute FOR statement.
 *
 * Syntax:
 * FOR var = start TO limit [STEP step]
 *
 * Behavior:
 * 1. Evaluate start, limit, and optional step (default 1).
 * 2. Set the variable to start.
 * 3. Check initial condition: if step > 0, var must be <= limit;
 * if step < 0, var must be >= limit. If false, skip to NEXT.
 * 4. Push a FRAME_FOR with variable, limit, step, and body index.
 * 5. Continue executing the loop body.
 */
static void parse_for(Lexer *lex, RuntimeState *rt, int line_num)
{
 char var_name;
 long start_val, limit_val, step_val;
 StackFrame frame;
 int skip_idx;

 /* Parse variable name */
 if (lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 var_name = lex->current.value.var_name;
 lexer_next(lex); /* consume variable */

 /* Parse = */
 if (!lexer_expect(lex, TOK_EQUALS)) return;

 /* Parse start expression */
 start_val = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /* Parse TO */
 if (!lexer_match_keyword(lex, KW_TO)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume TO */

 /* Parse limit expression */
 limit_val = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /* Parse optional STEP */
 step_val = 1; /* default step */
 if (lexer_match_keyword(lex, KW_STEP)) {
 lexer_next(lex); /* consume STEP */
 step_val = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (step_val == 0) {
 error_raise(ERR_HOW, line_num); /* zero step = infinite loop */
 return;
 }
 }

 /* Set the variable to the start value */
 runtime_set_var(rt, var_name, start_val);

 /*
 * Check initial condition: is the loop body reachable?
 * If step > 0 and start > limit, skip to NEXT.
 * If step < 0 and start < limit, skip to NEXT.
 */
 if ((step_val > 0 && start_val > limit_val) ||
 (step_val < 0 && start_val < limit_val)) {
 /* Skip to matching NEXT */
 skip_idx = runtime_find_matching(rt, rt->current_index,
 (int)KW_FOR, (int)KW_NEXT,
 line_num);
 if (skip_idx < 0) return; /* ERR_HOW already raised */
 rt->next_index = skip_idx + 1; /* skip past NEXT */
 lexer_skip_to_end(lex);
 return;
 }

 /* Push FOR frame - body starts at the next line */
 frame.type = FRAME_FOR;
 frame.data.for_loop.var_name = var_name;
 frame.data.for_loop.limit = limit_val;
 frame.data.for_loop.step = step_val;
 frame.data.for_loop.body_index = rt->current_index + 1;

 if (runtime_push(rt, &frame) != 0) {
 return; /* ERR_SORRY already raised */
 }

 /* Continue to loop body (next line) */
}

/*
 * parse_next - Parse and execute NEXT statement.
 *
 * Syntax:
 * NEXT var
 * NEXT (some dialects allow omitting the variable)
 *
 * Behavior:
 * 1. Pop the top FRAME_FOR from the stack (peek, don't remove yet).
 * 2. Verify the variable matches (if specified).
 * 3. Increment the variable by step.
 * 4. Check termination: if step > 0 and var > limit, or
 * step < 0 and var < limit, pop the frame and continue.
 * 5. Otherwise, jump back to body_index.
 */
static void parse_next(Lexer *lex, RuntimeState *rt, int line_num)
{
 char var_name = '\0';
 StackFrame *top;
 long val;

 /* Optional variable name */
 if (lex->current.type == TOK_VARIABLE) {
 var_name = lex->current.value.var_name;
 lexer_next(lex); /* consume variable */
 }

 /* Check stack for matching FOR frame */
 if (rt->stack_top <= 0) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 top = &rt->stack[rt->stack_top - 1];

 if (top->type != FRAME_FOR) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Verify variable matches (if specified) */
 if (var_name != '\0' && var_name != top->data.for_loop.var_name) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Increment the loop variable */
 val = runtime_get_var(rt, top->data.for_loop.var_name);
 val += top->data.for_loop.step;
 runtime_set_var(rt, top->data.for_loop.var_name, val);

 /* Check termination condition */
 if (top->data.for_loop.step > 0) {
 if (val > top->data.for_loop.limit) {
 /* Loop done - pop frame, continue after NEXT */
 rt->stack_top--;
 return;
 }
 } else {
 if (val < top->data.for_loop.limit) {
 /* Loop done - pop frame, continue after NEXT */
 rt->stack_top--;
 return;
 }
 }

 /* Loop continues - jump back to body */
 rt->next_index = top->data.for_loop.body_index;
 lexer_skip_to_end(lex);
}

/*
 * parse_while - Parse and execute WHILE statement.
 *
 * Syntax:
 * WHILE expression relop expression
 *
 * Behavior:
 * 1. Evaluate the condition (two expressions with relop).
 * 2. If true: push FRAME_WHILE and continue to loop body.
 * 3. If false: skip forward to matching WEND.
 *
 * On subsequent iterations, WEND jumps back to this WHILE line
 * to re-evaluate the condition.
 */
static void parse_while(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue cond;
 int condition;
 StackFrame frame;
 int wend_idx;

 /*
 * Evaluate the WHILE condition as a single BValue expression.
 * parse_expression_bval handles comparisons internally
 * (e.g., K$ <> "END" returns -1 or 0).
 * A non-zero result is true; zero is false.
 */
 cond = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;

 condition = (bval_to_int(&cond) != 0);

 if (condition) {
 /* Condition true - push frame and enter loop body */
 frame.type = FRAME_WHILE;
 frame.data.while_loop.loop_index = rt->current_index;

 if (runtime_push(rt, &frame) != 0) {
 return; /* ERR_SORRY already raised */
 }
 /* Continue to next line (loop body) */
 } else {
 /* Condition false - skip to matching WEND + 1 */
 wend_idx = runtime_find_matching(rt, rt->current_index,
 (int)KW_WHILE, (int)KW_WEND,
 line_num);
 if (wend_idx < 0) return;
 rt->next_index = wend_idx + 1; /* skip past WEND */
 lexer_skip_to_end(lex);
 }
}

/*
 * parse_wend - Parse and execute WEND statement.
 *
 * Behavior:
 * Pop the FRAME_WHILE from the stack and jump back to the
 * WHILE line to re-evaluate the condition.
 *
 * Note: we pop the frame before jumping so that WHILE can
 * push a fresh frame if the condition is still true. This
 * ensures each iteration gets its own push/pop cycle.
 */
static void parse_wend(Lexer *lex, RuntimeState *rt, int line_num)
{
 StackFrame frame;

 if (runtime_pop(rt, FRAME_WHILE, &frame) != 0) {
 return; /* ERR_HOW: no matching WHILE */
 }

 /* Jump back to the WHILE line to re-evaluate condition */
 rt->next_index = frame.data.while_loop.loop_index;
 lexer_skip_to_end(lex);

 (void)line_num;
}

/*
 * parse_do - Parse and execute DO statement.
 *
 * Syntax:
 * DO (unconditional entry, post-check)
 * DO WHILE expr relop expr (pre-check, WHILE condition)
 * DO UNTIL expr relop expr (pre-check, UNTIL condition)
 *
 * The matching LOOP statement closes the loop:
 * LOOP (unconditional repeat if no pre-check)
 * LOOP WHILE expr relop expr (post-check, WHILE condition)
 * LOOP UNTIL expr relop expr (post-check, UNTIL condition)
 */
static void parse_do(Lexer *lex, RuntimeState *rt, int line_num)
{
 StackFrame frame;
 int is_until = 0;
 int is_pre = 0;

 frame.type = FRAME_DO;
 frame.data.do_loop.body_index = rt->current_index + 1;

 /* Check for optional WHILE or UNTIL keyword */
 if (lexer_match_keyword(lex, KW_WHILE)) {
 long left, right;
 TokenType relop;
 int condition;
 int loop_idx;

 is_pre = 1;
 lexer_next(lex); /* consume WHILE */

 /* Evaluate condition */
 left = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 relop = lex->current.type;
 if (relop != TOK_EQUALS && relop != TOK_LT && relop != TOK_GT &&
 relop != TOK_LT_EQ && relop != TOK_GT_EQ &&
 relop != TOK_NOT_EQ && relop != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 right = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 condition = 0;
 switch (relop) {
 case TOK_EQUALS: condition = (left == right); break;
 case TOK_LT: condition = (left < right); break;
 case TOK_GT: condition = (left > right); break;
 case TOK_LT_EQ: condition = (left <= right); break;
 case TOK_GT_EQ: condition = (left >= right); break;
 case TOK_NOT_EQ: condition = (left != right); break;
 case TOK_HASH: condition = (left != right); break;
 default: break;
 }

 if (!condition) {
 /* Pre-check failed - skip to matching LOOP + 1 */
 loop_idx = runtime_find_matching(rt, rt->current_index,
 (int)KW_DO, (int)KW_LOOP,
 line_num);
 if (loop_idx < 0) return;
 rt->next_index = loop_idx + 1;
 lexer_skip_to_end(lex);
 return;
 }
 } else if (lexer_match_keyword(lex, KW_UNTIL)) {
 long left, right;
 TokenType relop;
 int condition;
 int loop_idx;

 is_pre = 1;
 is_until = 1;
 lexer_next(lex); /* consume UNTIL */

 /* Evaluate condition */
 left = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 relop = lex->current.type;
 if (relop != TOK_EQUALS && relop != TOK_LT && relop != TOK_GT &&
 relop != TOK_LT_EQ && relop != TOK_GT_EQ &&
 relop != TOK_NOT_EQ && relop != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 right = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 condition = 0;
 switch (relop) {
 case TOK_EQUALS: condition = (left == right); break;
 case TOK_LT: condition = (left < right); break;
 case TOK_GT: condition = (left > right); break;
 case TOK_LT_EQ: condition = (left <= right); break;
 case TOK_GT_EQ: condition = (left >= right); break;
 case TOK_NOT_EQ: condition = (left != right); break;
 case TOK_HASH: condition = (left != right); break;
 default: break;
 }

 /* UNTIL: loop while condition is NOT true */
 if (condition) {
 /* Condition already true - skip loop entirely */
 loop_idx = runtime_find_matching(rt, rt->current_index,
 (int)KW_DO, (int)KW_LOOP,
 line_num);
 if (loop_idx < 0) return;
 rt->next_index = loop_idx + 1;
 lexer_skip_to_end(lex);
 return;
 }
 }
 /* else: bare DO - unconditional entry, post-check at LOOP */

 frame.data.do_loop.is_until = is_until;
 frame.data.do_loop.is_pre = is_pre;

 if (runtime_push(rt, &frame) != 0) {
 return; /* ERR_SORRY already raised */
 }
 /* Continue to loop body (next line) */
}

/*
 * parse_loop - Parse and execute LOOP statement.
 *
 * Syntax:
 * LOOP (loop back unconditionally if no pre-check)
 * LOOP WHILE expr relop expr (post-check WHILE)
 * LOOP UNTIL expr relop expr (post-check UNTIL)
 *
 * Behavior:
 * Pop the FRAME_DO and evaluate post-condition if present.
 * If continuing, jump back to body_index (the DO line is
 * re-entered - but since we jump to body_index which is
 * DO+1, we re-push the frame by jumping to the DO line).
 *
 * Actually: we jump back to the DO line (body_index - 1)
 * so that pre-check DO WHILE/UNTIL re-evaluates. For bare
 * DO, we just re-push and enter body.
 */
static void parse_loop(Lexer *lex, RuntimeState *rt, int line_num)
{
 StackFrame frame;
 int do_index;

 if (runtime_pop(rt, FRAME_DO, &frame) != 0) {
 return; /* ERR_HOW: no matching DO */
 }

 do_index = frame.data.do_loop.body_index - 1;

 /* Check for post-check condition */
 if (lexer_match_keyword(lex, KW_WHILE)) {
 long left, right;
 TokenType relop;
 int condition;

 lexer_next(lex); /* consume WHILE */

 left = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 relop = lex->current.type;
 if (relop != TOK_EQUALS && relop != TOK_LT && relop != TOK_GT &&
 relop != TOK_LT_EQ && relop != TOK_GT_EQ &&
 relop != TOK_NOT_EQ && relop != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 right = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 condition = 0;
 switch (relop) {
 case TOK_EQUALS: condition = (left == right); break;
 case TOK_LT: condition = (left < right); break;
 case TOK_GT: condition = (left > right); break;
 case TOK_LT_EQ: condition = (left <= right); break;
 case TOK_GT_EQ: condition = (left >= right); break;
 case TOK_NOT_EQ: condition = (left != right); break;
 case TOK_HASH: condition = (left != right); break;
 default: break;
 }

 if (condition) {
 /* Still true - loop again */
 rt->next_index = do_index;
 }
 /* else: done - continue after LOOP */
 } else if (lexer_match_keyword(lex, KW_UNTIL)) {
 long left, right;
 TokenType relop;
 int condition;

 lexer_next(lex); /* consume UNTIL */

 left = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 relop = lex->current.type;
 if (relop != TOK_EQUALS && relop != TOK_LT && relop != TOK_GT &&
 relop != TOK_LT_EQ && relop != TOK_GT_EQ &&
 relop != TOK_NOT_EQ && relop != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 right = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 condition = 0;
 switch (relop) {
 case TOK_EQUALS: condition = (left == right); break;
 case TOK_LT: condition = (left < right); break;
 case TOK_GT: condition = (left > right); break;
 case TOK_LT_EQ: condition = (left <= right); break;
 case TOK_GT_EQ: condition = (left >= right); break;
 case TOK_NOT_EQ: condition = (left != right); break;
 case TOK_HASH: condition = (left != right); break;
 default: break;
 }

 if (!condition) {
 /* UNTIL condition not yet met - loop again */
 rt->next_index = do_index;
 }
 /* else: condition met - done, continue after LOOP */
 } else {
 /* Bare LOOP - loop unconditionally (re-enter DO) */
 rt->next_index = do_index;
 }

 if (rt->next_index >= 0) {
 lexer_skip_to_end(lex);
 }

 (void)line_num;
}

/* --- Statement Handlers ---
 * DATA, READ, RESTORE, MERGE, CHAIN, DIALECT
 */

/*
 * parse_data - Handle DATA statement during execution.
 *
 * DATA statements are not executed at runtime - they are
 * scanned at RUN time by runtime_collect_data(). At execution
 * time, DATA is simply skipped (like REM).
 */
static void parse_data(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)rt;
 (void)line_num;
 lexer_skip_to_end(lex);
}

/*
 * parse_read - Parse and execute READ statement.
 *
 * Syntax: READ var [, var ...]
 *
 * Reads the next value from the DATA pool for each variable.
 */
static void parse_read(Lexer *lex, RuntimeState *rt, int line_num)
{
 while (!error_occurred()) {
 BValue val;

 if (lex->current.type == TOK_VARIABLE) {
 /* Single-letter numeric var: READ A */
 val = runtime_read_data_bval(rt, line_num);
 if (error_occurred()) return;
 runtime_set_var(rt, lex->current.value.var_name,
 bval_to_int(&val));
 lexer_next(lex);
 } else if (lex->current.type == TOK_STRING_VAR) {
 /* Single-letter string var: READ A$ */
 char svar = lex->current.value.var_name;
 val = runtime_read_data_bval(rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&val)) {
 runtime_set_string_var(rt, svar, val);
 } else {
 /* Numeric DATA into string var - convert */
 char nbuf[32];
 char *ptr;
 int nlen;
 sprintf(nbuf, "%ld", bval_to_int(&val));
 nlen = (int)strlen(nbuf);
 ptr = strpool_store(&rt->strpool,
 nbuf, nlen);
 if (ptr != NULL) {
 runtime_set_string_var(rt, svar,
 bval_string(ptr, nlen));
 }
 }
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR) {
 /* Named numeric var: READ Score */
 const char *name = lex->current.str_start;
 int name_len = lex->current.str_length;
 val = runtime_read_data_bval(rt, line_num);
 if (error_occurred()) return;
 runtime_set_named_var_bval(rt, name, name_len,
 val);
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume comma */
 } else {
 break;
 }
 }
}

/*
 * parse_restore - Execute RESTORE statement.
 *
 * Resets the DATA pointer to the beginning.
 */
static void parse_restore(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)lex;
 (void)line_num;
 runtime_restore_data(rt);
}

/*
 * parse_merge_cmd - Parse MERGE "filename"
 */
static void parse_merge_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char filename[MAX_LINE_LENGTH + 1];

 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 fileio_merge(&rt->memory->program, filename);
}

/*
 * parse_chain_cmd - Parse CHAIN "filename"
 *
 * Loads the file and triggers execution (like RUN after LOAD).
 */
static void parse_chain_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char filename[MAX_LINE_LENGTH + 1];

 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 if (fileio_chain(&rt->memory->program, filename) == 0) {
 /* Trigger execution of the loaded program */
 exec_run(rt);
 }
}

/*
 * parse_dialect_cmd - Parse DIALECT command.
 *
 * Syntax:
 * DIALECT (list available dialects)
 * DIALECT "name" (switch by name or short code)
 * DIALECT number (switch by ID)
 *
 * Calls dialect_apply() after switching to reconfigure
 * the function registry and runtime for the new dialect.
 */
static void parse_dialect_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)rt;

 if (lex->current.type == TOK_EOF || lex->current.type == TOK_CR) {
 /* No argument - list all dialects */
 dialect_list_all();
 return;
 }

 if (lex->current.type == TOK_NUMBER) {
 /* Switch by numeric ID */
 int id = (int)lex->current.value.num_value;
 lexer_next(lex);
 if (id < 0 || id >= DIALECT_COUNT) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 dialect_init((DialectId)id);
 dialect_apply();
 printf("Dialect: %s [%s]\n",
 dialect_get_name(), dialect_get_short_name());
 return;
 }

 if (lex->current.type == TOK_STRING) {
 /* Switch by name or short code (substring match) */
 char name[MAX_LINE_LENGTH + 1];
 int found;

 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(name, lex->current.str_start,
 (size_t)lex->current.str_length);
 name[lex->current.str_length] = '\0';
 lexer_next(lex);

 found = dialect_find_by_name(name);
 if (found < 0) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 dialect_init((DialectId)found);
 dialect_apply();
 printf("Dialect: %s [%s]\n",
 dialect_get_name(), dialect_get_short_name());
 return;
 }
 /*
 * Handle DIALECT LIST (LIST is a keyword, not a string).
 * Also handle named-var form in extended-var dialects.
 */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_LIST) {
 lexer_next(lex);
 dialect_list_all();
 return;
 }
 if (lex->current.type == TOK_NAMED_VAR) {
 /* Try as dialect name (e.g. DIALECT GWBS) */
 char name[MAX_LINE_LENGTH + 1];
 int found;
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(name, lex->current.str_start,
 (size_t)lex->current.str_length);
 name[lex->current.str_length] = '\0';
 lexer_next(lex);
 found = dialect_find_by_name(name);
 if (found < 0) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 dialect_init((DialectId)found);
 dialect_apply();
 printf("Dialect: %s [%s]\n",
 dialect_get_name(),
 dialect_get_short_name());
 return;
 }

 error_raise(ERR_WHAT, line_num);
}

/* --- DEF FN - User-Defined Functions ---
 *
 * SYNTAX:
 * DEF FNA(X) = X*X+1
 * DEF FNA(X,Y) = X*Y+1
 *
 * SEMANTICS:
 * 1. Parses function name (single letter after FN prefix)
 * 2. Parses parameter list (single-letter variables in parens)
 * 3. Expects '=' followed by an expression
 * 4. Stores the ENTIRE expression text for later evaluation
 *
 * INVOCATION:
 * FNA(5) -> saves X, sets X=5, evaluates body, restores X
 *
 * WHY TEXT-BASED:
 * Classic BASIC stored DEF FN bodies as text and re-parsed
 * them at each invocation. This approach:
 * - Matches historical behavior exactly
 * - Requires no AST/bytecode storage
 * - Supports all expression features naturally
 * - Is memory-efficient (just a string per function)
 */

/*
 * parse_def_fn - Parse a DEF FN statement.
 *
 * DEF FN<name>(<params>) = <expression>
 *
 * Stores the function definition in the runtime's user_funcs table.
 */
static void parse_def_fn(Lexer *lex, RuntimeState *rt, int line_num)
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
 lexer_skip_to_end(lex);
 return;
 }
 if (lex->current.type == TOK_VARIABLE &&
 (lex->current.value.var_name == 'U' ||
 lex->current.value.var_name == 'u')) {
 lexer_skip_to_end(lex);
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
static BValue eval_user_fn(Lexer *outer_lex, RuntimeState *rt,
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

 /* Bind params */
 for (i = 0; i < argc && i < sd->param_count;
 i++) {
 set_param_by_name(rt, sd->params[i],
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
static int mat_get_array_name(Lexer *lex, char *name, int *name_len)
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

/*
 * mat_match_ident - Check if current token matches a specific
 * identifier string (case-insensitive). Used for ZER, CON, IDN,
 * TRN, INV which are not keywords but context-specific identifiers.
 */
static int mat_match_ident(Lexer *lex, const char *target)
{
 const char *src;
 int len, i;

 if (lex->current.type == TOK_NAMED_VAR) {
 src = lex->current.str_start;
 len = lex->current.str_length;
 } else if (lex->current.type == TOK_VARIABLE) {
 /* Single-letter - only matches single-letter targets */
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

/*
 * parse_mat_cmd - Parse and execute a MAT statement.
 *
 * This is the main dispatcher for all MAT operations.
 * The MAT keyword has already been consumed by the statement
 * dispatcher.
 */
static void parse_mat_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char name_a[MAX_VAR_NAME_LEN + 1];
 int name_a_len;

 /* MAT requires a dialect with DIM array support */
 if (!dialect_get_config()->has_dim_arrays) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /* MAT PRINT <name> */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_PRINT) {
 DimArray *arr;
 int r, c;

 lexer_next(lex); /* consume PRINT */
 if (!mat_get_array_name(lex, name_a, &name_a_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 arr = runtime_find_dim(rt, name_a, name_a_len);
 if (arr == NULL || arr->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Print matrix in row-major format, 1-based */
 for (r = 1; r < arr->size[0]; r++) {
 for (c = 1; c < arr->size[1]; c++) {
 int off = r * arr->size[1] + c;
 BValue v = arr->elements[off];
 if (v.type == VAL_FLOAT) {
 printf("%G", v.v.fval);
 } else {
 printf("%6ld", bval_to_int(&v));
 }
 if (c < arr->size[1] - 1) printf("\t");
 }
 printf("\n");
 }
 return;
 }

 /* MAT READ <name> */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_READ) {
 DimArray *arr;
 int r, c;

 lexer_next(lex); /* consume READ */
 if (!mat_get_array_name(lex, name_a, &name_a_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 arr = runtime_find_dim(rt, name_a, name_a_len);
 if (arr == NULL || arr->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Read values from DATA into matrix, 1-based */
 for (r = 1; r < arr->size[0]; r++) {
 for (c = 1; c < arr->size[1]; c++) {
 int off = r * arr->size[1] + c;
 arr->elements[off] = runtime_read_data_bval(
 rt, line_num);
 if (error_occurred()) return;
 }
 }
 return;
 }

 /* MAT <name> = ... */
 if (!mat_get_array_name(lex, name_a, &name_a_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 if (lex->current.type != TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume '=' */

 /* --- MAT A = ZER / CON / IDN --- */
 if (mat_match_ident(lex, "ZER")) {
 DimArray *arr = runtime_find_dim(rt, name_a, name_a_len);
 int i;
 if (arr == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 for (i = 0; i < arr->total; i++) {
 arr->elements[i] = bval_int(0);
 }
 lexer_next(lex);
 return;
 }

 if (mat_match_ident(lex, "CON")) {
 DimArray *arr = runtime_find_dim(rt, name_a, name_a_len);
 int r, c;
 if (arr == NULL || arr->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 for (r = 1; r < arr->size[0]; r++) {
 for (c = 1; c < arr->size[1]; c++) {
 arr->elements[r * arr->size[1] + c] = bval_int(1);
 }
 }
 lexer_next(lex);
 return;
 }

 if (mat_match_ident(lex, "IDN")) {
 DimArray *arr = runtime_find_dim(rt, name_a, name_a_len);
 int r, c;
 if (arr == NULL || arr->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 /* rows and cols must match for identity */
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

 /* --- MAT A = TRN(B) --- */
 if (mat_match_ident(lex, "TRN")) {
 char name_b[MAX_VAR_NAME_LEN + 1];
 int name_b_len;
 DimArray *a, *b;
 int r, c;

 lexer_next(lex); /* consume TRN */
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 if (!mat_get_array_name(lex, name_b, &name_b_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;

 b = runtime_find_dim(rt, name_b, name_b_len);
 if (b == NULL || b->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* A must be DIMmed with transposed dimensions */
 a = runtime_find_dim(rt, name_a, name_a_len);
 if (a == NULL || a->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 if (a->size[0] != b->size[1] || a->size[1] != b->size[0]) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* A(r,c) = B(c,r) - using 1-based indexing */
 for (r = 1; r < a->size[0]; r++) {
 for (c = 1; c < a->size[1]; c++) {
 int a_off = r * a->size[1] + c;
 int b_off = c * b->size[1] + r;
 a->elements[a_off] = b->elements[b_off];
 }
 }
 return;
 }

 /* --- MAT A = INV(B) --- */
 if (mat_match_ident(lex, "INV")) {
 char name_b[MAX_VAR_NAME_LEN + 1];
 int name_b_len;
 DimArray *a, *b;
 int n, r, c, p;
 double work[16][32]; /* max 15x15 matrix for inverse */
 double pivot, factor;

 lexer_next(lex); /* consume INV */
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 if (!mat_get_array_name(lex, name_b, &name_b_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;

 b = runtime_find_dim(rt, name_b, name_b_len);
 if (b == NULL || b->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 /* Must be square */
 if (b->size[0] != b->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 n = b->size[0] - 1; /* 1-based size */
 if (n > 15 || n < 1) {
 error_raise(ERR_SORRY, line_num); /* too large */
 return;
 }

 a = runtime_find_dim(rt, name_a, name_a_len);
 if (a == NULL || a->dims != 2 ||
 a->size[0] != b->size[0] ||
 a->size[1] != b->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Build augmented matrix [B | I] */
 for (r = 0; r < n; r++) {
 for (c = 0; c < n; c++) {
 BValue v = b->elements[(r+1) * b->size[1] + (c+1)];
 work[r][c] = bval_to_float(&v);
 work[r][c + n] = (r == c) ? 1.0 : 0.0;
 }
 }

 /* Gauss-Jordan elimination */
 for (p = 0; p < n; p++) {
 /* Find pivot */
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

 /* Swap rows if needed */
 if (max_row != p) {
 for (c = 0; c < 2 * n; c++) {
 double tmp = work[p][c];
 work[p][c] = work[max_row][c];
 work[max_row][c] = tmp;
 }
 }

 pivot = work[p][p];
 if (pivot > -1e-12 && pivot < 1e-12) {
 /* Singular matrix */
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Scale pivot row */
 for (c = 0; c < 2 * n; c++) {
 work[p][c] /= pivot;
 }

 /* Eliminate column */
 for (r = 0; r < n; r++) {
 if (r == p) continue;
 factor = work[r][p];
 for (c = 0; c < 2 * n; c++) {
 work[r][c] -= factor * work[p][c];
 }
 }
 }

 /* Extract inverse from right half */
 for (r = 0; r < n; r++) {
 for (c = 0; c < n; c++) {
 a->elements[(r+1) * a->size[1] + (c+1)] =
 bval_float(work[r][c + n]);
 }
 }
 return;
 }

 /* --- MAT A = (k) * B - scalar multiplication --- */
 if (lex->current.type == TOK_LPAREN) {
 BValue scalar;
 double k;
 char name_b[MAX_VAR_NAME_LEN + 1];
 int name_b_len;
 DimArray *a, *b;
 int r, c;

 lexer_next(lex); /* consume '(' */
 scalar = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 k = bval_to_float(&scalar);
 if (!lexer_expect(lex, TOK_RPAREN)) return;

 /* Expect '*' */
 if (lex->current.type != TOK_STAR) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume '*' */

 if (!mat_get_array_name(lex, name_b, &name_b_len)) {
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

 /* --- MAT A = B, MAT A = B + C, MAT A = B - C, MAT A = B * C --- */
 {
 char name_b[MAX_VAR_NAME_LEN + 1];
 int name_b_len;
 DimArray *a, *b;

 if (!mat_get_array_name(lex, name_b, &name_b_len)) {
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

 /* Check for operator: +, -, * */
 if (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 /* MAT A = B + C or MAT A = B - C */
 TokenType op = lex->current.type;
 char name_c[MAX_VAR_NAME_LEN + 1];
 int name_c_len;
 DimArray *cc;
 int r, c_idx;

 lexer_next(lex); /* consume +/- */
 if (!mat_get_array_name(lex, name_c, &name_c_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 cc = runtime_find_dim(rt, name_c, name_c_len);
 if (cc == NULL || cc->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Dimensions must match */
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
 /* MAT A = B * C */
 char name_c[MAX_VAR_NAME_LEN + 1];
 int name_c_len;
 DimArray *cc;
 int r, c_idx, k;

 lexer_next(lex); /* consume '*' */
 if (!mat_get_array_name(lex, name_c, &name_c_len)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 cc = runtime_find_dim(rt, name_c, name_c_len);
 if (cc == NULL || cc->dims != 2) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* B cols must match C rows */
 if (b->size[1] != cc->size[0]) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 /* A must be B rows x C cols */
 if (a->size[0] != b->size[0] ||
 a->size[1] != cc->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Matrix multiply: A(r,c) = sum B(r,k)*C(k,c) */
 for (r = 1; r < a->size[0]; r++) {
 for (c_idx = 1; c_idx < a->size[1]; c_idx++) {
 double sum = 0.0;
 for (k = 1; k < b->size[1]; k++) {
 double bv = bval_to_float(
 &b->elements[
 r * b->size[1] + k]);
 double cv = bval_to_float(
 &cc->elements[
 k * cc->size[1] + c_idx]);
 sum += bv * cv;
 }
 a->elements[r * a->size[1] + c_idx] =
 bval_float(sum);
 }
 }
 return;
 }

 /* MAT A = B (copy) */
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

/* --- File I/O - OPEN / CLOSE ---
 *
 * SYNTAX:
 * OPEN "filename" FOR INPUT AS #n
 * OPEN "filename" FOR OUTPUT AS #n
 * OPEN "filename" FOR APPEND AS #n
 * CLOSE #n
 *
 * The FOR keyword is detected as TOK_KEYWORD KW_FOR (reused from
 * loops). INPUT/OUTPUT/APPEND are detected as identifiers.
 */

static void parse_open(Lexer *lex, RuntimeState *rt, int line_num)
{
 char filename[MAX_LINE_LENGTH + 1];
 int mode = 0;
 int chan;

 (void)rt;

 /*
 * Detect ECMA-116 form vs GW-BASIC form:
 * ECMA-116: OPEN #n: NAME "file", ACCESS mode, ...
 * GW-BASIC: OPEN "file" FOR mode AS #n
 */
 if (lex->current.type == TOK_HASH) {
 /*
 * ===== ECMA-116 OPEN form =====
 * OPEN #expr: NAME "file"
 * [, ACCESS INPUT|OUTPUT|OUTIN]
 * [, ORGANIZATION SEQUENTIAL|RELATIVE|STREAM]
 * [, RECTYPE DISPLAY|INTERNAL]
 */
 int e116_org = FORG_SEQUENTIAL;
 int e116_rec = FREC_DISPLAY;
 int rec_len = 128;

 lexer_next(lex); /* consume # */
 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /* Expect : after channel */
 if (lex->current.type != TOK_COLON) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume : */

 /* Expect NAME keyword (as named var) */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NAME) {
 lexer_next(lex); /* consume NAME */
 } else if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 4) {
 const char *s = lex->current.str_start;
 if ((s[0]=='N'||s[0]=='n') &&
 (s[1]=='A'||s[1]=='a') &&
 (s[2]=='M'||s[2]=='m') &&
 (s[3]=='E'||s[3]=='e')) {
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /* Parse filename string */
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 /* Default mode: INPUT for sequential */
 mode = FCHAN_INPUT;

 /* Parse optional comma-separated attributes */
 while (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume , */

 /*
 * Check for ACCESS keyword
 */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ACCESS) {
 lexer_next(lex); /* consume ACCESS */
 /* Parse INPUT|OUTPUT|OUTIN */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_INPUT) {
 mode = FCHAN_INPUT;
 lexer_next(lex);
 } else if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *s =
 lex->current.str_start;
 int sn = lex->current.str_length;
 if (sn == 6 &&
 (s[0]=='O'||s[0]=='o') &&
 (s[1]=='U'||s[1]=='u') &&
 (s[2]=='T'||s[2]=='t') &&
 (s[3]=='P'||s[3]=='p') &&
 (s[4]=='U'||s[4]=='u') &&
 (s[5]=='T'||s[5]=='t')) {
 mode = FCHAN_OUTPUT;
 lexer_next(lex);
 } else if (sn == 5 &&
 (s[0]=='O'||s[0]=='o') &&
 (s[1]=='U'||s[1]=='u') &&
 (s[2]=='T'||s[2]=='t') &&
 (s[3]=='I'||s[3]=='i') &&
 (s[4]=='N'||s[4]=='n')) {
 mode = FCHAN_RANDOM; /* OUTIN=r/w */
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }
 /*
 * Check for ORGANIZATION (named var)
 */
 else if (lex->current.type == TOK_NAMED_VAR) {
 const char *s =
 lex->current.str_start;
 int sn = lex->current.str_length;

 /* ORGANIZATION */
 if (sn == 12 &&
 (s[0]=='O'||s[0]=='o') &&
 (s[1]=='R'||s[1]=='r') &&
 (s[2]=='G'||s[2]=='g') &&
 (s[3]=='A'||s[3]=='a') &&
 (s[4]=='N'||s[4]=='n') &&
 (s[5]=='I'||s[5]=='i') &&
 (s[6]=='Z'||s[6]=='z') &&
 (s[7]=='A'||s[7]=='a') &&
 (s[8]=='T'||s[8]=='t') &&
 (s[9]=='I'||s[9]=='i') &&
 (s[10]=='O'||s[10]=='o') &&
 (s[11]=='N'||s[11]=='n')) {
 lexer_next(lex); /* consume */
 /* Parse org type */
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *t =
 lex->current.str_start;
 int tn =
 lex->current.str_length;
 if (tn == 10 &&
 (t[0]=='S'||t[0]=='s') &&
 (t[1]=='E'||t[1]=='e') &&
 (t[2]=='Q'||t[2]=='q') &&
 (t[3]=='U'||t[3]=='u') &&
 (t[4]=='E'||t[4]=='e') &&
 (t[5]=='N'||t[5]=='n') &&
 (t[6]=='T'||t[6]=='t') &&
 (t[7]=='I'||t[7]=='i') &&
 (t[8]=='A'||t[8]=='a') &&
 (t[9]=='L'||t[9]=='l')) {
 e116_org = FORG_SEQUENTIAL;
 lexer_next(lex);
 } else if (tn == 8 &&
 (t[0]=='R'||t[0]=='r') &&
 (t[1]=='E'||t[1]=='e') &&
 (t[2]=='L'||t[2]=='l') &&
 (t[3]=='A'||t[3]=='a') &&
 (t[4]=='T'||t[4]=='t') &&
 (t[5]=='I'||t[5]=='i') &&
 (t[6]=='V'||t[6]=='v') &&
 (t[7]=='E'||t[7]=='e')) {
 e116_org = FORG_RELATIVE;
 mode = FCHAN_RANDOM;
 lexer_next(lex);
 } else if (tn == 6 &&
 (t[0]=='S'||t[0]=='s') &&
 (t[1]=='T'||t[1]=='t') &&
 (t[2]=='R'||t[2]=='r') &&
 (t[3]=='E'||t[3]=='e') &&
 (t[4]=='A'||t[4]=='a') &&
 (t[5]=='M'||t[5]=='m')) {
 e116_org = FORG_STREAM;
 mode = FCHAN_BINARY;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }
 /* RECTYPE */
 else if (sn == 7 &&
 (s[0]=='R'||s[0]=='r') &&
 (s[1]=='E'||s[1]=='e') &&
 (s[2]=='C'||s[2]=='c') &&
 (s[3]=='T'||s[3]=='t') &&
 (s[4]=='Y'||s[4]=='y') &&
 (s[5]=='P'||s[5]=='p') &&
 (s[6]=='E'||s[6]=='e')) {
 lexer_next(lex); /* consume */
 /* Parse rec type */
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *t =
 lex->current.str_start;
 int tn =
 lex->current.str_length;
 if (tn == 7 &&
 (t[0]=='D'||t[0]=='d') &&
 (t[1]=='I'||t[1]=='i') &&
 (t[2]=='S'||t[2]=='s') &&
 (t[3]=='P'||t[3]=='p') &&
 (t[4]=='L'||t[4]=='l') &&
 (t[5]=='A'||t[5]=='a') &&
 (t[6]=='Y'||t[6]=='y')) {
 e116_rec = FREC_DISPLAY;
 lexer_next(lex);
 } else if (tn == 8 &&
 (t[0]=='I'||t[0]=='i') &&
 (t[1]=='N'||t[1]=='n') &&
 (t[2]=='T'||t[2]=='t') &&
 (t[3]=='E'||t[3]=='e') &&
 (t[4]=='R'||t[4]=='r') &&
 (t[5]=='N'||t[5]=='n') &&
 (t[6]=='A'||t[6]=='a') &&
 (t[7]=='L'||t[7]=='l')) {
 e116_rec = FREC_INTERNAL;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }
 else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }
 else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }

 /* Open the file using the parsed attributes */
 if (e116_org == FORG_RELATIVE) {
 fileio_open_random(chan, filename,
 rec_len, line_num);
 } else if (e116_org == FORG_STREAM) {
 fileio_open_binary(chan, filename,
 line_num);
 } else {
 fileio_open(chan, filename, mode,
 line_num);
 }
 /* Store ECMA-116 metadata if open succeeded */
 if (!error_occurred()) {
 fileio_set_e116_metadata(chan,
 e116_org, e116_rec);
 }
 return;
 }

 /* ===== GW-BASIC OPEN form ===== */
 /* OPEN "filename" FOR mode AS #n */

 /* Parse filename string */
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 /*
 * ===== Old-style GW-BASIC short OPEN form =====
 * OPEN "R", channel, "filename" [, reclen]
 * OPEN "I", channel, "filename"
 * OPEN "O", channel, "filename"
 * OPEN "A", channel, "filename"
 *
 * Detection: if the first string is exactly 1 char
 * (R/I/O/A) and the next token is a comma, this is
 * the old short-form syntax.
 */
 if (strlen(filename) == 1 &&
 lex->current.type == TOK_COMMA) {
 char mode_ch = filename[0];
 int rec_len = 128;

 if (mode_ch >= 'a' && mode_ch <= 'z')
 mode_ch = (char)(mode_ch - 32);

 /* Map mode character */
 if (mode_ch == 'R') {
 mode = FCHAN_RANDOM;
 } else if (mode_ch == 'I') {
 mode = FCHAN_INPUT;
 } else if (mode_ch == 'O') {
 mode = FCHAN_OUTPUT;
 } else if (mode_ch == 'A') {
 mode = FCHAN_APPEND;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 lexer_next(lex); /* consume comma after mode */

 /* Parse channel number (no #) */
 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /* Expect comma before filename */
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume comma */

 /* Parse filename string */
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 /* Optional record length: , reclen */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume comma */
 rec_len = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 }

 /* Open the file */
 if (mode == FCHAN_RANDOM) {
 fileio_open_random(chan, filename,
 rec_len, line_num);
 } else if (mode == FCHAN_BINARY) {
 fileio_open_binary(chan, filename,
 line_num);
 } else {
 fileio_open(chan, filename, mode,
 line_num);
 }
 return;
 }

 /* Expect FOR keyword */
 if (lex->current.type != TOK_KEYWORD ||
 lex->current.value.keyword != KW_FOR) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume FOR */

 /* Parse mode: INPUT, OUTPUT, APPEND, RANDOM, BINARY */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_INPUT) {
 mode = FCHAN_INPUT;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_VARIABLE) {
 /*
 * OUTPUT, APPEND, RANDOM, BINARY are not keywords
 * - they appear as named variables. Match by text.
 */
 const char *src;
 int len;
 if (lex->current.type == TOK_NAMED_VAR) {
 src = lex->current.str_start;
 len = lex->current.str_length;
 } else {
 src = NULL;
 len = 0;
 }
 if (len == 6 && (src[0] == 'O' || src[0] == 'o') &&
 (src[1] == 'U' || src[1] == 'u') &&
 (src[2] == 'T' || src[2] == 't') &&
 (src[3] == 'P' || src[3] == 'p') &&
 (src[4] == 'U' || src[4] == 'u') &&
 (src[5] == 'T' || src[5] == 't')) {
 mode = FCHAN_OUTPUT;
 lexer_next(lex);
 } else if (len == 6 && (src[0] == 'A' || src[0] == 'a') &&
 (src[1] == 'P' || src[1] == 'p') &&
 (src[2] == 'P' || src[2] == 'p') &&
 (src[3] == 'E' || src[3] == 'e') &&
 (src[4] == 'N' || src[4] == 'n') &&
 (src[5] == 'D' || src[5] == 'd')) {
 mode = FCHAN_APPEND;
 lexer_next(lex);
 } else if (len == 6 && (src[0] == 'R' || src[0] == 'r') &&
 (src[1] == 'A' || src[1] == 'a') &&
 (src[2] == 'N' || src[2] == 'n') &&
 (src[3] == 'D' || src[3] == 'd') &&
 (src[4] == 'O' || src[4] == 'o') &&
 (src[5] == 'M' || src[5] == 'm')) {
 mode = FCHAN_RANDOM;
 lexer_next(lex);
 } else if (len == 6 && (src[0] == 'B' || src[0] == 'b') &&
 (src[1] == 'I' || src[1] == 'i') &&
 (src[2] == 'N' || src[2] == 'n') &&
 (src[3] == 'A' || src[3] == 'a') &&
 (src[4] == 'R' || src[4] == 'r') &&
 (src[5] == 'Y' || src[5] == 'y')) {
 mode = FCHAN_BINARY;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /*
 * Optional ACCESS clause:
 * ACCESS READ | WRITE | READ WRITE
 * Accepted and silently consumed for compatibility.
 */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ACCESS) {
 lexer_next(lex); /* consume ACCESS */
 /* Skip READ/WRITE/READ WRITE */
 while (lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_VARIABLE) {
 lexer_next(lex);
 }
 }

 /* Expect AS keyword */
 if (lex->current.type != TOK_KEYWORD ||
 lex->current.value.keyword != KW_AS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume AS */

 /* Expect # */
 if (lex->current.type != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume # */

 /* Parse channel number */
 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /*
 * Optional LEN = n (record length for RANDOM)
 */
 {
 int rec_len = 128;
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 3) {
 const char *s = lex->current.str_start;
 if ((s[0]=='L'||s[0]=='l') &&
 (s[1]=='E'||s[1]=='e') &&
 (s[2]=='N'||s[2]=='n')) {
 lexer_next(lex); /* consume LEN */
 if (lex->current.type == TOK_EQUALS)
 lexer_next(lex);
 rec_len = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 }

 if (mode == FCHAN_RANDOM) {
 fileio_open_random(chan, filename,
 rec_len, line_num);
 } else if (mode == FCHAN_BINARY) {
 fileio_open_binary(chan, filename,
 line_num);
 } else {
 fileio_open(chan, filename, mode,
 line_num);
 }
 }
}

/*
 * parse_close - Parse CLOSE statement.
 * The CLOSE keyword has already been consumed.
 */
static void parse_close(Lexer *lex, RuntimeState *rt, int line_num)
{
 int chan;

 /*
 * CLOSE forms:
 * CLOSE - close all files
 * CLOSE #n - close file #n
 * CLOSE n - close file n (old GW-BASIC style)
 * CLOSE #n, #m - close multiple files
 */

 /* CLOSE with no args = close all open files */
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 lex->current.type == TOK_COLON) {
 int ci;
 for (ci = 1; ci <= 8; ci++) {
 if (fileio_get_fp(ci) != NULL) {
 fileio_close(ci, line_num);
 }
 }
 return;
 }

 do {
 /* Optional # */
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex); /* consume # */
 }

 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 fileio_close(chan, line_num);
 if (error_occurred()) return;

 /* Check for comma -> more channels */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 } else {
 break;
 }
 } while (1);
}

/* --- ECMA-116 Enhanced Files: SET / ASK / REWRITE / ERASE ---
 */

/*
 * parse_set_file - Parse SET # statement.
 * SET #n: POINTER BEGIN | END | expr
 * The SET keyword has already been consumed.
 */
static void parse_set_file(Lexer *lex, RuntimeState *rt,
 int line_num)
{
 int chan;
 long pos;

 /* Expect # */
 if (lex->current.type != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /* Expect : */
 if (lex->current.type != TOK_COLON) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 /* Expect POINTER keyword */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_POINTER) {
 lexer_next(lex); /* consume POINTER */

 /* Parse BEGIN, END, or expression */
 if (lex->current.type == TOK_NAMED_VAR) {
 const char *s = lex->current.str_start;
 int sn = lex->current.str_length;
 if (sn == 5 &&
 (s[0]=='B'||s[0]=='b') &&
 (s[1]=='E'||s[1]=='e') &&
 (s[2]=='G'||s[2]=='g') &&
 (s[3]=='I'||s[3]=='i') &&
 (s[4]=='N'||s[4]=='n')) {
 pos = 0; /* BEGIN */
 lexer_next(lex);
 } else {
 /* Could be END or an expression */
 pos = (long)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_END) {
 pos = -1; /* END */
 lexer_next(lex);
 } else {
 pos = (long)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }

 fileio_set_pointer(chan, pos, line_num);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
}

/*
 * parse_ask_file - Parse ASK # statement.
 * ASK #n: POINTER var | FILESIZE var
 * The ASK keyword has already been consumed.
 */
static void parse_ask_file(Lexer *lex, RuntimeState *rt,
 int line_num)
{
 int chan;

 /* Expect # */
 if (lex->current.type != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /* Expect : */
 if (lex->current.type != TOK_COLON) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 /* POINTER or FILESIZE */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_POINTER) {
 long pos;
 lexer_next(lex); /* consume POINTER */

 pos = fileio_ask_pointer(chan, line_num);
 if (error_occurred()) return;

 /* Assign to variable */
 if (lex->current.type == TOK_VARIABLE) {
 int vi = lex->current.value.var_name - 'A';
 rt->variables[vi] = bval_float((double)pos);
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR &&
 dialect_get_config()->has_extended_vars) {
 runtime_set_named_var_bval(rt,
 lex->current.str_start,
 lex->current.str_length,
 bval_float((double)pos));
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_FILESIZE) {
 long sz;
 lexer_next(lex); /* consume FILESIZE */

 sz = fileio_ask_filesize(chan, line_num);
 if (error_occurred()) return;

 /* Assign to variable */
 if (lex->current.type == TOK_VARIABLE) {
 int vi = lex->current.value.var_name - 'A';
 rt->variables[vi] = bval_float((double)sz);
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR &&
 dialect_get_config()->has_extended_vars) {
 runtime_set_named_var_bval(rt,
 lex->current.str_start,
 lex->current.str_length,
 bval_float((double)sz));
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 }
}

/*
 * parse_rewrite - Parse REWRITE # statement.
 * REWRITE #n: expr [, expr ...]
 * The REWRITE keyword has already been consumed.
 */
static void parse_rewrite(Lexer *lex, RuntimeState *rt,
 int line_num)
{
 int chan;
 char buf[MAX_RECORD_LEN];
 int pos = 0;

 /* Expect # */
 if (lex->current.type != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 /* Expect : */
 if (lex->current.type != TOK_COLON) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 /* Build record from expressions */
 memset(buf, ' ', MAX_RECORD_LEN);
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 BValue val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&val)) {
 int sl = val.v.sval.length;
 if (sl > 0 && pos + sl <= MAX_RECORD_LEN) {
 memcpy(buf + pos, val.v.sval.data,
 (size_t)sl);
 pos += sl;
 }
 } else {
 char nb[32];
 int nl;
 sprintf(nb, "%g", bval_to_float(&val));
 nl = (int)strlen(nb);
 if (pos + nl <= MAX_RECORD_LEN) {
 memcpy(buf + pos, nb, (size_t)nl);
 pos += nl;
 }
 }
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 else
 break;
 }

 fileio_rewrite_record(chan, buf, pos, line_num);
}

/*
 * parse_erase_file - Parse ERASE # statement.
 * ERASE #n
 * This is separate from the existing ERASE (clear arrays).
 * The ERASE keyword has already been consumed.
 */
static void parse_erase_file(Lexer *lex, RuntimeState *rt,
 int line_num)
{
 int chan;

 /* # means it's a file ERASE, not array ERASE */
 lexer_next(lex); /* consume # */
 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 fileio_erase_channel(chan, line_num);
}

/* --- Statement Dispatcher ---
 */

/*
 * parse_statement - Parse and execute a single BASIC statement.
 *
 * Identifies the statement type from the current token and
 * dispatches to the appropriate handler. If the current token
 * is a variable and LET is optional (in PATB), treats it as
 * a bare assignment.
 *
 * This function handles one statement only. The caller
 * (parser_execute_line) handles the ';' separator for
 * multi-statement lines.
 */
static void parse_statement(Lexer *lex, RuntimeState *rt, int line_num)
{
 if (error_occurred()) return;

 /* Keyword-based dispatch */
 if (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;
 lexer_next(lex); /* consume keyword */

 /*
 * Strict mode gate: reject keywords that don't
 * belong to the active dialect's bitmask.
 * In union mode this always passes.
 */
 if (!dialect_keyword_allowed(kw)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 switch (kw) {
 case KW_PRINT:
 parse_print(lex, rt, line_num);
 return;
 case KW_LET:
 parse_let(lex, rt, line_num, 1);
 return;
 case KW_INPUT:
 parse_input(lex, rt, line_num);
 return;
 case KW_IF:
 parse_if(lex, rt, line_num);
 return;
 case KW_GOTO:
 parse_goto(lex, rt, line_num);
 return;
 case KW_GOSUB:
 parse_gosub(lex, rt, line_num);
 return;
 case KW_RETURN:
 parse_return(lex, rt, line_num);
 return;
 case KW_END:
 parse_end(lex, rt, line_num);
 return;
 case KW_STOP:
 parse_stop(lex, rt, line_num);
 return;
 case KW_REM:
 parse_rem(lex, rt, line_num);
 return;
 case KW_LIST:
 parse_list_cmd(lex, rt, line_num);
 return;
 case KW_RUN:
 parse_run_cmd(lex, rt, line_num);
 return;
 case KW_NEW:
 parse_new_cmd(lex, rt, line_num);
 return;
 case KW_SAVE:
 if (security_check(SECOP_FILE_WRITE, line_num))
 return;
 parse_save_cmd(lex, rt, line_num);
 return;
 case KW_LOAD:
 if (security_check(SECOP_FILE_READ, line_num))
 return;
 parse_load_cmd(lex, rt, line_num);
 return;
 case KW_FOR:
 parse_for(lex, rt, line_num);
 return;
 case KW_NEXT:
 parse_next(lex, rt, line_num);
 return;
 case KW_WHILE:
 parse_while(lex, rt, line_num);
 return;
 case KW_WEND:
 parse_wend(lex, rt, line_num);
 return;
 case KW_DO:
 parse_do(lex, rt, line_num);
 return;
 case KW_LOOP:
 parse_loop(lex, rt, line_num);
 return;
 case KW_DATA:
 parse_data(lex, rt, line_num);
 return;
 case KW_READ:
 parse_read(lex, rt, line_num);
 return;
 case KW_RESTORE:
 parse_restore(lex, rt, line_num);
 return;
 case KW_MERGE:
 if (security_check(SECOP_FILE_READ, line_num))
 return;
 parse_merge_cmd(lex, rt, line_num);
 return;
 case KW_CHAIN:
 if (security_check(SECOP_CHAIN, line_num))
 return;
 parse_chain_cmd(lex, rt, line_num);
 return;
 case KW_DIALECT:
 parse_dialect_cmd(lex, rt, line_num);
 return;
 /* DEF FN - user-defined functions */
 case KW_DEF:
 parse_def_fn(lex, rt, line_num);
 return;
 /* MAT - matrix operations */
 case KW_MAT:
 parse_mat_cmd(lex, rt, line_num);
 return;
 /* File I/O channels */
 case KW_OPEN:
 if (security_check(SECOP_FILE_WRITE, line_num))
 return;
 parse_open(lex, rt, line_num);
 return;
 case KW_CLOSE:
 parse_close(lex, rt, line_num);
 return;
 /* ECMA-116 Enhanced Files */
 case KW_SET:
 parse_set_file(lex, rt, line_num);
 return;
 case KW_ASK:
 parse_ask_file(lex, rt, line_num);
 return;
 case KW_REWRITE:
 if (security_check(SECOP_FILE_WRITE,
 line_num))
 return;
 parse_rewrite(lex, rt, line_num);
 return;
 case KW_DIM:
 parse_dim(lex, rt, line_num);
 return;
 case KW_COMPILE:
 if (security_check(SECOP_COMPILE, line_num))
 return;
 {
 char fname[MAX_LINE_LENGTH + 1];
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(fname, lex->current.str_start,
 (size_t)lex->current.str_length);
 fname[lex->current.str_length] = '\0';
 lexer_next(lex);
 compiler_compile(rt->program, fname);
 return;
 }
 /* ===== Bytecode commands ===== */
 case KW_BSAVE:
 if (security_check(SECOP_FILE_WRITE, line_num))
 return;
 {
 char fname[MAX_LINE_LENGTH + 1];
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(fname, lex->current.str_start,
 (size_t)lex->current.str_length);
 fname[lex->current.str_length] = '\0';
 lexer_next(lex);
 bpp_save(rt->program, fname);
 return;
 }

 case KW_BLOAD:
 if (security_check(SECOP_FILE_READ, line_num))
 return;
 {
 char fname[MAX_LINE_LENGTH + 1];
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(fname, lex->current.str_start,
 (size_t)lex->current.str_length);
 fname[lex->current.str_length] = '\0';
 lexer_next(lex);
 bpp_load(&rt->memory->program, fname);
 return;
 }

 /* ===== Module system ===== */
 case KW_MODULE:
 if (lex->current.type == TOK_STRING) {
 /*
 * MODULE "name" - Activate a module.
 */
 char mname[MAX_LINE_LENGTH + 1];
 if (security_check(SECOP_MODULE, line_num))
 return;
 if (lex->current.str_length >=
 MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(mname, lex->current.str_start,
 (size_t)lex->current.str_length);
 mname[lex->current.str_length] = '\0';
 lexer_next(lex);
 module_activate(mname, rt);
 } else {
 /*
 * MODULE (no args) - List modules.
 */
 int mi, mc;
 mc = module_count();
 if (mc == 0) {
 printf("No modules registered.\n");
 } else {
 printf("%-16s %-10s %-6s %s\n",
 "Module", "Class",
 "Caps", "Status");
 for (mi = 0; mi < mc; mi++) {
 const ModuleInfo *m;
 char caps[12];
 m = module_get(mi);
 if (!m) continue;
 module_caps_string(
 m->capabilities,
 caps, (int)sizeof(caps));
 printf("%-16s %-10s %-6s %s\n",
 m->name,
 module_class_name(
 m->mod_class),
 caps,
 module_is_loaded(mi)
 ? "ACTIVE"
 : "INACTIVE");
 }
 }
 }
 return;

 /* ===== Security ===== */
 case KW_SECURITY:
 if (lex->current.type == TOK_STRING) {
 /*
 * SECURITY "level" - Set security level.
 */
 char lname[MAX_LINE_LENGTH + 1];
 int si;
 if (lex->current.str_length >=
 MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(lname, lex->current.str_start,
 (size_t)lex->current.str_length);
 lname[lex->current.str_length] = '\0';
 lexer_next(lex);
 /* Match level name */
 for (si = 0; si < SEC_COUNT; si++) {
 if (str_case_equal(lname,
 security_level_name(
 (SecLevel)si))) {
 security_set_level(
 (SecLevel)si);
 printf("Security: %s\n",
 security_level_name(
 (SecLevel)si));
 return;
 }
 }
 printf("Unknown level '%s'. "
 "Use OPEN, STANDARD, "
 "or RESTRICTED.\n", lname);
 } else {
 /*
 * SECURITY (no args) - Show level.
 */
 SecLevel lv = security_get_level();
 printf("Security: %s",
 security_level_name(lv));
 switch (lv) {
 case SEC_OPEN:
 printf(" (no restrictions)");
 break;
 case SEC_STANDARD:
 printf(" (file I/O only)");
 break;
 case SEC_RESTRICTED:
 printf(" (math/string only)");
 break;
 default: break;
 }
 printf("\n");
 }
 return;

 /* ===== Cross-platform ===== */
 case KW_SYSTEM:
 if (lex->current.type == TOK_STRING) {
 /*
 * SYSTEM "query" - Specific info query.
 */
 char qname[MAX_LINE_LENGTH + 1];
 if (lex->current.str_length >=
 MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(qname, lex->current.str_start,
 (size_t)lex->current.str_length);
 qname[lex->current.str_length] = '\0';
 lexer_next(lex);
 if (str_case_equal(qname, "PLATFORM")) {
 printf("%s (%s)\n",
 platform_name(),
 platform_short_name());
 } else if (str_case_equal(qname,
 "VERSION")) {
 printf("%s %s\n",
 BASICPP_NAME,
 BASICPP_VERSION);
 } else if (str_case_equal(qname,
 "MEMORY")) {
 platform_print_memory(rt);
 } else if (str_case_equal(qname,
 "COMPILER")) {
 const PlatformInfo *pi;
 pi = platform_get_info();
 printf("%s %s\n",
 pi->compiler,
 pi->compiler_ver);
 } else if (str_case_equal(qname,
 "WORDSIZE")) {
 printf("%d-bit\n",
 platform_word_size());
 } else {
 printf("Unknown query '%s'. "
 "Use PLATFORM, VERSION, "
 "MEMORY, COMPILER, "
 "or WORDSIZE.\n", qname);
 }
 } else {
 /*
 * SYSTEM (no args) - Full summary.
 */
 platform_print_info();
 }
 return;

 /* ===== Interactive debugger ===== */
 case KW_BREAK:
 {
 /*
 * BREAK N - Set breakpoint at line N
 * BREAK -N - Clear breakpoint at line N
 * BREAK - List all breakpoints
 */
 if (lex->current.type == TOK_NUMBER ||
 lex->current.type == TOK_MINUS) {
 int negative = 0;
 int bp_line;

 if (lex->current.type == TOK_MINUS) {
 negative = 1;
 lexer_next(lex);
 }

 if (lex->current.type != TOK_NUMBER) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 bp_line = (int)lex->current.value.num_value;
 lexer_next(lex);

 if (negative) {
 if (runtime_breakpoint_remove(
 rt, bp_line) == 0) {
 printf("Breakpoint cleared"
 " at line %d.\n",
 bp_line);
 } else {
 printf("No breakpoint"
 " at line %d.\n",
 bp_line);
 }
 } else {
 runtime_breakpoint_add(rt, bp_line);
 printf("Breakpoint set"
 " at line %d.\n", bp_line);
 }
 } else {
 runtime_breakpoint_list(rt);
 }
 return;
 }

 case KW_CONT:
 /*
 * CONT - Continue from paused state.
 */
 exec_cont(rt);
 return;

 case KW_VARS:
 {
 /*
 * VARS - Dump all non-zero variables.
 */
 int v;
 int printed = 0;

 /* Numeric variables A-Z */
 for (v = 0; v < MAX_VARIABLES; v++) {
 if (rt->variables[v].type ==
 VAL_INTEGER &&
 rt->variables[v].v.ival != 0) {
 printf("%c=%ld ",
 'A' + v,
 rt->variables[v].v.ival);
 printed = 1;
 } else if (rt->variables[v].type ==
 VAL_FLOAT &&
 rt->variables[v].v.fval
 != 0.0) {
 printf("%c=%g ",
 'A' + v,
 rt->variables[v].v.fval);
 printed = 1;
 }
 }

 /* String variables A$-Z$ */
 for (v = 0; v < MAX_STRING_VARS; v++) {
 if (rt->string_vars[v].type ==
 VAL_STRING &&
 rt->string_vars[v].v.sval.data !=
 NULL &&
 rt->string_vars[v].v.sval.length
 > 0) {
 printf("%c$=\"%.*s\" ",
 'A' + v,
 rt->string_vars[v].v.sval
 .length,
 rt->string_vars[v].v.sval
 .data);
 printed = 1;
 }
 }

 if (printed) {
 printf("\n");
 } else {
 printf("All variables clear.\n");
 }
 return;
 }

 /* ===== Self-test framework ===== */
 case KW_ASSERT:
 {
 /*
 * ASSERT condition
 *
 * Evaluates the condition as a full BValue
 * expression (comparisons, logical ops, etc.
 * are handled internally by parse_expression_bval).
 * Non-zero = pass, zero = fail.
 */
 BValue cond;
 int condition;

 cond = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;

 condition = (bval_to_int(&cond) != 0);

 rt->test_total++;
 if (condition) {
 rt->test_pass++;
 } else {
 rt->test_fail++;
 printf("FAIL at line %d\n", line_num);
 }
 return;
 }

 case KW_TEST:
 {
 /*
 * TEST "name" - Start a named test block.
 */
 if (lex->current.type == TOK_STRING) {
 int len = lex->current.str_length;
 if (len > 63) len = 63;
 memcpy(rt->test_name,
 lex->current.str_start,
 (size_t)len);
 rt->test_name[len] = '\0';
 lexer_next(lex);
 } else {
 strcpy(rt->test_name, "Unnamed");
 }
 rt->in_test = 1;
 /* Reset block counters */
 rt->test_pass = 0;
 rt->test_fail = 0;
 rt->test_total = 0;
 printf("--- %s ---\n", rt->test_name);
 return;
 }

 case KW_ENDTEST:
 /*
 * ENDTEST - End test block, report results.
 */
 printf("%s: %d passed, %d failed\n",
 rt->test_name,
 rt->test_pass, rt->test_fail);
 rt->in_test = 0;
 return;

 case KW_SELFTEST:
 /*
 * SELFTEST - Run built-in validation suite.
 */
 selftest_run(rt);
 return;

 /* ===== Help & introspection ===== */
 case KW_HELP:
 {
 /*
 * HELP - Show command summary
 * HELP "keyword" - Show help for keyword
 * HELP RENUM - bare keyword works
 * HELP ABS - function name works
 * HELP FPRINT - compound word works
 *
 * STRATEGY:
 * 1. If next token is TOK_STRING, use
 * the quoted value directly.
 * 2. Otherwise, scan the raw source for
 * the remaining word(s) on the line.
 * This avoids issues where the lexer
 * splits compound names like FPRINT
 * into TOK_VARIABLE('F') + KW_PRINT.
 */
 char topic[64];
 topic[0] = '\0';

 if (lex->current.type == TOK_STRING) {
 /* Quoted: HELP "PRINT" */
 int len = lex->current.str_length;
 if (len > 63) len = 63;
 memcpy(topic,
 lex->current.str_start,
 (size_t)len);
 topic[len] = '\0';
 lexer_next(lex);
 } else if (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 /*
 * Grab the remaining text on the line
 * as a raw string. Take the first
 * contiguous word (letters and $ only).
 */
 int tlen = 0;

 /*
 * Back up: lex->pos points past the
 * current token. The token itself
 * starts earlier. Use the source text
 * starting from where the current
 * token's first character is.
 *
 * For keyword tokens, the start is at
 * lex->pos minus the keyword length.
 * For variables, it's pos-1.
 * Simplest: scan backwards to find the
 * start of the current word.
 */
 {
 const char *src = lex->source;
 int spos = lex->pos;
 int wstart;
 int wend;
 char c;

 /* Back up to start of current
 * token */
 if (spos > 0) spos--;
 while (spos > 0 &&
 ((src[spos-1] >= 'A' &&
 src[spos-1] <= 'Z') ||
 (src[spos-1] >= 'a' &&
 src[spos-1] <= 'z'))) {
 spos--;
 }
 wstart = spos;

 /* Find end of word (letters + $) */
 wend = wstart;
 c = src[wend];
 while (c != '\0' &&
 ((c >= 'A' && c <= 'Z') ||
 (c >= 'a' && c <= 'z') ||
 c == '$')) {
 wend++;
 c = src[wend];
 }

 tlen = wend - wstart;
 if (tlen > 63) tlen = 63;
 memcpy(topic, src + wstart,
 (size_t)tlen);
 topic[tlen] = '\0';
 }

 /* Uppercase the topic */
 {
 int ti;
 for (ti = 0; topic[ti]; ti++) {
 if (topic[ti] >= 'a' &&
 topic[ti] <= 'z') {
 topic[ti] -= 32;
 }
 }
 }

 /* Skip past all remaining tokens */
 lexer_skip_to_end(lex);
 }

 help_show(topic[0] ? topic : NULL);
 return;
 }

 case KW_INFO:
 /*
 * INFO - System information.
 */
 help_info(rt);
 return;

 case KW_CATALOG:
 /*
 * CATALOG - List registered functions.
 */
 help_catalog();
 return;

 /* ===== Final polish ===== */
 case KW_RENUM:
 {
 /*
 * RENUM [start[,step]]
 * Renumber all program lines and fix
 * GOTO/GOSUB line references.
 * Default: start=10, step=10.
 */
 int start_num = 10;
 int step_num = 10;
 int i, count;
 int *old_nums;
 int *new_nums;

 /* Parse optional arguments */
 if (lex->current.type == TOK_NUMBER) {
 start_num = (int)lex->current
 .value.num_value;
 lexer_next(lex);
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 if (lex->current.type == TOK_NUMBER) {
 step_num = (int)lex->current
 .value.num_value;
 lexer_next(lex);
 }
 }
 }

 count = rt->program->count;
 if (count == 0) {
 printf("No program.\n");
 return;
 }

 /* Build old->new mapping */
 old_nums = (int *)malloc(
 (size_t)count * sizeof(int));
 new_nums = (int *)malloc(
 (size_t)count * sizeof(int));
 if (!old_nums || !new_nums) {
 free(old_nums);
 free(new_nums);
 error_raise(ERR_SORRY, line_num);
 return;
 }

 for (i = 0; i < count; i++) {
 old_nums[i] = rt->program->lines[i]
 .line_number;
 new_nums[i] = start_num + i * step_num;
 }

 /*
 * Fix GOTO/GOSUB references in each line.
 * Scan line text for GOTO/GOSUB/RESTORE
 * followed by a number - replace the number
 * with the new line number.
 */
 for (i = 0; i < count; i++) {
 char *txt = rt->program->lines[i].text;
 char buf[MAX_LINE_LENGTH + 1];
 char *dst = buf;
 const char *src = txt;
 int new_line = new_nums[i];

 /* Write new line number */
 dst += sprintf(dst, "%d", new_line);

 /* Skip old line number */
 while (*src >= '0' && *src <= '9') src++;

 /* Copy rest, fixing GOTO/GOSUB targets */
 while (*src) {
 /* Check for GOTO/GOSUB/RESTORE/THEN
 * followed by space and number */
 int is_jump = 0;
 if (strncmp(src, "GOTO ", 5) == 0) {
 is_jump = 5;
 } else if (strncmp(src, "GOSUB ",
 6) == 0) {
 is_jump = 6;
 } else if (strncmp(src, "THEN ",
 5) == 0) {
 is_jump = 5;
 } else if (strncmp(src, "RESTORE ",
 8) == 0) {
 is_jump = 8;
 }

 if (is_jump > 0) {
 /* Copy keyword */
 memcpy(dst, src, (size_t)is_jump);
 dst += is_jump;
 src += is_jump;

 /* Parse old target number */
 while (*src) {
 int old_target = 0;
 int digits = 0;
 const char *ns = src;

 while (*ns >= '0' &&
 *ns <= '9') {
 old_target = old_target
 * 10 + (*ns - '0');
 ns++;
 digits++;
 }

 if (digits > 0) {
 /* Look up in mapping */
 int j;
 int mapped = old_target;
 for (j = 0; j < count;
 j++) {
 if (old_nums[j] ==
 old_target) {
 mapped =
 new_nums[j];
 break;
 }
 }
 dst += sprintf(dst, "%d",
 mapped);
 src = ns;

 /* Handle comma-separated
 * lists (ON...GOTO) */
 if (*src == ',') {
 *dst++ = *src++;
 continue;
 }
 break;
 } else {
 break;
 }
 }
 } else {
 *dst++ = *src++;
 }
 }
 *dst = '\0';

 /* Update the line */
 rt->program->lines[i].line_number =
 new_line;
 strcpy(rt->program->lines[i].text, buf);
 }

 free(old_nums);
 free(new_nums);
 printf("Renumbered %d lines"
 " (%d,%d).\n",
 count, start_num, step_num);
 return;
 }

 case KW_DELETE:
 {
 /*
 * DELETE from-to
 * Delete all lines in range [from, to].
 */
 int from_line, to_line;
 int deleted = 0;
 int i;

 if (lex->current.type != TOK_NUMBER) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 from_line = (int)lex->current
 .value.num_value;
 lexer_next(lex);

 to_line = from_line;
 if (lex->current.type == TOK_MINUS) {
 lexer_next(lex);
 if (lex->current.type != TOK_NUMBER) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 to_line = (int)lex->current
 .value.num_value;
 lexer_next(lex);
 }

 /* Delete lines in range (reverse order) */
 for (i = rt->program->count - 1;
 i >= 0; i--) {
 int ln = rt->program->lines[i]
 .line_number;
 if (ln >= from_line && ln <= to_line) {
 program_delete(rt->program, ln);
 deleted++;
 }
 }

 printf("%d line%s deleted.\n",
 deleted,
 deleted == 1 ? "" : "s");
 return;
 }

 case KW_VER:
 /*
 * VER - Display version information.
 */
 printf("%s %s\n",
 BASICPP_NAME, BASICPP_VERSION);
 printf("%s\n", BASICPP_COPYRIGHT);
 printf("Build: %s\n", __DATE__);
 return;

 case KW_BYE:
 /*
 * BYE - Exit the interpreter.
 *
 * Prints a farewell message and terminates the
 * process. This is the clean way to exit BASIC++
 * back to the command prompt.
 */
 printf("Goodbye.\n");
 exit(0);
 return; /* unreachable, but satisfies compilers */

 /* ===== Core commands ===== */
 case KW_CLS:
 /*
 * CLS - Clear screen.
 *
 * Routes through the virtual console device's cls
 * operation. If the device doesn't support cls,
 * this is a no-op.
 */
 vdev_cls(rt->dev_con);
 return;

 case KW_HOME:
 /*
 * HOME - Move cursor to top-left.
 *
 * Like CLS but does NOT clear screen.
 * Resets cursor position to row 1, col 1
 * and sends ANSI cursor-home escape.
 */
 rt->cursor_row = 1;
 rt->cursor_col = 1;
 printf("\033[H");
 fflush(stdout);
 return;
 case KW_CLEAR:
 /*
 * CLEAR - Reset variables, arrays, stack.
 *
 * GW-BASIC: clears all variables, string
 * space, resets the stack, and closes files.
 * Optional: CLEAR [,stack_size]
 * We ignore the optional argument.
 */
 {
 int vi;
 rt->stack_top = 0;
 for (vi = 0; vi < MAX_VARIABLES; vi++)
 rt->variables[vi] = bval_int(0);
 for (vi = 0; vi < MAX_STRING_VARS;
 vi++) {
 rt->string_vars[vi] =
 bval_string(NULL, 0);
 }
 rt->named_count = 0;
 memset(rt->named_vars, 0,
 sizeof(rt->named_vars));
 rt->dim_count = 0;
 rt->dim_elements_used = 0;
 if (rt->array_base != NULL) {
 memset(rt->array_base, 0,
 (size_t)(rt->array_size
 * (long)sizeof(long)));
 }
 strpool_reset(&rt->strpool);
 /* Consume optional ,stack_size */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 parse_expression(lex, rt,
 line_num);
 }
 }
 return;

 case KW_CLR:
 /*
 * CLR - Clear the runtime stack only.
 *
 * Resets GOSUB return addresses and FOR/WHILE/DO
 * loop frames. Does NOT clear variables, arrays,
 * or the program. Useful for recovery after an
 * error interrupts nested GOSUBs.
 *
 * Note: CLEAR does the full GW-BASIC reset
 * (variables + stack + string space). CLR is
 * the lightweight stack-only variant.
 */
 rt->stack_top = 0;
 return;

 case KW_TRON:
 /*
 * TRON - Trace ON.
 *
 * Enables line-by-line trace output. When active,
 * the interpreter prints [line_number] before
 * executing each line. Output goes through the
 * console device.
 */
 rt->trace_on = 1;
 return;

 case KW_TROFF:
 /*
 * TROFF - Trace OFF.
 *
 * Disables the trace output.
 */
 rt->trace_on = 0;
 return;

 case KW_ON:
 /*
 * ON expr GOTO l1,l2,l3 - computed GOTO
 * ON expr GOSUB l1,l2,l3 - computed GOSUB
 * ON ERROR GOTO n - error handler
 */
 {
 /*
 * Peek: if next token is ERROR, handle
 * as ON ERROR GOTO. Otherwise, parse as
 * ON expr GOTO/GOSUB line-list.
 */
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword ==
 KW_ERROR) {
 /* ON ERROR GOTO n */
 long target;
 lexer_next(lex); /* consume ERROR */
 if (lex->current.type != TOK_KEYWORD
 || lex->current.value.keyword !=
 KW_GOTO) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume GOTO */
 target = parse_expression(
 lex, rt, line_num);
 rt->on_error_line = (int)target;
 }
 /*
 * ON COM(n) / KEY(n) / PEN / PLAY(n) /
 * STRIG(n) / TIMER(n) GOSUB line
 *
 * Event trapping stubs. Accept syntax
 * for compatibility; consume arguments.
 * ON TIMER stores handler for future use.
 */
 else if (lex->current.type ==
 TOK_KEYWORD &&
 (lex->current.value.keyword
 == KW_COM ||
 lex->current.value.keyword
 == KW_KEY ||
 lex->current.value.keyword
 == KW_PEN ||
 lex->current.value.keyword
 == KW_PLAY ||
 lex->current.value.keyword
 == KW_STRIG ||
 lex->current.value.keyword
 == KW_TIMER)) {
 KeywordId evkw =
 lex->current.value.keyword;
 lexer_next(lex);

 /* Consume optional (n) */
 if (lex->current.type ==
 TOK_LPAREN) {
 lexer_next(lex);
 (void)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type ==
 TOK_RPAREN)
 lexer_next(lex);
 }

 /* Expect GOSUB */
 if (!lexer_match_keyword(lex,
 KW_GOSUB)) {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 lexer_next(lex);

 /* Parse target line */
 {
 long tgt = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 if (evkw == KW_TIMER) {
 rt->on_timer_line =
 (int)tgt;
 }
 /* Other events: accepted,
 * no hardware to trap */
 (void)tgt;
 }
 } else {
 /*
 * ON expr GOTO/GOSUB line-list.
 * Evaluate expr, then parse the
 * GOTO/GOSUB keyword, then collect
 * comma-separated line numbers.
 * Branch to the Nth target.
 */
 long selector;
 int is_gosub = 0;
 int targets[20];
 int count = 0;

 selector = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 /* Expect GOTO or GOSUB */
 if (lex->current.type == TOK_KEYWORD
 && lex->current.value.keyword ==
 KW_GOTO) {
 is_gosub = 0;
 } else if (lex->current.type ==
 TOK_KEYWORD &&
 lex->current.value
 .keyword ==
 KW_GOSUB) {
 is_gosub = 1;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 /* Parse line number list */
 targets[count] = (int)
 parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 count++;

 while (lex->current.type ==
 TOK_COMMA && count < 20) {
 lexer_next(lex);
 targets[count] = (int)
 parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 count++;
 }

 /* Branch if selector is valid */
 if (selector >= 1 &&
 selector <= count) {
 int tgt = targets[
 (int)selector - 1];
 if (is_gosub) {
 vm_call(rt, tgt, line_num);
 } else {
 vm_jump(rt, tgt, line_num);
 }
 }
 /* else: fall through to next line
 * (GW-BASIC behavior) */
 }
 }
 return;

 case KW_SWAP:
 /*
 * SWAP A,B - exchange two variable values.
 * Both must be single-letter variables.
 */
 {
 char var_a, var_b;
 BValue tmp_a, tmp_b;

 if (lex->current.type !=
 TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 var_a = lex->current.value.var_name;
 lexer_next(lex);

 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume comma */

 if (lex->current.type !=
 TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 var_b = lex->current.value.var_name;
 lexer_next(lex);

 tmp_a = runtime_get_var_bval(
 rt, var_a);
 tmp_b = runtime_get_var_bval(
 rt, var_b);
 runtime_set_var_bval(rt, var_a, tmp_b);
 runtime_set_var_bval(rt, var_b, tmp_a);
 }
 return;

 case KW_RANDOMIZE:
 /*
 * RANDOMIZE [n] - seed RNG.
 * No argument or 0 = time-based seed.
 *
 * Seeds both the custom LCG (rnd_seed,
 * used by RND) and srand() for compat.
 */
 {
 long seed = 0;
 if (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 seed = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (seed == 0) {
 unsigned long ts;
 ts = (unsigned long)time(NULL);
 rt->rnd_seed = ts & 0x7FFFFFFFUL;
 srand((unsigned int)ts);
 } else {
 rt->rnd_seed = (unsigned long)seed
 & 0x7FFFFFFFUL;
 srand((unsigned int)seed);
 }
 /* Ensure non-zero seed */
 if (rt->rnd_seed == 0)
 rt->rnd_seed = 1;
 }
 return;

 case KW_ELSE:
 /*
 * ELSE encountered during execution:
 *
 * Single-line IF: ELSE has content on same
 * line - skip rest of line (true branch ran).
 *
 * Block IF with block_if_depth > 0: true
 * branch already ran - skip to END IF.
 *
 * Block IF with block_if_depth == 0: reached
 * from false path via scan - fall through to
 * execute the ELSE body.
 */
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR) {
 /* Block ELSE */
 if (rt->block_if_depth > 0) {
 /* True branch ran - skip to END IF */
 block_if_skip_to_end(rt, line_num);
 } else {
 /* False path - enter ELSE body */
 rt->block_if_depth++;
 }
 } else {
 /* Single-line ELSE - skip rest */
 lexer_skip_to_end(lex);
 }
 return;

 case KW_ELSEIF:
 /*
 * ELSEIF encountered during execution.
 *
 * If block_if_depth > 0, we fell through from
 * a TRUE block - skip to END IF.
 *
 * If block_if_depth == 0, we were scanned to
 * from a FALSE path - evaluate the condition.
 */
 if (rt->block_if_depth > 0) {
 /* True branch already ran - skip to END IF */
 block_if_skip_to_end(rt, line_num);
 return;
 }
 {
 BValue elif_val;
 int elif_cond;

 elif_val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 elif_cond = (bval_to_int(&elif_val) != 0);

 /* Consume optional THEN */
 if (lexer_match_keyword(lex, KW_THEN))
 lexer_next(lex);

 if (elif_cond) {
 /*
 * TRUE: increment depth and let
 * execution continue into the body.
 */
 rt->block_if_depth++;
 return;
 } else {
 /*
 * FALSE: scan forward for next
 * ELSEIF/ELSE/END IF.
 */
 block_if_scan(rt, line_num);
 return;
 }
 }

 case KW_ENDIF:
 /*
 * ENDIF (single word) - end of block IF.
 * Decrement depth if inside a true block.
 */
 if (rt->block_if_depth > 0)
 rt->block_if_depth--;
 return;

 case KW_FILES:
 /*
 * FILES [pattern] - List directory.
 *
 * With no argument, lists current dir.
 * With a string argument, passes it as
 * a wildcard pattern (e.g. FILES "*.BAS")
 *
 * Uses system() for portability.
 */
 {
 char cmd[280];
 if (lex->current.type == TOK_STRING
 && lex->current.str_start != NULL
 && lex->current.str_length > 0) {
 int sl = lex->current.str_length;
 if (sl > 200) sl = 200;
#if defined(_WIN32) || defined(_WIN64) || \
 defined(__MSDOS__) || defined(__DOS__) || \
 defined(MSDOS)
 memcpy(cmd, "dir /w ", 7);
 memcpy(cmd + 7,
 lex->current.str_start,
 (size_t)sl);
 cmd[7 + sl] = '\0';
#else
 memcpy(cmd, "ls -C ", 6);
 memcpy(cmd + 6,
 lex->current.str_start,
 (size_t)sl);
 cmd[6 + sl] = '\0';
#endif
 lexer_next(lex);
 } else {
#if defined(_WIN32) || defined(_WIN64) || \
 defined(__MSDOS__) || defined(__DOS__) || \
 defined(MSDOS)
 strcpy(cmd, "dir /w");
#else
 strcpy(cmd, "ls -C");
#endif
 }
 printf("\n");
 system(cmd);
 printf("\n");
 }
 return;

 case KW_DIR:
 /*
 * DIR [pattern] - List filenames only.
 *
 * Like FILES but shows only filenames,
 * no sizes, dates, or other info.
 * Uses dir /b on Windows/DOS,
 * ls -1 on Linux/POSIX.
 */
 {
 char cmd[280];
 if (lex->current.type == TOK_STRING
 && lex->current.str_start != NULL
 && lex->current.str_length > 0) {
 int sl = lex->current.str_length;
 if (sl > 200) sl = 200;
#if defined(_WIN32) || defined(_WIN64) || \
 defined(__MSDOS__) || defined(__DOS__) || \
 defined(MSDOS)
 memcpy(cmd, "dir /b ", 7);
 memcpy(cmd + 7,
 lex->current.str_start,
 (size_t)sl);
 cmd[7 + sl] = '\0';
#else
 memcpy(cmd, "ls -1 ", 6);
 memcpy(cmd + 6,
 lex->current.str_start,
 (size_t)sl);
 cmd[6 + sl] = '\0';
#endif
 lexer_next(lex);
 } else {
#if defined(_WIN32) || defined(_WIN64) || \
 defined(__MSDOS__) || defined(__DOS__) || \
 defined(MSDOS)
 strcpy(cmd, "dir /b");
#else
 strcpy(cmd, "ls -1");
#endif
 }
 printf("\n");
 system(cmd);
 printf("\n");
 }
 return;

 case KW_BEEP:
 /*
 * BEEP - Emit audible bell.
 * BEEP ON - Enable error beep (default).
 * BEEP OFF - Disable error beep.
 *
 * Routes through vdev_beep() which uses
 * platform-specific audio:
 * - Windows: Beep(800,200) via kernel32
 * - Linux: terminal bell (BEL)
 * - FreeDOS: PC speaker (BEL)
 */
 if (lexer_match_keyword(lex, KW_ON)) {
 lexer_next(lex);
 error_set_beep(1);
 } else if (lex->current.type ==
 TOK_NAMED_VAR &&
 lex->current.str_length == 3 &&
 (lex->current.str_start[0] == 'O' ||
 lex->current.str_start[0] == 'o') &&
 (lex->current.str_start[1] == 'F' ||
 lex->current.str_start[1] == 'f') &&
 (lex->current.str_start[2] == 'F' ||
 lex->current.str_start[2] == 'f')) {
 lexer_next(lex);
 error_set_beep(0);
 } else {
 vdev_beep();
 }
 return;

 case KW_COLOR:
 /*
 * COLOR [fg[,bg]] - Set text color.
 *
 * Maps GW-BASIC color codes (0-15) to
 * ANSI SGR escape sequences. On Windows
 * this requires VT mode or Windows
 * Terminal. On DOS, requires ANSI.SYS.
 *
 * GW-BASIC palette:
 * 0=Black 1=Blue 2=Green 3=Cyan
 * 4=Red 5=Magenta 6=Brown 7=White
 * 8-15 = bright versions
 *
 * COLOR with no args resets to default.
 */
 {
 /*
 * ANSI fg codes for GW-BASIC palette.
 * Index = GW-BASIC color number.
 * Value = ANSI SGR parameter.
 */
 static const int ansi_fg[] = {
 30, 34, 32, 36, 31, 35, 33, 37,
 90, 94, 92, 96, 91, 95, 93, 97
 };
 static const int ansi_bg[] = {
 40, 44, 42, 46, 41, 45, 43, 47,
 100,104,102,106,101,105,103,107
 };
 int fg = -1, bg = -1;

 if (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 fg = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 bg = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }

 if (fg < 0 && bg < 0) {
 /* Reset to defaults */
 printf("\033[0m");
 } else {
 if (fg >= 0 && fg <= 15)
 printf("\033[%dm",
 ansi_fg[fg]);
 if (bg >= 0 && bg <= 15)
 printf("\033[%dm",
 ansi_bg[bg]);
 }
 fflush(stdout);
 }
 return;

 case KW_AUTO:
 /*
 * AUTO [start[,increment]]
 *
 * Enable auto line numbering mode.
 * Default: AUTO 10,10
 * The REPL loop picks up auto_line and
 * auto_step from the runtime state.
 * Empty input or '.' cancels AUTO mode.
 */
 {
 int start = 10;
 int step = 10;

 if (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 start = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 step = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (start < 1) start = 1;
 if (step < 1) step = 1;
 if (start > LINE_NUMBER_MAX)
 start = LINE_NUMBER_MAX;
 rt->auto_line = start;
 rt->auto_step = step;
 }
 return;

 case KW_SOUND:
 /*
 * SOUND freq, duration
 *
 * GW-BASIC: freq is Hz (37-32767).
 * duration is in clock ticks at 18.2
 * ticks per second. We convert to ms:
 * ms = ticks * 1000 / 18.2
 * = ticks * 55 (approximately)
 *
 * SOUND 0,n is a pause (silence).
 */
 {
 int freq, dur_ticks, dur_ms;
 freq = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 dur_ticks = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 dur_ms = dur_ticks * 55;
 if (freq <= 0) {
 /* Silence: just pause */
 vdev_sleep(dur_ms);
 } else {
 vdev_sound(freq, dur_ms);
 }
 }
 return;

 case KW_PLAY:
 /*
 * PLAY string$ - Music macro language.
 *
 * GW-BASIC music commands:
 * C D E F G A B - notes
 * # or + - sharp
 * - - flat
 * O0-O6 - set octave (default 4)
 * > - octave up
 * < - octave down
 * L1-L64 - set note length (default 4)
 * T32-T255 - set tempo (default 120)
 * Pn - pause for length n
 * Nn - play note by number (0-84)
 * . - dot (extend 1.5x)
 * MN/ML/MS - music normal/legato/staccato
 * (ignored - we play full length)
 */
 {
 /*
 * Note frequency table: semitone
 * frequencies for octave 4 (middle).
 * Index: 0=C 1=C# 2=D 3=D# 4=E 5=F
 * 6=F# 7=G 8=G# 9=A 10=A# 11=B
 */
 static const int note_freq4[] = {
 262, 277, 294, 311, 330, 349,
 370, 392, 415, 440, 466, 494
 };
 /*
 * Map note letter to semitone index.
 * C=0 D=2 E=4 F=5 G=7 A=9 B=11
 */
 static const int note_semi[] = {
 9, 11, 0, 2, 4, 5, 7
 }; /* A B C D E F G */

 const char *s;
 int slen, si;
 int octave = 4;
 int note_len = 4; /* quarter note */
 int tempo = 120; /* BPM */

 /* Get the music string */
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 s = lex->current.str_start;
 slen = lex->current.str_length;
 lexer_next(lex);

 /* Parse the music string */
 for (si = 0; si < slen; si++) {
 char ch;
 ch = s[si];
 /* Uppercase */
 if (ch >= 'a' && ch <= 'z')
 ch = (char)(ch - 32);

 if (ch >= 'A' && ch <= 'G') {
 /* Note */
 int semi, freq, dur;
 int this_len = note_len;
 int dotted = 0;
 semi = note_semi[ch - 'A'];

 /* Check for sharp/flat */
 if (si+1 < slen &&
 (s[si+1] == '#' ||
 s[si+1] == '+')) {
 semi++;
 if (semi > 11) semi = 0;
 si++;
 } else if (si+1 < slen &&
 s[si+1] == '-') {
 semi--;
 if (semi < 0) semi = 11;
 si++;
 }

 /* Check for length suffix */
 if (si+1 < slen &&
 s[si+1] >= '1' &&
 s[si+1] <= '9') {
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 1 && num <= 64)
 this_len = num;
 }

 /* Check for dot */
 if (si+1 < slen &&
 s[si+1] == '.') {
 dotted = 1;
 si++;
 }

 /* Calculate frequency */
 freq = note_freq4[semi];
 if (octave < 4) {
 int sh;
 for (sh = 0;
 sh < 4-octave;
 sh++)
 freq /= 2;
 } else if (octave > 4) {
 int sh;
 for (sh = 0;
 sh < octave-4;
 sh++)
 freq *= 2;
 }

 /* Calculate duration ms */
 /* quarter = 60000/tempo ms */
 dur = (240000 /
 (tempo * this_len));
 if (dotted)
 dur = dur + dur / 2;

 vdev_sound(freq, dur);

 } else if (ch == 'O') {
 /* Set octave */
 if (si+1 < slen &&
 s[si+1] >= '0' &&
 s[si+1] <= '6') {
 octave = s[si+1] - '0';
 si++;
 }
 } else if (ch == '>') {
 if (octave < 6) octave++;
 } else if (ch == '<') {
 if (octave > 0) octave--;
 } else if (ch == 'L') {
 /* Set default note length */
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 1 && num <= 64)
 note_len = num;
 } else if (ch == 'T') {
 /* Set tempo */
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 32 && num <= 255)
 tempo = num;
 } else if (ch == 'P') {
 /* Pause */
 int plen = note_len;
 int dur;
 if (si+1 < slen &&
 s[si+1] >= '1' &&
 s[si+1] <= '9') {
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 1 && num <= 64)
 plen = num;
 }
 dur = 240000 /
 (tempo * plen);
 vdev_sleep(dur);
 } else if (ch == 'N') {
 /* Note by number 0-84 */
 int num = 0;
 int freq, oct, semi, dur;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num == 0) {
 /* N0 = rest */
 dur = 240000 /
 (tempo * note_len);
 vdev_sleep(dur);
 } else if (num <= 84) {
 num--;
 oct = num / 12;
 semi = num % 12;
 freq = note_freq4[semi];
 if (oct < 4) {
 int sh;
 for (sh = 0;
 sh < 4-oct;
 sh++)
 freq /= 2;
 } else if (oct > 4) {
 int sh;
 for (sh = 0;
 sh < oct-4;
 sh++)
 freq *= 2;
 }
 dur = 240000 /
 (tempo * note_len);
 vdev_sound(freq, dur);
 }
 }
 /* M, space, etc: skip */
 }
 }
 return;

 case KW_SCREEN:
 /*
 * SCREEN mode [,color_switch]
 *
 * GW-BASIC screen modes:
 * 0 = text 80x25 (or 40x25)
 * 1 = 320x200, 4 colors
 * 2 = 640x200, 2 colors
 * 7-13 = EGA/VGA modes
 *
 * We are a text-mode interpreter, so:
 * SCREEN 0 = text mode (no-op, default)
 * SCREEN n = accepted, sets mode number
 * for compatibility but rendering is
 * text-based via DRAW character canvas.
 */
 {
 int mode = 0;
 mode = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 rt->screen_mode = mode;
 /* Reset DRAW cursor to center */
 if (mode == 1 || mode == 7) {
 /* 320x200 modes: 40x25 canvas */
 rt->draw_x = 20;
 rt->draw_y = 12;
 } else if (mode == 2 || mode == 8) {
 /* 640x200 modes: 80x25 canvas */
 rt->draw_x = 40;
 rt->draw_y = 12;
 } else {
 rt->draw_x = 40;
 rt->draw_y = 25;
 }

 /* Consume optional ,color_switch */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 parse_expression(lex, rt, line_num);
 }

 /* Activate/deactivate gfx buffer */
 if (mode > 0) {
 gfxbuf_set_active(1);
 } else {
 gfxbuf_set_active(0);
 }
 }
 return;

 case KW_WIDTH:
 /*
 * WIDTH columns [,lines]
 *
 * GW-BASIC: sets the screen width.
 * Common values: 40 or 80 columns.
 * WIDTH 80 - 80-column mode
 * WIDTH 40 - 40-column mode
 * WIDTH 80,25 - 80 cols, 25 lines
 * WIDTH - display current width
 *
 * We store the value and use it for
 * PRINT word-wrap (future) and to
 * report the current setting.
 */
 {
 int w, lines;
 char sep = dialect_get_separator();

 /* No arguments = display current */
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type ==
 TOK_COLON && sep == ':') ||
 (lex->current.type ==
 TOK_SEMICOLON &&
 sep == ';')) {
 printf("WIDTH %d,%d\n",
 rt->screen_width,
 rt->screen_lines);
 return;
 }

 w = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 if (w == 40 || w == 80) {
 rt->screen_width = w;
 } else if (w >= 1 && w <= 255) {
 rt->screen_width = w;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /* Optional ,lines */
 lines = rt->screen_lines;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 lines = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lines >= 1 && lines <= 60) {
 rt->screen_lines = lines;
 }
 }
 }
 return;

 case KW_DRAW:
 /*
 * DRAW string$ - Graphics macro language.
 *
 * GW-BASIC DRAW commands:
 * U[n] - up n pixels
 * D[n] - down n pixels
 * L[n] - left n pixels
 * R[n] - right n pixels
 * E[n] - diagonal up-right
 * F[n] - diagonal down-right
 * G[n] - diagonal down-left
 * H[n] - diagonal up-left
 * M x,y - move to (absolute or relative)
 * B - move without drawing (prefix)
 * N - return after drawing (prefix)
 * Cn - set color (pen character)
 * Sn - set scale (1-255, 4=normal)
 * An - set angle (0-3, 90-deg steps)
 *
 * Text-mode rendering:
 * We render to an 80x50 character canvas
 * using '*' (or Cn mapped character) for
 * drawn pixels. After DRAW completes,
 * output the canvas rows that have content.
 */
 {
 /* Canvas: 80 cols x 50 rows */
 #define DCANV_W 80
 #define DCANV_H 50
 static char canvas[DCANV_H][DCANV_W];

 const char *s;
 int slen, si;
 int cx, cy;
 int scale = 4; /* 4 = 1:1 */
 int angle = 0;
 int pen = '*';
 int blank_prefix = 0;
 int return_prefix = 0;
 int min_y, max_y, row, col;

 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 s = lex->current.str_start;
 slen = lex->current.str_length;
 lexer_next(lex);

 /* Clear canvas */
 memset(canvas, ' ', sizeof(canvas));

 /* Start from DRAW cursor */
 cx = rt->draw_x;
 cy = rt->draw_y;
 if (cx < 0) cx = 0;
 if (cx >= DCANV_W) cx = DCANV_W - 1;
 if (cy < 0) cy = 0;
 if (cy >= DCANV_H) cy = DCANV_H - 1;

 /* Parse draw string */
 for (si = 0; si < slen; si++) {
 char ch = s[si];
 int dx = 0, dy = 0, dist = 1;
 int save_x, save_y;
 int step;

 /* Uppercase */
 if (ch >= 'a' && ch <= 'z')
 ch = (char)(ch - 32);

 /* Prefix: B (blank move) */
 if (ch == 'B') {
 blank_prefix = 1;
 continue;
 }
 /* Prefix: N (return after) */
 if (ch == 'N') {
 return_prefix = 1;
 continue;
 }

 /* Direction commands */
 if (ch == 'U') { dx=0; dy=-1; }
 else if (ch=='D') { dx=0; dy=1; }
 else if (ch=='L') { dx=-1; dy=0; }
 else if (ch=='R') { dx=1; dy=0; }
 else if (ch=='E') { dx=1; dy=-1; }
 else if (ch=='F') { dx=1; dy=1; }
 else if (ch=='G') { dx=-1; dy=1; }
 else if (ch=='H') { dx=-1;dy=-1; }
 else if (ch == 'C') {
 /* Set color/pen character */
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 /*
 * Map GW-BASIC color to
 * ASCII pen character
 */
 if (num == 0) pen = ' ';
 else if (num <= 3) pen = ".+*"[num-1];
 else if (num <= 7) pen = "#@%&"[num-4];
 else pen = '*';
 rt->draw_color = pen;
 continue;
 }
 else if (ch == 'S') {
 /* Set scale */
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 1 && num <= 255)
 scale = num;
 continue;
 }
 else if (ch == 'A') {
 /* Set angle */
 if (si+1 < slen &&
 s[si+1] >= '0' &&
 s[si+1] <= '3') {
 angle = s[si+1] - '0';
 si++;
 }
 continue;
 }
 else if (ch == 'M') {
 /* Move to x,y */
 int mx = 0, my = 0;
 int rel = 0, neg;
 si++;
 /* Skip spaces */
 while (si < slen &&
 s[si] == ' ') si++;
 /* Check for +/- (relative) */
 neg = 0;
 if (si < slen &&
 (s[si]=='+' ||
 s[si]=='-')) {
 rel = 1;
 if (s[si] == '-') neg=1;
 si++;
 }
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 mx = mx*10 +
 (s[si] - '0');
 si++;
 }
 if (neg) mx = -mx;
 /* Expect comma */
 if (si < slen &&
 s[si] == ',') si++;
 /* Parse Y */
 neg = 0;
 if (si < slen &&
 (s[si]=='+' ||
 s[si]=='-')) {
 if (!rel) rel = 1;
 if (s[si] == '-') neg=1;
 si++;
 }
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 my = my*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (neg) my = -my;

 if (rel) {
 /* Relative move */
 cx += mx;
 cy += my;
 } else {
 cx = mx;
 cy = my;
 }
 if (cx < 0) cx = 0;
 if (cx >= DCANV_W)
 cx = DCANV_W-1;
 if (cy < 0) cy = 0;
 if (cy >= DCANV_H)
 cy = DCANV_H-1;
 if (!blank_prefix) {
 canvas[cy][cx] =
 (char)pen;
 }
 blank_prefix = 0;
 return_prefix = 0;
 continue;
 }
 else if (ch == ' ' || ch == ';') {
 continue;
 }
 else {
 continue;
 }

 /* Parse optional distance */
 if (si+1 < slen &&
 s[si+1] >= '0' &&
 s[si+1] <= '9') {
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 dist = num;
 }

 /* Apply scale: dist*scale/4 */
 dist = (dist * scale) / 4;
 if (dist < 1) dist = 1;

 /* Apply angle rotation */
 if (angle != 0) {
 int tdx = dx, tdy = dy;
 switch (angle) {
 case 1: /* 90 deg */
 dx = -tdy;
 dy = tdx;
 break;
 case 2: /* 180 deg */
 dx = -tdx;
 dy = -tdy;
 break;
 case 3: /* 270 deg */
 dx = tdy;
 dy = -tdx;
 break;
 }
 }

 /* Save position for N prefix */
 save_x = cx;
 save_y = cy;

 /* Draw line */
 for (step = 0;
 step < dist;
 step++) {
 cx += dx;
 cy += dy;
 if (cx < 0) cx = 0;
 if (cx >= DCANV_W)
 cx = DCANV_W - 1;
 if (cy < 0) cy = 0;
 if (cy >= DCANV_H)
 cy = DCANV_H - 1;
 if (!blank_prefix) {
 canvas[cy][cx] =
 (char)pen;
 }
 }

 /* N prefix: return to start */
 if (return_prefix) {
 cx = save_x;
 cy = save_y;
 }
 blank_prefix = 0;
 return_prefix = 0;
 }

 /* Update cursor position */
 rt->draw_x = cx;
 rt->draw_y = cy;

 /*
 * Render canvas: find bounding rows
 * that have non-space content and
 * print them. Trim trailing spaces.
 */
 min_y = DCANV_H;
 max_y = -1;
 for (row = 0; row < DCANV_H; row++) {
 for (col = 0;
 col < DCANV_W; col++) {
 if (canvas[row][col] != ' '){
 if (row < min_y)
 min_y = row;
 if (row > max_y)
 max_y = row;
 break;
 }
 }
 }
 if (max_y >= 0) {
 for (row = min_y;
 row <= max_y; row++) {
 int last = 0;
 for (col = DCANV_W - 1;
 col >= 0; col--) {
 if (canvas[row][col]
 != ' ') {
 last = col;
 break;
 }
 }
 for (col = 0;
 col <= last; col++) {
 putchar(
 canvas[row][col]);
 }
 putchar('\n');
 }
 }

 #undef DCANV_W
 #undef DCANV_H
 }
 return;


 case KW_SLEEP:
 /*
 * SLEEP [seconds]
 * Pause execution. Without argument, pause 1 second.
 */
 lexer_next(lex); /* consume SLEEP */
 {
 int secs = 1;
 if (lex->current.type == TOK_NUMBER ||
 lex->current.type == TOK_VARIABLE ||
 lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_LPAREN) {
 secs = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 }
 if (secs < 0) secs = 0;
 vdev_sleep((unsigned int)secs * 1000);
 }
 return;

 case KW_LOCATE:
 /*
 * LOCATE row, col
 * Position cursor using ANSI escape codes.
 * Keyword already consumed by switch entry.
 */
 {
 int row, col;
 row = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_COMMA)) return;
 col = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (row < 1) row = 1;
 if (col < 1) col = 1;
 printf("\033[%d;%dH", row, col);
 fflush(stdout);
 rt->cursor_row = row;
 rt->cursor_col = col;
 }
 return;

 case KW_LINE:
 /*
 * LINE INPUT ["prompt";] var$
 * LINE (x1,y1)-(x2,y2) [,color [,B[F]]]
 * Read entire line or draw graphics line.
 * Keyword already consumed by switch entry.
 */
 if (lexer_match_keyword(lex, KW_INPUT)) {
 lexer_next(lex); /* consume INPUT */
 {
 char buf[256];
 char svar_name = 0;
 char *ptr;
 int len;
 int file_chan = 0;

 /* LINE INPUT #n, var$ (file) */
 if (lex->current.type ==
 TOK_HASH) {
 lexer_next(lex);
 file_chan =
 (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 }

 /* Optional prompt (stdin only) */
 if (file_chan == 0 &&
 lex->current.type ==
 TOK_STRING) {
 printf("%.*s",
 lex->current.str_length,
 lex->current.str_start);
 lexer_next(lex);
 if (lex->current.type ==
 TOK_SEMICOLON) {
 lexer_next(lex);
 }
 }

 /* Parse string variable */
 if (lex->current.type ==
 TOK_STRING_VAR) {
 svar_name =
 lex->current.value
 .var_name;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }

 if (file_chan > 0) {
 /* Read from file */
 if (fileio_input_line(
 file_chan, buf,
 (int)sizeof(buf),
 line_num) != 0) {
 return;
 }
 len = (int)strlen(buf);
 } else {
 fflush(stdout);
 if (!fgets(buf, sizeof(buf),
 stdin)) {
 error_raise(ERR_HOW,
 line_num);
 return;
 }
 len = (int)strlen(buf);
 }
 /* Remove trailing newline */
 while (len > 0 &&
 (buf[len-1] == '\n' ||
 buf[len-1] == '\r')) {
 len--;
 }
 ptr = strpool_store(
 &rt->strpool, buf, len);
 if (ptr != NULL) {
 runtime_set_string_var(rt,
 svar_name,
 bval_string(ptr, len));
 }
 }
 } else if (lex->current.type == TOK_LPAREN) {
 /*
 * LINE (x1,y1)-(x2,y2) [,color [,B[F]]]
 * Graphics line/box drawing.
 */
 int x1, y1, x2, y2, clr = 15;
 int is_box = 0, is_filled = 0;

 lexer_next(lex); /* consume ( */
 x1 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 y1 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;

 /* Expect - separator */
 if (lex->current.type == TOK_MINUS)
 lexer_next(lex);

 if (!lexer_expect(lex, TOK_LPAREN))
 return;
 x2 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 y2 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;

 /* Optional color */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 clr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }

 /* Optional B or BF */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 if (lex->current.type ==
 TOK_VARIABLE &&
 lex->current.value.var_name
 == 'B') {
 is_box = 1;
 lexer_next(lex);
 /* Check for F after B */
 if (lex->current.type ==
 TOK_VARIABLE &&
 lex->current.value
 .var_name == 'F') {
 is_filled = 1;
 lexer_next(lex);
 }
 }
 }

 if (is_box)
 gfxbuf_box(x1, y1, x2, y2,
 clr, is_filled);
 else
 gfxbuf_line(x1, y1, x2, y2, clr);
 gfxbuf_render();
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;


 case KW_SELECT:
 /*
 * SELECT CASE expr
 * CASE value [, value...]
 * CASE value TO value
 * CASE IS relop value
 * CASE ELSE
 * ...
 * END SELECT
 *
 * Evaluate expr, scan through CASE clauses
 * to find match, execute that block, skip rest.
 */
 if (!lexer_match_keyword(lex, KW_CASE)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume CASE */
 {
 BValue sel_val;
 long sel_num;
 int found = 0;
 int pline, idx, depth;
 ProgramStore *pgm = rt->program;

 sel_val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 sel_num = bval_to_int(&sel_val);

 /* Scan forward for matching CASE */
 idx = rt->current_index + 1;
 depth = 0;

 while (idx < pgm->count && !found) {
 Lexer cl;
 const char *cline =
 pgm->lines[idx].text;
 pline = pgm->lines[idx].line_number;
 lexer_init(&cl, cline);
 lexer_next(&cl);

 /* Skip line number */
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);

 if (cl.current.type == TOK_KEYWORD) {
 KeywordId kk =
 cl.current.value.keyword;

 if (kk == KW_SELECT) {
 depth++;
 idx++;
 continue;
 }
 if (kk == KW_END) {
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value.keyword
 == KW_SELECT) {
 if (depth > 0) {
 depth--;
 idx++;
 continue;
 }
 /* END SELECT at our level
 - no match found */
 rt->next_index = idx + 1;
 return;
 }
 }
 if (kk == KW_CASE && depth == 0){
 lexer_next(&cl);
 /* CASE ELSE */
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value.keyword
 == KW_ELSE) {
 found = 1;
 rt->next_index = idx + 1;
 break;
 }
 /* CASE IS relop value */
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value.keyword
 == KW_IS) {
 long cv;
 TokenType rop;
 int match = 0;
 lexer_next(&cl);
 rop = cl.current.type;
 lexer_next(&cl);
 cv = parse_expression(
 &cl, rt, pline);
 if (error_occurred())
 return;
 switch (rop) {
 case TOK_LT:
 match=(sel_num<cv);
 break;
 case TOK_GT:
 match=(sel_num>cv);
 break;
 case TOK_LT_EQ:
 match=(sel_num<=cv);
 break;
 case TOK_GT_EQ:
 match=(sel_num>=cv);
 break;
 case TOK_EQUALS:
 match=(sel_num==cv);
 break;
 case TOK_NOT_EQ:
 match=(sel_num!=cv);
 break;
 default:
 break;
 }
 if (match) {
 found = 1;
 rt->next_index =
 idx + 1;
 }
 idx++;
 continue;
 }
 /* CASE val [TO val]
 [, val...] */
 for (;;) {
 long v1;
 v1 = parse_expression(
 &cl, rt, pline);
 if (error_occurred())
 return;
 /* Check TO range */
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value
 .keyword == KW_TO) {
 long v2;
 lexer_next(&cl);
 v2 =
 parse_expression(
 &cl, rt, pline);
 if (error_occurred())
 return;
 if (sel_num >= v1 &&
 sel_num <= v2) {
 found = 1;
 rt->next_index =
 idx + 1;
 }
 } else if (sel_num==v1) {
 found = 1;
 rt->next_index =
 idx + 1;
 }
 if (found) break;
 if (cl.current.type !=
 TOK_COMMA) break;
 lexer_next(&cl);
 }
 }
 }
 idx++;
 }
 if (!found) {
 /* Skip to END SELECT */
 rt->next_index = idx;
 }
 }
 return;

 case KW_CASE:
 /*
 * CASE inside executed SELECT block.
 * We've finished the matched block.
 * Skip to END SELECT.
 */
 {
 int idx = rt->current_index + 1;
 int depth = 0;
 ProgramStore *pgm = rt->program;
 while (idx < pgm->count) {
 Lexer cl;
 const char *cline =
 pgm->lines[idx].text;
 lexer_init(&cl, cline);
 lexer_next(&cl);
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD) {
 KeywordId kk =
 cl.current.value.keyword;
 if (kk == KW_SELECT) {
 depth++;
 }
 if (kk == KW_END) {
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value.keyword
 == KW_SELECT) {
 if (depth > 0) {
 depth--;
 } else {
 rt->next_index =
 idx + 1;
 return;
 }
 }
 }
 }
 idx++;
 }
 error_raise(ERR_WHAT, line_num);
 }
 return;

 case KW_EXIT:
 /*
 * EXIT FOR - jump past matching NEXT
 * EXIT DO - jump past matching LOOP
 */
 if (lexer_match_keyword(lex, KW_FOR)) {
 lexer_next(lex); /* consume FOR */
 /* Pop stack until FOR frame */
 while (rt->stack_top > 0) {
 rt->stack_top--;
 if (rt->stack[rt->stack_top].type ==
 FRAME_FOR) {
 /* Find NEXT line and skip */
 int idx=rt->current_index + 1;
 ProgramStore *pgm = rt->program;
 while (idx < pgm->count) {
 Lexer cl;
 const char *cl2 =
 pgm->lines[idx].text;
 lexer_init(&cl, cl2);
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_NUMBER)
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value.keyword
 == KW_NEXT) {
 rt->next_index = idx + 1;
 return;
 }
 idx++;
 }
 rt->next_index = pgm->count;
 return;
 }
 }
 error_raise(ERR_WHAT, line_num);
 } else if (lexer_match_keyword(lex, KW_DO)) {
 lexer_next(lex); /* consume DO */
 /* Pop stack until DO frame */
 while (rt->stack_top > 0) {
 rt->stack_top--;
 if (rt->stack[rt->stack_top].type ==
 FRAME_DO) {
 /* Find LOOP line and skip */
 int idx=rt->current_index + 1;
 ProgramStore *pgm = rt->program;
 while (idx < pgm->count) {
 Lexer cl;
 const char *cl2 =
 pgm->lines[idx].text;
 lexer_init(&cl, cl2);
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_NUMBER)
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value.keyword
 == KW_LOOP) {
 rt->next_index = idx + 1;
 return;
 }
 idx++;
 }
 rt->next_index = pgm->count;
 return;
 }
 }
 error_raise(ERR_WHAT, line_num);
 } else if (lexer_match_keyword(lex,KW_SUB)){
 lexer_next(lex); /* consume SUB */
 /* EXIT SUB: pop frame, restore vars */
 {
 StackFrame frame;
 int i;
 if (runtime_pop(rt, FRAME_SUB,
 &frame) != 0)
 {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 for (i = 0; i < MAX_VARIABLES; i++)
 rt->variables[i] =
 frame.data.sub_call
 .saved_vars[i];
 for (i = 0; i < MAX_STRING_VARS; i++)
 rt->string_vars[i] =
 frame.data.sub_call
 .saved_strvars[i];
 rt->in_sub_index = -1;
 rt->next_index =
 frame.data.sub_call.return_index;
 }
 } else if (lexer_match_keyword(lex,
 KW_FUNCTION)) {
 lexer_next(lex); /* consume FUNCTION */
 /* EXIT FUNCTION: same as EXIT SUB */
 {
 StackFrame frame;
 int i;
 if (runtime_pop(rt, FRAME_SUB,
 &frame) != 0)
 {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 for (i = 0; i < MAX_VARIABLES; i++)
 rt->variables[i] =
 frame.data.sub_call
 .saved_vars[i];
 for (i = 0; i < MAX_STRING_VARS; i++)
 rt->string_vars[i] =
 frame.data.sub_call
 .saved_strvars[i];
 rt->in_sub_index = -1;
 rt->next_index =
 frame.data.sub_call.return_index;
 }
 } else if (lex->current.type ==
 TOK_NAMED_VAR &&
 lex->current.str_length >= 7 &&
 lex->current.str_start != NULL &&
 (lex->current.str_start[0] == 'H' ||
 lex->current.str_start[0] == 'h')) {
 /* EXIT HANDLER */
 int i;
 lexer_next(lex); /* consume HANDLER */
 for (i = rt->stack_top - 1; i >= 0; i--) {
 if (rt->stack[i].type ==
 FRAME_EXCEPTION) {
 int end_idx = rt->stack[i]
 .data.exception
 .end_when_index;
 rt->stack_top = i;
 rt->next_index = end_idx;
 return;
 }
 }
 error_raise(ERR_WHAT, line_num);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;

 case KW_CONST_KW:
 /*
 * CONST name = value
 * Store named constant in runtime table.
 */
 {
 const char *nm;
 int nlen, ci;
 BValue cv;

 if (lex->current.type != TOK_KEYWORD &&
 lex->current.type != TOK_NAMED_VAR &&
 lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 nm = lex->current.str_start;
 nlen = lex->current.str_length;
 if (nm == NULL || nlen == 0) {
 /* Single letter variable */
 nm = &lex->current.value.var_name;
 nlen = 1;
 }
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_EQUALS))
 return;
 cv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;

 if (rt->const_count >= MAX_CONSTANTS) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 ci = rt->const_count++;
 if (nlen > 31) nlen = 31;
 memcpy(rt->constants[ci].name, nm,
 (size_t)nlen);
 rt->constants[ci].name[nlen] = '\0';
 rt->constants[ci].name_len = nlen;
 rt->constants[ci].value = cv;
 }
 return;

 case KW_ERASE:
 /*
 * ERASE # n - truncate file (ECMA-116)
 * ERASE arrayname [, ...] - clear DIM arrays
 */
 if (lex->current.type == TOK_HASH) {
 parse_erase_file(lex, rt, line_num);
 return;
 }
 {
 for (;;) {
 const char *nm;
 int nlen, di;
 char namebuf[MAX_VAR_NAME_LEN + 1];

 if (lex->current.type ==
 TOK_NAMED_VAR) {
 nm = lex->current.str_start;
 nlen = lex->current.str_length;
 } else if (lex->current.type ==
 TOK_VARIABLE) {
 namebuf[0] = lex->current.value
 .var_name;
 namebuf[1] = '\0';
 nm = namebuf;
 nlen = 1;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 /* Save name before advancing */
 if (nlen > MAX_VAR_NAME_LEN)
 nlen = MAX_VAR_NAME_LEN;
 memcpy(namebuf, nm, (size_t)nlen);
 namebuf[nlen] = '\0';
 nm = namebuf;
 lexer_next(lex);

 /* Find and clear the DIM array */
 for (di = 0; di < rt->dim_count;
 di++) {
 DimArray *da =
 &rt->dim_arrays[di];
 if ((int)strlen(da->name) ==
 nlen &&
 memcmp(da->name, nm,
 (size_t)nlen) == 0) {
 /* Zero out elements */
 int j;
 for (j = 0; j < da->total;
 j++) {
 da->elements[j] =
 bval_int(0);
 }
 break;
 }
 }

 if (lex->current.type != TOK_COMMA)
 break;
 lexer_next(lex);
 }
 }
 return;

 case KW_LPRINT:
 /*
 * LPRINT [expr] [; expr...] [;]
 * Print to stderr (portable printer substitute).
 */
 {
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_COLON) {
 if (lex->current.type ==
 TOK_SEMICOLON) {
 lexer_next(lex);
 continue;
 }
 if (lex->current.type == TOK_COMMA) {
 fprintf(stderr, "\t");
 lexer_next(lex);
 continue;
 }
 {
 BValue v;
 v = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&v)) {
 fprintf(stderr, "%.*s",
 v.v.sval.length,
 v.v.sval.data ?
 v.v.sval.data : "");
 } else if (v.type == VAL_FLOAT) {
 fprintf(stderr, "%g",
 bval_to_float(&v));
 } else {
 fprintf(stderr, "%ld",
 bval_to_int(&v));
 }
 }
 }
 fprintf(stderr, "\n");
 fflush(stderr);
 }
 return;

 case KW_DECLARE:
 /*
 * DECLARE [EXTERNAL] SUB name [(params)]
 * DECLARE [EXTERNAL] FUNCTION name [(params)]
 *
 * ECMA-116 / QBasic: Forward declaration.
 * Pre-registers the SUB/FUNCTION name in the
 * subs table with body_index = -1.
 * The actual definition fills in body_index.
 */
 {
 int is_func;
 const char *nm;
 int nlen;
 char namebuf[MAX_VAR_NAME_LEN + 1];
 SubDef *sd;

 /* Consume optional EXTERNAL */
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length >= 3 &&
 lex->current.str_start != NULL &&
 (lex->current.str_start[0] == 'E' ||
 lex->current.str_start[0] == 'e')) {
 lexer_next(lex); /* EXTERNAL */
 }

 /* Expect SUB or FUNCTION */
 if (lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.value.keyword == KW_SUB) {
 is_func = 0;
 } else if (lex->current.value.keyword ==
 KW_FUNCTION) {
 is_func = 1;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume SUB/FUNCTION */

 /* Parse name */
 if (lex->current.type != TOK_NAMED_VAR &&
 lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 nm = lex->current.str_start;
 nlen = lex->current.str_length;
 if (lex->current.type == TOK_VARIABLE) {
 namebuf[0] = lex->current.value
 .var_name;
 namebuf[1] = '\0';
 nm = namebuf;
 nlen = 1;
 }

 /* Skip if already declared */
 if (runtime_find_sub(rt, nm, nlen)
 != NULL) {
 lexer_skip_to_end(lex);
 return;
 }

 /* Pre-register in subs table */
 if (rt->sub_count >= MAX_SUBS) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 sd = &rt->subs[rt->sub_count];
 {
 int ci = nlen;
 int j;
 if (ci > MAX_VAR_NAME_LEN)
 ci = MAX_VAR_NAME_LEN;
 memcpy(sd->name, nm, (size_t)ci);
 sd->name[ci] = '\0';
 for (j = 0; j < ci; j++) {
 if (sd->name[j] >= 'a' &&
 sd->name[j] <= 'z')
 sd->name[j] =
 (char)(sd->name[j] - 32);
 }
 sd->name_len = ci;
 }
 sd->is_function = is_func;
 sd->param_count = 0;
 sd->body_index = -1;
 rt->sub_count++;

 /* Skip rest of line (param list etc) */
 lexer_skip_to_end(lex);
 }
 return;

 case KW_SUB:
 case KW_FUNCTION:
 /*
 * SUB Name [(param1, param2, ...)]
 * body...
 * END SUB
 *
 * FUNCTION Name [(param1, param2, ...)]
 * body...
 * END FUNCTION
 *
 * At definition time: store in sub table
 * and skip forward to END SUB/FUNCTION.
 */
 {
 int is_func = (kw == KW_FUNCTION) ? 1 : 0;
 const char *nm;
 int nlen, ci;
 char namebuf[MAX_VAR_NAME_LEN + 1];
 SubDef *sd;
 KeywordId end_kw;

 /* Parse the sub/function name */
 if (lex->current.type != TOK_NAMED_VAR &&
 lex->current.type != TOK_VARIABLE &&
 lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 nm = lex->current.str_start;
 nlen = lex->current.str_length;
 if (lex->current.type == TOK_VARIABLE) {
 namebuf[0] = lex->current.value
 .var_name;
 namebuf[1] = '\0';
 nm = namebuf;
 nlen = 1;
 } else if (nm == NULL || nlen <= 0) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /* Check for existing entry */
 {
 SubDef *existing =
 runtime_find_sub(rt, nm, nlen);
 if (existing != NULL &&
 existing->body_index >= 0) {
 /* Already fully defined - skip
 * body (allows re-run) */
 } else if (existing != NULL &&
 existing->body_index < 0) {
 /* Forward-declared by DECLARE:
 * fill in body_index + params */
 sd = existing;
 sd->is_function = is_func;
 sd->param_count = 0;
 sd->body_index =
 rt->current_index + 1;
 lexer_next(lex);

 /* Parse params for fwd decl */
 if (lex->current.type ==
 TOK_LPAREN) {
 lexer_next(lex);
 while (lex->current.type !=
 TOK_RPAREN &&
 lex->current.type !=
 TOK_EOF) {
 const char *pn;
 int pl;
 char pb[MAX_VAR_NAME_LEN+1];
 int is_str = 0;
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 pn = lex->current
 .str_start;
 pl = lex->current
 .str_length;
 } else if (
 lex->current.type ==
 TOK_VARIABLE) {
 pb[0] = lex->current
 .value
 .var_name;
 pb[1] = '\0';
 pn = pb;
 pl = 1;
 } else if (
 lex->current.type ==
 TOK_STRING_VAR) {
 pb[0] = lex->current
 .value
 .var_name;
 pb[1] = '\0';
 pn = pb;
 pl = 1;
 is_str = 1;
 } else {
 break;
 }
 if (sd->param_count <
 MAX_SUB_PARAMS) {
 int cpl = pl;
 int j;
 if (cpl >
 MAX_VAR_NAME_LEN)
 cpl =
 MAX_VAR_NAME_LEN;
 memcpy(sd->params[
 sd->param_count],
 pn, (size_t)cpl);
 sd->params[
 sd->param_count]
 [cpl] = '\0';
 for (j = 0; j < cpl;
 j++) {
 char c =
 sd->params[
 sd->param_count
 ][j];
 if (c >= 'a' &&
 c <= 'z')
 sd->params[
 sd->param_count
 ][j] =
 (char)(c-32);
 }
 sd->param_is_string[
 sd->param_count] =
 is_str;
 sd->param_count++;
 }
 lexer_next(lex);
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 }
 if (lex->current.type ==
 TOK_RPAREN)
 lexer_next(lex);
 } else {
 lexer_next(lex);
 }
 /* No sub_count++ - already
 * counted by DECLARE */
 } else {
 /* Store in sub table */
 if (rt->sub_count >= MAX_SUBS) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 sd = &rt->subs[rt->sub_count];
 ci = nlen;
 if (ci > MAX_VAR_NAME_LEN)
 ci = MAX_VAR_NAME_LEN;
 memcpy(sd->name, nm, (size_t)ci);
 sd->name[ci] = '\0';
 /* Uppercase */
 {
 int j;
 for (j = 0; j < ci; j++) {
 if (sd->name[j] >= 'a' &&
 sd->name[j] <= 'z')
 sd->name[j] =
 (char)(sd->name[j]
 - 32);
 }
 }
 sd->name_len = ci;
 sd->is_function = is_func;
 sd->param_count = 0;
 sd->body_index =
 rt->current_index + 1;

 lexer_next(lex); /* consume name */

 /* Parse optional parameter list */
 if (lex->current.type == TOK_LPAREN) {
 lexer_next(lex); /* consume ( */
 while (lex->current.type !=
 TOK_RPAREN &&
 lex->current.type !=
 TOK_EOF) {
 const char *pn;
 int pl;
 char pb[MAX_VAR_NAME_LEN+1];
 int is_str = 0;

 if (lex->current.type ==
 TOK_NAMED_VAR) {
 pn = lex->current
 .str_start;
 pl = lex->current
 .str_length;
 } else if (lex->current.type
 == TOK_VARIABLE) {
 pb[0] = lex->current
 .value
 .var_name;
 pb[1] = '\0';
 pn = pb;
 pl = 1;
 } else if (lex->current.type
 == TOK_STRING_VAR)
 {
 pb[0] = lex->current
 .value
 .var_name;
 pb[1] = '\0';
 pn = pb;
 pl = 1;
 is_str = 1;
 } else {
 break;
 }
 if (sd->param_count <
 MAX_SUB_PARAMS) {
 int cpl = pl;
 int j;
 if (cpl > MAX_VAR_NAME_LEN)
 cpl = MAX_VAR_NAME_LEN;
 memcpy(sd->params[
 sd->param_count],
 pn, (size_t)cpl);
 sd->params[
 sd->param_count]
 [cpl] = '\0';
 for (j = 0; j < cpl; j++){
 char c = sd->params[
 sd->param_count]
 [j];
 if (c >= 'a' &&
 c <= 'z')
 sd->params[
 sd->param_count]
 [j] =
 (char)(c-32);
 }
 sd->param_is_string[
 sd->param_count] =
 is_str;
 sd->param_count++;
 }
 lexer_next(lex);
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 }
 if (lex->current.type ==
 TOK_RPAREN)
 lexer_next(lex);
 } else {
 lexer_next(lex); /* consume name */
 }

 rt->sub_count++;
 }
 }

 /* Skip forward to END SUB / END FUNCTION */
 end_kw = is_func ? KW_FUNCTION : KW_SUB;
 {
 int idx = rt->current_index + 1;
 ProgramStore *pgm = rt->program;
 while (idx < pgm->count) {
 Lexer cl;
 lexer_init(&cl,
 pgm->lines[idx].text);
 /* Skip line number */
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);
 /* Check for END */
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value.keyword ==
 KW_END) {
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value
 .keyword == end_kw) {
 /* Found matching END */
 rt->next_index = idx + 1;
 return;
 }
 }
 idx++;
 }
 error_raise(ERR_HOW, line_num);
 }
 }
 return;

 case KW_CALL:
 /*
 * CALL SubName [(arg1, arg2, ...)]
 *
 * Look up SUB, push FRAME_SUB, save vars,
 * bind params, jump to body.
 */
 {
 const char *nm;
 int nlen;
 char namebuf[MAX_VAR_NAME_LEN + 1];
 SubDef *sd;
 StackFrame frame;
 int i;

 if (lex->current.type != TOK_NAMED_VAR &&
 lex->current.type != TOK_VARIABLE &&
 lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 nm = lex->current.str_start;
 nlen = lex->current.str_length;
 if (lex->current.type == TOK_VARIABLE) {
 namebuf[0] = lex->current.value
 .var_name;
 namebuf[1] = '\0';
 nm = namebuf;
 nlen = 1;
 }
 if (nm == NULL || nlen <= 0) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 sd = runtime_find_sub(rt, nm, nlen);
 if (sd == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 lexer_next(lex); /* consume name */

 /* Push FRAME_SUB with saved variables */
 frame.type = FRAME_SUB;
 frame.data.sub_call.return_index =
 rt->current_index + 1;
 frame.data.sub_call.sub_index =
 (int)(sd - rt->subs);
 for (i = 0; i < MAX_VARIABLES; i++) {
 frame.data.sub_call.saved_vars[i] =
 rt->variables[i];
 }
 for (i = 0; i < MAX_STRING_VARS; i++) {
 frame.data.sub_call
 .saved_strvars[i] =
 rt->string_vars[i];
 }
 if (runtime_push(rt, &frame) != 0)
 return;

 /* Parse and bind arguments */
 if (lex->current.type == TOK_LPAREN) {
 lexer_next(lex); /* consume ( */
 for (i = 0; i < sd->param_count;
 i++) {
 BValue av;
 if (i > 0) {
 if (lex->current.type !=
 TOK_COMMA) break;
 lexer_next(lex);
 }
 av = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 /* Set param as named var */
 set_param_by_name(rt,
 sd->params[i], av);
 }
 if (lex->current.type == TOK_RPAREN)
 lexer_next(lex);
 } else {
 /* Args without parens */
 for (i = 0; i < sd->param_count;
 i++) {
 BValue av;
 if (i > 0) {
 if (lex->current.type !=
 TOK_COMMA) break;
 lexer_next(lex);
 }
 av = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 set_param_by_name(rt,
 sd->params[i], av);
 }
 }

 rt->in_sub_index =
 (int)(sd - rt->subs);
 rt->next_index = sd->body_index;
 lexer_skip_to_end(lex);
 }
 return;

 case KW_SHELL:
 /*
 * SHELL [command$]
 * Execute an OS command via system().
 * Without argument, opens interactive shell.
 * Captures exit code in ERRORLEVEL.
 *
 * SHELL "cmd" > "file" - redirect stdout
 * SHELL "cmd" >> "file" - append stdout
 * SHELL "cmd" | "cmd2" - pipe
 */
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_COLON ||
 lex->current.type == TOK_CR) {
 /* No argument: interactive shell */
#ifdef _WIN32
 rt->last_shell_exitcode = system("cmd");
#else
 rt->last_shell_exitcode = system("/bin/sh");
#endif
 } else {
 /*
 * Parse command as a direct string
 * token to avoid the expression parser
 * consuming > >> | as relops.
 */
 if (lex->current.type == TOK_STRING) {
 char cmd[512];
 int cl =
 lex->current.str_length;
 if (cl > 510) cl = 510;
 memcpy(cmd,
 lex->current.str_start ?
 lex->current.str_start : "",
 (size_t)cl);
 cmd[cl] = '\0';
 lexer_next(lex);

 /* Check for > or >> redirect */
 if (lex->current.type == TOK_GT ||
 lex->current.type ==
 TOK_APPEND) {
 int append =
 (lex->current.type ==
 TOK_APPEND);
 char rfile[256];
 int rlen;
 char full[768];
 lexer_next(lex);
 if (lex->current.type !=
 TOK_STRING) {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 rlen =
 lex->current.str_length;
 if (rlen > 254) rlen = 254;
 memcpy(rfile,
 lex->current.str_start,
 (size_t)rlen);
 rfile[rlen] = '\0';
 lexer_next(lex);
 sprintf(full, "%s %s \"%s\"",
 cmd,
 append ? ">>" : ">",
 rfile);
 rt->last_shell_exitcode =
 system(full);
 } else if (lex->current.type ==
 TOK_PIPE) {
 /* SHELL "a" | "b" */
 char full[1024];
 int fpos;
 fpos = (int)strlen(cmd);
 memcpy(full, cmd,
 (size_t)fpos);
 while (lex->current.type ==
 TOK_PIPE) {
 BValue nv;
 int nl;
 lexer_next(lex);
 nv =
 parse_expression_bval(
 lex, rt,
 line_num);
 if (error_occurred())
 return;
 if (!bval_is_string(&nv)){
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 full[fpos++] = ' ';
 full[fpos++] = '|';
 full[fpos++] = ' ';
 nl =
 nv.v.sval.length;
 if (fpos + nl > 1020)
 nl = 1020 - fpos;
 if (nv.v.sval.data)
 memcpy(full + fpos,
 nv.v.sval.data,
 (size_t)nl);
 fpos += nl;
 }
 full[fpos] = '\0';
 rt->last_shell_exitcode =
 system(full);
 } else {
 rt->last_shell_exitcode =
 system(cmd);
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 }
 return;

 case KW_REDIM:
 /*
 * REDIM array(size)
 * Resize dynamic arrays. Peek at name,
 * erase existing array, then DIM.
 */
 {
 /* Peek at the array name to erase it */
 char rname[MAX_VAR_NAME_LEN + 1];
 int rlen = 0, ri;
 DimArray *existing;

 if (lex->current.type == TOK_VARIABLE) {
 rname[0] = lex->current.value
 .var_name;
 rname[1] = '\0';
 rlen = 1;
 } else if (lex->current.type ==
 TOK_NAMED_VAR) {
 rlen = lex->current.str_length;
 if (rlen > MAX_VAR_NAME_LEN)
 rlen = MAX_VAR_NAME_LEN;
 memcpy(rname,
 lex->current.str_start,
 (size_t)rlen);
 rname[rlen] = '\0';
 }
 /* Uppercase for matching */
 for (ri = 0; ri < rlen; ri++) {
 if (rname[ri] >= 'a' &&
 rname[ri] <= 'z')
 rname[ri] = (char)
 (rname[ri] - 32);
 }

 /* Find and erase existing array */
 existing = runtime_find_dim(
 rt, rname, rlen);
 if (existing != NULL) {
 existing->name[0] = '\0';
 existing->total = 0;
 }

 parse_dim(lex, rt, line_num);
 }
 return;

 case KW_SHARED:
 case KW_STATIC:
 /*
 * SHARED / STATIC - variable scoping.
 * In our architecture, all variables are
 * effectively global (saved/restored by
 * FRAME_SUB). These are recognized as
 * no-ops for QBasic compatibility.
 */
 lexer_skip_to_end(lex);
 return;

 case KW_RESUME:
 /*
 * RESUME [NEXT | line]
 * Return from ON ERROR GOTO handler.
 * RESUME - retry the failed line.
 * RESUME NEXT - continue at next line.
 */
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_COLON) {
 /* RESUME - re-execute current line */
 /* (no-op, execution continues) */
 } else if (lex->current.type ==
 TOK_KEYWORD &&
 lex->current.value.keyword ==
 KW_NEXT) {
 lexer_next(lex);
 /* RESUME NEXT: continue at next line */
 } else {
 /* RESUME linenum */
 long target;
 target = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 vm_jump(rt, (int)target, line_num);
 }
 error_clear();
 return;

 case KW_OPTION:
 /*
 * OPTION BASE 0 | 1
 * Set default array lower bound.
 *
 * OPTION STRICT
 * Enable strict dialect mode: only
 * keywords belonging to the active
 * dialect are allowed.
 *
 * OPTION STRICT OFF
 * Disable strict mode (union mode).
 */
 /*
 * OPTION ANGLE DEGREES | RADIANS
 * Set trig function angle mode.
 */
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 5 &&
 lex->current.str_start != NULL) {
 const char *a = lex->current.str_start;
 if ((a[0]=='A'||a[0]=='a') &&
 (a[1]=='N'||a[1]=='n') &&
 (a[2]=='G'||a[2]=='g') &&
 (a[3]=='L'||a[3]=='l') &&
 (a[4]=='E'||a[4]=='e')) {
 lexer_next(lex); /* consume ANGLE */
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length >= 3 &&
 lex->current.str_start != NULL) {
 const char *m =
 lex->current.str_start;
 if ((m[0]=='D'||m[0]=='d') &&
 (m[1]=='E'||m[1]=='e') &&
 (m[2]=='G'||m[2]=='g')) {
 rt->angle_degrees = 1;
 lexer_next(lex);
 } else if (
 (m[0]=='R'||m[0]=='r') &&
 (m[1]=='A'||m[1]=='a') &&
 (m[2]=='D'||m[2]=='d')) {
 rt->angle_degrees = 0;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;
 }
 }
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 6 &&
 lex->current.str_start != NULL) {
 /* Check for "STRICT" */
 const char *s = lex->current.str_start;
 if ((s[0]=='S'||s[0]=='s') &&
 (s[1]=='T'||s[1]=='t') &&
 (s[2]=='R'||s[2]=='r') &&
 (s[3]=='I'||s[3]=='i') &&
 (s[4]=='C'||s[4]=='c') &&
 (s[5]=='T'||s[5]=='t')) {
 lexer_next(lex); /* consume STRICT */
 /* Check for OFF */
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 3 &&
 lex->current.str_start != NULL &&
 (lex->current.str_start[0]=='O'||
 lex->current.str_start[0]=='o')) {
 dialect_set_strict(0);
 printf("Strict mode: OFF"
 " (union)\n");
 lexer_next(lex);
 } else {
 dialect_set_strict(1);
 printf("Strict mode: ON"
 " (%s only)\n",
 dialect_get_short_name());
 }
 return;
 }
 }
 /*
 * OPTION ARITHMETIC NATIVE | DECIMAL
 * ECMA-116 arithmetic mode. NATIVE uses
 * hardware floating point (our default).
 * DECIMAL would require base-10 math.
 */
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 10 &&
 lex->current.str_start != NULL) {
 const char *a = lex->current.str_start;
 if ((a[0]=='A'||a[0]=='a') &&
 (a[1]=='R'||a[1]=='r') &&
 (a[2]=='I'||a[2]=='i') &&
 (a[3]=='T'||a[3]=='t') &&
 (a[4]=='H'||a[4]=='h') &&
 (a[5]=='M'||a[5]=='m') &&
 (a[6]=='E'||a[6]=='e') &&
 (a[7]=='T'||a[7]=='t') &&
 (a[8]=='I'||a[8]=='i') &&
 (a[9]=='C'||a[9]=='c')) {
 lexer_next(lex); /* ARITHMETIC */
 if (lex->current.type == TOK_NAMED_VAR
 && lex->current.str_length >= 3
 && lex->current.str_start
 != NULL) {
 const char *m =
 lex->current.str_start;
 if ((m[0]=='N'||m[0]=='n') &&
 (m[1]=='A'||m[1]=='a') &&
 (m[2]=='T'||m[2]=='t')) {
 /* NATIVE - no-op (default) */
 lexer_next(lex);
 } else if (
 (m[0]=='D'||m[0]=='d') &&
 (m[1]=='E'||m[1]=='e') &&
 (m[2]=='C'||m[2]=='c')) {
 /* DECIMAL - not supported */
 printf("SORRY. Decimal"
 " arithmetic is not"
 " implemented.\n");
 error_raise(ERR_SORRY,
 line_num);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;
 }
 }
 /*
 * Consume BASE (may be keyword, named var,
 * or multiple single-letter variables when
 * extended vars are off). Skip everything
 * until we reach the numeric argument.
 */
 while (lex->current.type != TOK_NUMBER &&
 lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 lexer_next(lex);
 }
 /* Consume the 0 or 1 and set option_base */
 if (lex->current.type == TOK_NUMBER) {
 int base_val = (int)lex->current.value.num_value;
 if (base_val == 0 || base_val == 1) {
 rt->option_base = base_val;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 }
 return;

 case KW_COMMON:
 /*
 * COMMON [SHARED] var1, var2, ...
 * Share variables with CHAINed programs.
 * No-op: all vars are shared in our
 * single-process architecture.
 */
 lexer_skip_to_end(lex);
 return;

 case KW_FIELD:
 /*
 * FIELD #n, width AS var$ [, width AS var$]
 * Map string variables to positions in
 * the random-access file record buffer.
 */
 {
 int chan;
 int offset = 0;
 FieldMap flds[MAX_FIELD_MAPS];
 int fcount = 0;

 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);

 /* Parse width AS var$ pairs */
 while (fcount < MAX_FIELD_MAPS &&
 lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 int w;
 w = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 /* Expect AS */
 if (!lexer_match_keyword(
 lex, KW_AS)) {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 lexer_next(lex);
 /* Get var name (string var) */
 if (lex->current.type ==
 TOK_STRING_VAR) {
 flds[fcount].name[0] =
 lex->current
 .value.var_name;
 flds[fcount].name_len = 1;
 } else if (lex->current.type ==
 TOK_NAMED_VAR) {
 int nl =
 lex->current.str_length;
 if (nl > 30) nl = 30;
 memcpy(flds[fcount].name,
 lex->current.str_start,
 (size_t)nl);
 flds[fcount].name_len = nl;
 } else {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 lexer_next(lex);
 flds[fcount].offset = offset;
 flds[fcount].width = w;
 offset += w;
 fcount++;
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 }
 fileio_set_field(chan, flds,
 fcount, line_num);
 }
 return;

 case KW_GET:
 /*
 * GET #n [, record]
 * Read a record from a random-access
 * file, or read bytes from BINARY.
 *
 * GET (x1,y1)-(x2,y2), arrayname
 * Capture a graphics screen region
 * (graphics stub).
 */
 if (lex->current.type == TOK_HASH ||
 lex->current.type == TOK_NUMBER) {
 int chan;
 long rec = 0;
 int got_rec = 0;
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 rec = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 got_rec = 1;
 }
 {
 int cmode =
 fileio_get_channel_mode(
 chan);
 if (cmode == FCHAN_RANDOM) {
 if (!got_rec) rec = 0;
 fileio_get_record(chan,
 rec > 0 ? rec : 1,
 line_num);
 if (!error_occurred()) {
 /* Copy fields to vars */
 int fi;
 for (fi = 0; fi < 26;
 fi++) {
 char nm[2];
 int flen;
 const char *fv;
 nm[0]=(char)('A'+fi);
 nm[1]='\0';
 fv =
 fileio_get_field_value(
 chan, nm, 1,
 &flen);
 if (fv) {
 char *p =
 strpool_store(
 &rt->strpool,
 fv, flen);
 runtime_set_string_var(
 rt,
 (char)('A'+fi),
 bval_string(
 p, flen));
 }
 }
 }
 } else if (cmode==FCHAN_BINARY){
 /* Binary: GET #n, pos */
 /* Reads 1 byte at pos */
 char bb;
 int nr;
 nr = fileio_get_binary(
 chan,
 got_rec ? rec : 0,
 &bb, 1, line_num);
 (void)nr;
 } else {
 error_raise(ERR_HOW,
 line_num);
 }
 }
 } else {
 lexer_skip_to_end(lex);
 }
 return;

 case KW_PUT:
 /*
 * PUT #n [, record]
 * Write a record to a random-access
 * file, or write bytes to BINARY.
 *
 * PUT (x,y), array (graphics stub)
 */
 if (lex->current.type == TOK_HASH ||
 lex->current.type == TOK_NUMBER) {
 int chan;
 long rec = 0;
 int got_rec = 0;
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 rec = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 got_rec = 1;
 }
 {
 int cmode =
 fileio_get_channel_mode(
 chan);
 if (cmode == FCHAN_RANDOM) {
 fileio_put_record(chan,
 rec > 0 ? rec : 1,
 line_num);
 } else if (cmode==FCHAN_BINARY){
 char bb = 0;
 fileio_put_binary(chan,
 got_rec ? rec : 0,
 &bb, 1, line_num);
 } else {
 error_raise(ERR_HOW,
 line_num);
 }
 }
 } else {
 lexer_skip_to_end(lex);
 }
 return;

 case KW_IOCTL:
 /*
 * IOCTL #n, string$
 * Send I/O control string to device.
 * Returns status of the channel.
 */
 {
 int chan;
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 /* Consume the control string */
 (void)parse_expression_bval(
 lex, rt, line_num);
 (void)chan;
 /* Control string accepted;
 * no device-level action. */
 }
 return;

 case KW_KEY:
 /*
 * KEY n, string$
 * Assign string to function key slot.
 * Slots: 1-12=F1-F12, 13-24=SHIFT+F,
 * 25-36=CTRL+F, 37-48=ALT+F.
 *
 * KEY ON / KEY OFF
 * Toggle function key display bar.
 *
 * KEY LIST
 * Print all key assignments.
 *
 * KEY(n) ON / OFF / STOP
 * Enable/disable key event trapping.
 */
 /* KEY(n) ON/OFF/STOP form */
 if (lex->current.type == TOK_LPAREN) {
 lexer_skip_to_end(lex);
 return;
 }
 /* KEY ON / KEY LIST (ON is keyword) */
 if (lex->current.type == TOK_KEYWORD) {
 KeywordId sub = lex->current
 .value.keyword;
 if (sub == KW_ON) {
 lexer_next(lex);
 rt->fkey_display = 1;
 /* Print the key bar */
 goto do_key_bar;
 }
 if (sub == KW_LIST) {
 lexer_next(lex);
 goto do_key_list;
 }
 }
 /* KEY OFF / KEY LIST / ON (named var) */
 if (lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_VARIABLE) {
 const char *nv;
 int nlen;
 /* For TOK_VARIABLE, get letter */
 if (lex->current.type == TOK_VARIABLE) {
 char vl = lex->current
 .value.var_name;
 /* Check if remaining text is
 * "OFF" or "ON" by scanning
 * ahead in source */
 nv = lex->source + lex->pos - 1;
 /* Variable already consumed the
 * first char. We need the raw
 * text. Look at source from the
 * start of this ident. */
 (void)vl;
 /* Can't reconstruct - fall
 * through to assignment */
 goto do_key_assign;
 }
 nv = lex->current.str_start;
 nlen = lex->current.str_length;
 /* OFF */
 if (nlen == 3 &&
 (nv[0]=='O'||nv[0]=='o') &&
 (nv[1]=='F'||nv[1]=='f') &&
 (nv[2]=='F'||nv[2]=='f')) {
 lexer_next(lex);
 rt->fkey_display = 0;
 return;
 }
 /* LIST */
 if (nlen == 4 &&
 (nv[0]=='L'||nv[0]=='l') &&
 (nv[1]=='I'||nv[1]=='i') &&
 (nv[2]=='S'||nv[2]=='s') &&
 (nv[3]=='T'||nv[3]=='t')) {
 lexer_next(lex);
 goto do_key_list;
 }
 }
 /* Fall through to assignment */
 goto do_key_assign;

 do_key_list:
 {
 static const char *mod[] = {
 "", "SHIFT+",
 "CTRL+", "ALT+"
 };
 int si;
 int found = 0;
 for (si = 1; si <= FKEY_MAX_SLOTS; si++) {
 if (rt->fkey_macros[si][0]) {
 int grp = (si - 1) / 12;
 int fn = (si - 1) % 12 + 1;
 printf(" KEY %2d "
 "%sF%-2d \"%s\"\n",
 si, mod[grp],
 fn,
 rt->fkey_macros[si]);
 found = 1;
 }
 }
 if (!found) {
 printf(" (no keys "
 "assigned)\n");
 }
 }
 return;

 do_key_bar:
 /*
 * Print GW-BASIC style key bar:
 * 1HELP 2LOAD 3RUN ...10 
 */
 {
 int ki;
 for (ki = 1; ki <= 10; ki++) {
 if (rt->fkey_macros[ki][0]) {
 char trunc[7];
 int ti;
 int pad;
 for (ti = 0; ti < 6 &&
 rt->fkey_macros[ki][ti];
 ti++) {
 trunc[ti] =
 rt->fkey_macros[ki][ti];
 }
 trunc[ti] = '\0';
 printf("%d%s", ki, trunc);
 /* Pad to 8 chars total */
 pad = 8 - ti
 - (ki >= 10 ? 2 : 1);
 while (pad-- > 0)
 putchar(' ');
 } else {
 printf("%-8d", ki);
 }
 }
 printf("\n");
 }
 return;

 do_key_assign:
 /* KEY n, string$ - assignment */
 {
 long slot;
 int slen;
 slot = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); /* consume , */
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (slot < 1 || slot > FKEY_MAX_SLOTS) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 slen = lex->current.str_length;
 if (slen > FKEY_MAX_LEN)
 slen = FKEY_MAX_LEN;
 memcpy(rt->fkey_macros[slot],
 lex->current.str_start,
 (size_t)slen);
 rt->fkey_macros[slot][slen] = '\0';
 lexer_next(lex);
 }
 return;

 case KW_KILL:
 /*
 * KILL filename$
 * Delete a file from disk.
 */
 {
 char fname[260];
 int flen;

 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 flen = lex->current.str_length;
 if (flen > 259) flen = 259;
 memcpy(fname,
 lex->current.str_start,
 (size_t)flen);
 fname[flen] = '\0';
 lexer_next(lex);

 if (remove(fname) != 0) {
 printf("File not found: %s\n",
 fname);
 }
 }
 return;

 case KW_LLIST:
 /*
 * LLIST [start] [-] [end]
 * List program to printer. In our
 * terminal environment, same as LIST.
 */
 {
 int ls = 0, le = 0;
 if (lex->current.type ==
 TOK_NUMBER) {
 ls = (int)lex->current
 .value.num_value;
 lexer_next(lex);
 }
 if (lex->current.type == TOK_MINUS) {
 lexer_next(lex);
 if (lex->current.type ==
 TOK_NUMBER) {
 le = (int)lex->current
 .value.num_value;
 lexer_next(lex);
 } else {
 le = 99999;
 }
 } else if (ls > 0) {
 le = ls;
 }
 /* Reuse LIST output */
 {
 int li;
 for (li = 0;
 li < rt->program->count;
 li++) {
 int ln = rt->program
 ->lines[li]
 .line_number;
 if (ls > 0 && ln < ls)
 continue;
 if (le > 0 && ln > le)
 break;
 printf("%s\n",
 rt->program->lines[li]
 .text);
 }
 }
 }
 return;

 case KW_LOCK:
 case KW_UNLOCK:
 /*
 * LOCK/UNLOCK #n [, record [TO record]]
 * OS-level file locking.
 */
 {
 int is_lock = (kw == KW_LOCK);
 int chan;
 long s = 1, e = 1;
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 s = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 e = s;
 }
 if (lex->current.type == TOK_KEYWORD
 && lex->current.value.keyword
 == KW_TO) {
 lexer_next(lex);
 e = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (is_lock)
 fileio_lock(chan, s, e,
 line_num);
 else
 fileio_unlock(chan, s, e,
 line_num);
 }
 return;

 case KW_VIEW:
 /*
 * VIEW [[SCREEN] (x1,y1)-(x2,y2) [,c[,b]]]
 * VIEW PRINT [top TO bottom]
 *
 * VIEW: set graphics viewport.
 * VIEW PRINT: set text scroll region.
 * Accepted for compatibility; consume args.
 */
 if (lexer_match_keyword(lex,
 KW_PRINT)) {
 /* VIEW PRINT [top TO bottom] */
 lexer_next(lex);
 }
 lexer_skip_to_end(lex);
 return;

 case KW_WAIT:
 /*
 * WAIT port, and_mask [, xor_mask]
 * Wait until I/O port satisfies mask.
 * No port I/O; consume args as no-op.
 */
 lexer_skip_to_end(lex);
 return;

 case KW_WINDOW:
 /*
 * WINDOW [[SCREEN] (x1,y1)-(x2,y2)]
 * Define logical coordinate system.
 * Accepted for compatibility; consume.
 */
 lexer_skip_to_end(lex);
 return;

 case KW_MEMMAP:
 /*
 * MEMMAP "platform"
 * MEMMAP LIST
 *
 * Select a platform memory map preset.
 * Initializes the 64K virtual memory
 * with correct ROM/hardware values for
 * the selected 8-bit platform.
 */
 if (lexer_match_keyword(lex,
 KW_LIST)) {
 lexer_next(lex);
 memmap_list();
 } else if (lex->current.type ==
 TOK_STRING) {
 MemMapType mtype;
 mtype = memmap_from_string(
 lex->current.str_start,
 lex->current.str_length);
 lexer_next(lex);
 if (mtype == MMAP_COUNT) {
 printf("Unknown memory map."
 " Use MEMMAP LIST.\n");
 } else {
 memmap_init(rt->mem_segment,
 mtype);
 rt->memmap_type = (int)mtype;
 rt->mem_seg_base = 0;
 printf("Memory map: %s\n",
 memmap_get_name(mtype));
 }
 } else {
 /* Show current map */
 printf("Current map: %s\n",
 memmap_get_name(
 (MemMapType)rt->memmap_type));
 }
 return;

 case KW_SYS:
 /*
 * SYS address
 * Commodore-style: call machine language
 * at the given address.
 * No machine code execution; consume the
 * address argument as a no-op stub.
 */
 (void)parse_expression(lex, rt,
 line_num);
 return;

 case KW_EXEC:
 /*
 * EXEC command$
 * Fire-and-forget: launch a command
 * without waiting for it to complete.
 * Uses "start" on Windows, "& " on Unix.
 */
 {
 BValue ev =
 parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&ev)) {
 char cmd[560];
 int el = ev.v.sval.length;
 if (el > 500) el = 500;
#ifdef _WIN32
 memcpy(cmd, "start \"\" ", 9);
 if (ev.v.sval.data)
 memcpy(cmd + 9,
 ev.v.sval.data,
 (size_t)el);
 cmd[9 + el] = '\0';
#else
 if (ev.v.sval.data)
 memcpy(cmd,
 ev.v.sval.data,
 (size_t)el);
 cmd[el] = ' ';
 cmd[el + 1] = '&';
 cmd[el + 2] = '\0';
#endif
 system(cmd);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 }
 }
 return;

 case KW_WRITE:
 /*
 * WRITE [#n,] expr [, expr ...]
 * Output data items separated by commas.
 * Strings are enclosed in double quotes.
 * A newline is printed at the end.
 *
 * WRITE (no args) prints a blank line.
 * WRITE #n, ... writes to file channel.
 */
 {
 int wfile = 0; /* file channel */
 int wfirst = 1; /* first item? */

 /* WRITE #n, ... */
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex);
 wfile = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 }

 /* Parse expression list */
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type !=
 TOK_COLON) {

 /* Print comma between items */
 if (!wfirst) {
 if (wfile > 0)
 fileio_print(wfile,
 ",", line_num);
 else
 printf(",");
 }
 wfirst = 0;

 /* String literal or expression */
 if (lex->current.type ==
 TOK_STRING) {
 /* Print quoted string */
 char buf[256];
 int slen =
 lex->current.str_length;
 if (slen > 253) slen = 253;
 buf[0] = '"';
 memcpy(buf + 1,
 lex->current.str_start,
 (size_t)slen);
 buf[slen + 1] = '"';
 buf[slen + 2] = '\0';
 if (wfile > 0)
 fileio_print(wfile,
 buf, line_num);
 else
 printf("%s", buf);
 lexer_next(lex);
 } else {
 /* Numeric expression */
 BValue val =
 parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (val.type == VAL_STRING) {
 char buf[256];
 int slen =
 val.v.sval.length;
 if (slen > 253)
 slen = 253;
 buf[0] = '"';
 if (val.v.sval.data)
 memcpy(buf + 1,
 val.v.sval.data,
 (size_t)slen);
 buf[slen + 1] = '"';
 buf[slen + 2] = '\0';
 if (wfile > 0)
 fileio_print(wfile,
 buf, line_num);
 else
 printf("%s", buf);
 } else {
 char buf[32];
 sprintf(buf, "%G",
 bval_to_float(&val));
 if (wfile > 0)
 fileio_print(wfile,
 buf, line_num);
 else
 printf(" %s", buf);
 }
 }

 /* Consume comma separator */
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 else
 break;
 }

 /* Newline at end */
 if (wfile > 0)
 fileio_print_newline(wfile,
 line_num);
 else
 printf("\n");
 }
 return;

 case KW_LSET:
 /*
 * LSET var$ = string$
 * Left-justify string in field buffer.
 * If var is a FIELD variable, pads with
 * spaces to field width. Otherwise acts
 * as plain assignment.
 */
 if (lex->current.type ==
 TOK_STRING_VAR) {
 char sv = lex->current
 .value.var_name;
 BValue val;
 char vn[2];
 int ch;
 lexer_next(lex);
 if (lex->current.type != TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 /* Try all channels for field match */
 vn[0] = sv; vn[1] = '\0';
 for (ch = 1; ch <= 8; ch++) {
 if (fileio_get_channel_mode(ch)
 == FCHAN_RANDOM) {
 int r;
 r = fileio_set_field_value(
 ch, vn, 1,
 bval_is_string(&val) ?
 val.v.sval.data : "",
 bval_is_string(&val) ?
 val.v.sval.length : 0,
 0, line_num);
 if (r == 0) return;
 }
 }
 /* Not a field - plain assignment */
 runtime_set_string_var(rt, sv, val);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;

 case KW_RSET:
 /*
 * RSET var$ = string$
 * Right-justify string in field buffer.
 */
 if (lex->current.type ==
 TOK_STRING_VAR) {
 char sv = lex->current
 .value.var_name;
 BValue val;
 char vn[2];
 int ch;
 lexer_next(lex);
 if (lex->current.type != TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 vn[0] = sv; vn[1] = '\0';
 for (ch = 1; ch <= 8; ch++) {
 if (fileio_get_channel_mode(ch)
 == FCHAN_RANDOM) {
 int r;
 r = fileio_set_field_value(
 ch, vn, 1,
 bval_is_string(&val) ?
 val.v.sval.data : "",
 bval_is_string(&val) ?
 val.v.sval.length : 0,
 1, line_num);
 if (r == 0) return;
 }
 }
 runtime_set_string_var(rt, sv, val);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;

 case KW_RESET:
 /*
 * RESET
 * Close all open files.
 * GW-BASIC: closes all files and writes
 * directory information to disk.
 */
 fileio_channels_cleanup();
 fileio_channels_init();
 return;

 case KW_RMDIR:
 /*
 * RMDIR path$
 * Remove a directory.
 */
 {
 char dname[260];
 int dlen;

 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 dlen = lex->current.str_length;
 if (dlen > 259) dlen = 259;
 memcpy(dname,
 lex->current.str_start,
 (size_t)dlen);
 dname[dlen] = '\0';
 lexer_next(lex);
#ifdef _WIN32
 _rmdir(dname);
#else
 rmdir(dname);
#endif
 }
 return;

 case KW_ALIAS:
 /*
 * ALIAS keyword = "newname"
 * ALIAS LIST
 * ALIAS CLEAR
 *
 * Remap a keyword to a user-defined name.
 * The alias then works everywhere the
 * original keyword does.
 */
 {
 KeywordId target_kw;

 /* ALIAS LIST */
 if (lexer_match_keyword(lex,
 KW_LIST)) {
 lexer_next(lex);
 lexer_list_aliases();
 return;
 }

 /* ALIAS CLEAR (named var in GWBS) */
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *nv =
 lex->current.str_start;
 int nlen =
 lex->current.str_length;
 if (nlen == 5 &&
 (nv[0]=='C'||nv[0]=='c') &&
 (nv[1]=='L'||nv[1]=='l') &&
 (nv[2]=='E'||nv[2]=='e') &&
 (nv[3]=='A'||nv[3]=='a') &&
 (nv[4]=='R'||nv[4]=='r')) {
 lexer_next(lex);
 lexer_clear_aliases();
 printf("Aliases cleared.\n");
 return;
 }
 }
 /* ALIAS CLEAR (keyword in GWBS) */
 if (lexer_match_keyword(lex,
 KW_CLEAR)) {
 lexer_next(lex);
 lexer_clear_aliases();
 printf("Aliases cleared.\n");
 return;
 }

 /* Expect a keyword to alias */
 if (lex->current.type !=
 TOK_KEYWORD) {
 printf(
 "Usage:\n"
 " ALIAS keyword = "
 "\"newname\"\n"
 " ALIAS LIST\n"
 " ALIAS CLEAR\n");
 return;
 }
 target_kw =
 lex->current.value.keyword;
 lexer_next(lex);

 /* Skip optional $ for functions */
 /* (already consumed by lexer) */

 /* Expect = */
 if (lex->current.type !=
 TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 /* Expect string with new name */
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 {
 const char *aname =
 lex->current.str_start;
 int alen =
 lex->current.str_length;
 const char *orig;
 int need_d;

 lexer_next(lex);

 if (lexer_add_alias(aname, alen,
 target_kw)
 != 0) {
 printf("Alias table full.\n");
 return;
 }

 orig = lexer_keyword_name(
 target_kw);
 need_d =
 lexer_keyword_needs_dollar(
 target_kw);
 printf("ALIAS %s%s = \"",
 orig,
 need_d ? "$" : "");
 {
 int pi;
 for (pi = 0; pi < alen;
 pi++)
 putchar(toupper(
 aname[pi]));
 }
 printf("\"\n");
 }
 }
 return;

 case KW_OUT:
 /*
 * OUT port, value
 * Write to I/O port.
 * Routes to virtual memory segment so
 * MEMMAP presets can be modified via OUT
 * just like POKE.
 */
 {
 int port, val, paddr;
 port = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 val = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 paddr = port & 0xFFFF;
 if (paddr >= 0 &&
 paddr < MAX_MEM_SEGMENT) {
 rt->mem_segment[paddr] =
 (unsigned char)(val & 0xFF);
 }
 }
 return;

 case KW_COM:
 case KW_PEN:
 case KW_STRIG:
 /*
 * COM(n) ON/OFF/STOP
 * PEN ON/OFF/STOP
 * STRIG ON/OFF/STOP
 * STRIG(n) ON/OFF/STOP
 *
 * Event trapping control. Accepted for
 * compatibility; consume all arguments.
 */
 lexer_skip_to_end(lex);
 return;

 case KW_NAME:
 /*
 * NAME "oldname" AS "newname"
 * Rename a file (GW-BASIC compatible).
 */
 {
 char old_name[260], new_name[260];
 int olen, nlen;

 /* Old filename */
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 olen = lex->current.str_length;
 if (olen > 259) olen = 259;
 memcpy(old_name,
 lex->current.str_start,
 (size_t)olen);
 old_name[olen] = '\0';
 lexer_next(lex);

 /* Expect AS keyword */
 if (!lexer_match_keyword(lex,
 KW_AS)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 /* New filename */
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 nlen = lex->current.str_length;
 if (nlen > 259) nlen = 259;
 memcpy(new_name,
 lex->current.str_start,
 (size_t)nlen);
 new_name[nlen] = '\0';
 lexer_next(lex);

 if (rename(old_name, new_name) != 0) {
 printf("File not found: %s\n",
 old_name);
 }
 }
 return;

 case KW_RENAME:
 /*
 * RENAME "oldname" AS "newname"
 * Like NAME but auto-appends .bas/.bpp
 * extension if not present.
 */
 {
 char old_name[260], new_name[260];
 char old_try[264], new_try[264];
 int olen, nlen;
 int has_ext_o, has_ext_n;
 int ok = 0;

 /* Old filename */
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 olen = lex->current.str_length;
 if (olen > 255) olen = 255;
 memcpy(old_name,
 lex->current.str_start,
 (size_t)olen);
 old_name[olen] = '\0';
 lexer_next(lex);

 /* Expect AS keyword */
 if (!lexer_match_keyword(lex,
 KW_AS)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 /* New filename */
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 nlen = lex->current.str_length;
 if (nlen > 255) nlen = 255;
 memcpy(new_name,
 lex->current.str_start,
 (size_t)nlen);
 new_name[nlen] = '\0';
 lexer_next(lex);

 /* Check if names have extensions */
 {
 int di;
 has_ext_o = 0;
 has_ext_n = 0;
 for (di = olen - 1; di >= 0;
 di--) {
 if (old_name[di] == '.') {
 has_ext_o = 1;
 break;
 }
 if (old_name[di] == '/' ||
 old_name[di] == '\\')
 break;
 }
 for (di = nlen - 1; di >= 0;
 di--) {
 if (new_name[di] == '.') {
 has_ext_n = 1;
 break;
 }
 if (new_name[di] == '/' ||
 new_name[di] == '\\')
 break;
 }
 }

 /* Try with extensions if none given */
 if (!has_ext_o && !has_ext_n) {
 /* Try .bas first */
 sprintf(old_try, "%s.bas",
 old_name);
 sprintf(new_try, "%s.bas",
 new_name);
 if (rename(old_try,
 new_try) == 0) {
 ok = 1;
 } else {
 /* Try .bpp */
 sprintf(old_try, "%s.bpp",
 old_name);
 sprintf(new_try, "%s.bpp",
 new_name);
 if (rename(old_try,
 new_try) == 0) {
 ok = 1;
 }
 }
 }

 /* Fall back to exact names */
 if (!ok) {
 if (rename(old_name,
 new_name) != 0) {
 printf("File not found: "
 "%s\n", old_name);
 }
 }
 }
 return;

 case KW_MKDIR:
 /*
 * MKDIR path$
 * Create a directory.
 */
 {
 char dname[260];
 int dlen;

 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 dlen = lex->current.str_length;
 if (dlen > 259) dlen = 259;
 memcpy(dname,
 lex->current.str_start,
 (size_t)dlen);
 dname[dlen] = '\0';
 lexer_next(lex);
#ifdef _WIN32
 _mkdir(dname);
#else
 mkdir(dname, 0755);
#endif
 }
 return;

 case KW_MID:
 /*
 * MID$(var$, start [, len]) = rep$
 * Replace characters in var$ in-place.
 */
 {
 char sv_name;
 int start_pos, rep_len;
 int max_replace;
 BValue cur_val, rep_val;
 char *cur_data, *rep_data;
 int cur_len, rd_len, copy_len;
 char *new_ptr;

 /* Expect ( */
 if (lex->current.type !=
 TOK_LPAREN) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 /* String variable name */
 if (lex->current.type !=
 TOK_STRING_VAR) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 sv_name =
 lex->current.value.var_name;
 lexer_next(lex);

 /* Comma, start position */
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 start_pos =
 (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 /* Optional length */
 max_replace = -1;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 max_replace =
 (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }

 /* Close paren */
 if (!lexer_expect(lex, TOK_RPAREN))
 return;

 /* = */
 if (lex->current.type !=
 TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 /* Replacement string */
 rep_val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;

 /* Get current string value */
 cur_val = runtime_get_string_var(
 rt, sv_name);
 if (cur_val.type != VAL_STRING ||
 cur_val.v.sval.data == NULL) {
 return; /* empty, nothing to do */
 }
 cur_data = cur_val.v.sval.data;
 cur_len = cur_val.v.sval.length;

 if (start_pos < 1 ||
 start_pos > cur_len) {
 return; /* out of range, no-op */
 }

 rep_data = rep_val.v.sval.data;
 rd_len = rep_val.v.sval.length;
 if (rep_data == NULL) rd_len = 0;

 /* Calculate how many chars to copy */
 rep_len = cur_len - (start_pos - 1);
 if (max_replace >= 0 &&
 max_replace < rep_len)
 rep_len = max_replace;
 copy_len = rd_len;
 if (copy_len > rep_len)
 copy_len = rep_len;

 /* Build new string */
 new_ptr = strpool_store(
 &rt->strpool, cur_data, cur_len);
 if (new_ptr && copy_len > 0) {
 memcpy(new_ptr + (start_pos - 1),
 rep_data,
 (size_t)copy_len);
 runtime_set_string_var(rt,
 sv_name,
 bval_string(new_ptr,
 cur_len));
 }
 }
 return;

 case KW_DEFINT:
 case KW_DEFDBL:
 case KW_DEFSNG:
 case KW_DEFSTR:
 /*
 * DEFINT A-Z / DEFDBL A-Z / etc.
 * GW-BASIC: declares default type for
 * variable name ranges. No-op in BASIC++
 * which is dynamically typed.
 * Consume everything to end of line.
 */
 lexer_skip_to_end(lex);
 return;

 case KW_EDIT:
 /*
 * EDIT [line_number]
 * Interactive line editor. No-op in batch
 * execution. Consume optional line number.
 */
 lexer_skip_to_end(lex);
 return;

 case KW_ERDEV:
 case KW_EXTERR:
 /*
 * ERDEV / EXTERR
 * Device and DOS extended error. As
 * statements these are no-ops.
 */
 lexer_skip_to_end(lex);
 return;

 case KW_ENVIRON:
 /*
 * ENVIRON$ as function handled in factor.
 * ENVIRON as statement: set env var.
 * Not fully supported - skip.
 */
 lexer_skip_to_end(lex);
 return;

 case KW_SEEK:
 /*
 * SEEK #n, position
 * Set the file position for channel n.
 */
 {
 int chan;
 long pos;
 FILE *fp;

 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 pos = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;

 fp = fileio_get_fp(chan);
 if (fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 fseek(fp, pos - 1, SEEK_SET);
 }
 return;

 case KW_CHDIR:
 /*
 * CHDIR path$
 * Change the current working directory.
 */
 {
 BValue pv;
 pv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&pv) &&
 pv.v.sval.data != NULL) {
 char path[512];
 int pl = pv.v.sval.length;
 if (pl > 510) pl = 510;
 memcpy(path, pv.v.sval.data,
 (size_t)pl);
 path[pl] = '\0';
#ifdef _WIN32
 _chdir(path);
#else
 chdir(path);
#endif
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 }
 return;

 case KW_ERROR:
 /*
 * ERROR n
 * Trigger a simulated error with code n.
 * Used for testing error handlers.
 */
 {
 long errcode;
 errcode = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 /* Map to internal error codes or use
 * ERR_HOW as generic runtime error */
 error_raise(ERR_HOW, line_num);
 (void)errcode;
 }
 return;

 case KW_CAUSE:
 /*
 * CAUSE EXCEPTION n
 * ECMA-116: Raise a user exception with code n.
 *
 * Sets ERR = n and ERL = current line, then
 * raises a runtime error. Compatible with
 * ON ERROR GOTO error trapping.
 *
 * The word EXCEPTION is consumed as a named
 * variable token (not a keyword).
 */
 {
 long exc_code;
 /* Consume EXCEPTION (optional for compat) */
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length >= 3) {
 const char *e =
 lex->current.str_start;
 if ((e[0]=='E'||e[0]=='e') &&
 (e[1]=='X'||e[1]=='x') &&
 (e[2]=='C'||e[2]=='c')) {
 lexer_next(lex);
 }
 }
 exc_code = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 rt->last_err_code = (int)exc_code;
 rt->last_err_line = line_num;
 error_raise(ERR_HOW, line_num);
 }
 return;

 case KW_WHEN:
 /*
 * WHEN EXCEPTION IN
 * ECMA-116: Start a structured exception block.
 *
 * Forward-scan to find USE and END WHEN, push
 * a FRAME_EXCEPTION, then continue executing
 * the protected block.
 *
 * The words EXCEPTION and IN are consumed as
 * named variable tokens (not keywords).
 */
 {
 int use_idx, end_when_idx;
 StackFrame *f;

 /* Consume EXCEPTION IN tokens */
 if (lex->current.type == TOK_NAMED_VAR)
 lexer_next(lex); /* EXCEPTION */
 if (lex->current.type == TOK_NAMED_VAR)
 lexer_next(lex); /* IN */

 /* Forward-scan for USE and END WHEN */
 if (!when_exception_scan(
 rt, rt->current_index,
 &use_idx, &end_when_idx,
 line_num)) {
 return; /* error already raised */
 }

 /* Push exception frame */
 if (rt->stack_top >= MAX_STACK_DEPTH) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 f = &rt->stack[rt->stack_top++];
 f->type = FRAME_EXCEPTION;
 f->data.exception.when_index =
 rt->current_index;
 f->data.exception.use_index = use_idx;
 f->data.exception.end_when_index =
 end_when_idx;
 f->data.exception.err_index = -1;
 }
 return;

 case KW_USE:
 /*
 * USE - ECMA-116 exception handler marker.
 *
 * If reached by fall-through (no error or
 * after CONTINUE), skip to END WHEN.
 * If exception frame exists, pop it first.
 * If frame was already popped (CONTINUE),
 * scan forward for END WHEN.
 */
 {
 int i;
 /* Find the innermost exception frame */
 for (i = rt->stack_top - 1; i >= 0; i--) {
 if (rt->stack[i].type ==
 FRAME_EXCEPTION) {
 int end_idx = rt->stack[i]
 .data.exception
 .end_when_index;
 /* Pop all frames down to
 * and including this one */
 rt->stack_top = i;
 /* Jump to END WHEN */
 rt->next_index = end_idx;
 return;
 }
 }
 /*
 * No frame (popped by CONTINUE).
 * Scan forward for END WHEN.
 */
 {
 int idx = rt->current_index + 1;
 ProgramStore *pgm = rt->program;
 int depth = 0;
 while (idx < pgm->count) {
 Lexer cl;
 lexer_init(&cl,
 pgm->lines[idx].text);
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_KEYWORD) {
 if (cl.current.value.keyword
 == KW_WHEN) {
 depth++;
 } else if (
 cl.current.value.keyword
 == KW_END) {
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value
 .keyword ==
 KW_WHEN) {
 if (depth > 0)
 depth--;
 else {
 rt->next_index =
 idx;
 return;
 }
 }
 }
 }
 idx++;
 }
 error_raise(ERR_WHAT, line_num);
 }
 }
 return;

 case KW_RETRY:
 /*
 * RETRY - ECMA-116: Re-enter protected block.
 *
 * Only valid inside a USE handler. Jumps back
 * to WHEN EXCEPTION IN + 1 (first line of the
 * protected block). The exception frame stays
 * on the stack.
 */
 {
 int i;
 for (i = rt->stack_top - 1; i >= 0; i--) {
 if (rt->stack[i].type ==
 FRAME_EXCEPTION) {
 int when_idx = rt->stack[i]
 .data.exception.when_index;
 rt->next_index = when_idx + 1;
 return;
 }
 }
 error_raise(ERR_WHAT, line_num);
 }
 return;

 case KW_CONTINUE:
 /*
 * CONTINUE - ECMA-116: Resume after error.
 *
 * Only valid inside a USE handler. Jumps to the
 * line after the one that caused the exception.
 * Pops the exception frame.
 */
 {
 int i;
 for (i = rt->stack_top - 1; i >= 0; i--) {
 if (rt->stack[i].type ==
 FRAME_EXCEPTION) {
 int err_idx = rt->stack[i]
 .data.exception.err_index;
 if (err_idx < 0) {
 error_raise(ERR_HOW,
 line_num);
 return;
 }
 /* Pop exception frame */
 rt->stack_top = i;
 /* Resume after error line */
 rt->next_index = err_idx + 1;
 return;
 }
 }
 error_raise(ERR_WHAT, line_num);
 }
 return;

 case KW_POKE:
 /*
 * POKE address, value
 * Write a byte to virtual memory.
 */
 {
 int addr, val, offset;
 addr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 val = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 offset = rt->mem_seg_base + addr;
 if (offset >= 0 &&
 offset < MAX_MEM_SEGMENT) {
 rt->mem_segment[offset] =
 (unsigned char)(val & 0xFF);
 }
 }
 return;

 case KW_PSET:
 /*
 * PSET (x, y) [, color]
 * Set a pixel in the graphics framebuffer.
 */
 {
 int px, py, clr = 15;
 if (!lexer_expect(lex, TOK_LPAREN))
 return;
 px = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 py = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 clr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 gfxbuf_pset(px, py, clr);
 gfxbuf_render();
 }
 return;

 case KW_CIRCLE:
 /*
 * CIRCLE (cx, cy), radius [, color]
 * Draw a circle.
 */
 {
 int cx, cy, r, clr = 15;
 if (!lexer_expect(lex, TOK_LPAREN))
 return;
 cx = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 cy = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 r = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 clr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 gfxbuf_circle(cx, cy, r, clr);
 gfxbuf_render();
 }
 return;

 case KW_PAINT:
 /*
 * PAINT (x, y), fill_color [, border]
 * Flood fill from (x, y).
 */
 {
 int px, py, fill = 15, border = -1;
 if (!lexer_expect(lex, TOK_LPAREN))
 return;
 px = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 py = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 fill = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 border = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (border < 0) border = fill;
 gfxbuf_paint(px, py, fill, border);
 gfxbuf_render();
 }
 return;

 case KW_PALETTE:
 /*
 * PALETTE [attr, color]
 * PALETTE USING array-name
 * Remap palette entries.
 */
 if (lexer_match_keyword(lex,
 KW_USING)) {
 /* PALETTE USING array%() */
 lexer_next(lex);
 lexer_skip_to_end(lex);
 } else if (lex->current.type ==
 TOK_EOF ||
 lex->current.type ==
 TOK_CR ||
 lex->current.type ==
 TOK_COLON) {
 /* Bare PALETTE - reset all */
 gfxbuf_palette(-1, -1);
 } else {
 int attr, clr;
 attr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 clr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 gfxbuf_palette(attr, clr);
 }
 return;

 case KW_PCOPY:
 /*
 * PCOPY src, dst
 * Copy display page. No-op in single-
 * page graphics; consume arguments.
 */
 {
 (void)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 (void)parse_expression(
 lex, rt, line_num);
 }
 return;

 case KW_PRESET:
 /*
 * PRESET (x, y) [, color]
 * Reset pixel to background color.
 * Like PSET but defaults to color 0.
 */
 {
 int px, py, pc = 0;
 if (lex->current.type == TOK_LPAREN)
 lexer_next(lex);
 px = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 py = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_RPAREN)
 lexer_next(lex);
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 pc = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 gfxbuf_pset(px, py, pc);
 }
 return;

 case KW_TYPE:
 /*
 * TYPE name
 * field AS STRING/INTEGER/...
 * END TYPE
 *
 * Define a user type. Scan lines until
 * END TYPE is found.
 */
 {
 char tname[MAX_VAR_NAME_LEN + 1];
 int tlen = 0, ti;
 int tidx;
 UserTypeDef *td;
 ProgramStore *pgm = rt->program;

 /* Get type name */
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 tlen = lex->current.str_length;
 if (tlen > MAX_VAR_NAME_LEN)
 tlen = MAX_VAR_NAME_LEN;
 memcpy(tname,
 lex->current.str_start,
 (size_t)tlen);
 } else if (lex->current.type ==
 TOK_VARIABLE) {
 tname[0] = lex->current.value
 .var_name;
 tlen = 1;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 tname[tlen] = '\0';
 lexer_next(lex);

 if (rt->type_count >=
 MAX_USER_TYPES) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 tidx = rt->type_count;
 td = &rt->user_types[tidx];
 memcpy(td->name, tname,
 (size_t)(tlen + 1));
 td->field_count = 0;

 /* Scan forward for fields */
 ti = rt->current_index + 1;
 while (ti < pgm->count) {
 Lexer fl;
 const char *src;

 src = pgm->lines[ti].text;
 lexer_init(&fl, src);

 /* Skip line number */
 if (fl.current.type == TOK_NUMBER)
 lexer_next(&fl);

 /* Check for END TYPE */
 if (fl.current.type ==
 TOK_KEYWORD &&
 fl.current.value.keyword ==
 KW_END) {
 lexer_next(&fl);
 if (fl.current.type ==
 TOK_KEYWORD &&
 fl.current.value
 .keyword ==
 KW_TYPE) {
 break;
 }
 }

 /* Parse field: name AS type */
 if (fl.current.type ==
 TOK_NAMED_VAR ||
 fl.current.type ==
 TOK_VARIABLE) {
 int fi = td->field_count;
 if (fi < MAX_TYPE_FIELDS) {
 int nl;
 if (fl.current.type ==
 TOK_NAMED_VAR) {
 nl = fl.current
 .str_length;
 if (nl >
 MAX_VAR_NAME_LEN)
 nl =
 MAX_VAR_NAME_LEN;
 memcpy(
 td->fields[fi]
 .name,
 fl.current
 .str_start,
 (size_t)nl);
 td->fields[fi]
 .name[nl] =
 '\0';
 } else {
 td->fields[fi]
 .name[0] =
 fl.current.value
 .var_name;
 td->fields[fi]
 .name[1] =
 '\0';
 }
 td->fields[fi]
 .is_string = 0;
 lexer_next(&fl);
 /* Skip AS keyword */
 if (fl.current.type ==
 TOK_KEYWORD &&
 fl.current.value
 .keyword ==
 KW_AS) {
 lexer_next(&fl);
 /* Check STRING */
 if (fl.current.type
 == TOK_KEYWORD
 &&
 fl.current.value
 .keyword ==
 KW_STRING_FUNC) {
 td->fields[fi]
 .is_string =
 1;
 }
 }
 td->field_count++;
 }
 }
 ti++;
 }

 /* Skip past END TYPE */
 rt->current_index = ti;
 rt->type_count++;
 }
 return;

 case KW_ACCESS:
 /*
 * ACCESS READ|WRITE|READ WRITE
 * Standalone ACCESS is a no-op; it is
 * normally part of OPEN syntax.
 */
 lexer_skip_to_end(lex);
 return;

 default:
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }

 /*
 * If the current token is a variable and LET is optional,
 * treat as a bare assignment (e.g., "A=5").
 */
 if (lex->current.type == TOK_VARIABLE &&
 dialect_get_config()->has_let_optional) {
 parse_let(lex, rt, line_num, 0);
 return;
 }

 /*
 * Named variable bare assignment (e.g., "SCORE=100")
 * when both LET-optional and extended vars are active.
 * Also check for label definitions and implicit SUB calls.
 */
 if (lex->current.type == TOK_NAMED_VAR &&
 dialect_get_config()->has_let_optional) {
 const char *nm = lex->current.str_start;
 int nlen = lex->current.str_length;

 /*
 * Check for label definition (identifier:).
 * If the next token after the name is ':', skip
 * the label and continue with any statements
 * after the colon.
 */
 if (nm != NULL && nlen > 0) {
 /* Peek ahead: save lexer state */
 Lexer saved = *lex;
 lexer_next(lex);
 if (lex->current.type == TOK_COLON) {
 lexer_next(lex); /* consume colon */
 /* Execute rest of line if any */
 if (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 parse_statement(lex, rt, line_num);
 }
 return;
 }
 /* Not a label - restore and continue */
 *lex = saved;
 }

 /*
 * Check for implicit SUB call (SubName args).
 * If the name matches a defined SUB, treat as CALL.
 */
 if (nm != NULL && nlen > 0) {
 SubDef *sd = runtime_find_sub(rt, nm, nlen);
 if (sd != NULL && !sd->is_function) {
 StackFrame frame;
 int i;
 lexer_next(lex); /* consume name */

 /* Push FRAME_SUB */
 frame.type = FRAME_SUB;
 frame.data.sub_call.return_index =
 rt->current_index + 1;
 frame.data.sub_call.sub_index =
 (int)(sd - rt->subs);
 for (i = 0; i < MAX_VARIABLES; i++) {
 frame.data.sub_call.saved_vars[i] =
 rt->variables[i];
 }
 for (i = 0; i < MAX_STRING_VARS; i++) {
 frame.data.sub_call.saved_strvars[i] =
 rt->string_vars[i];
 }
 if (runtime_push(rt, &frame) != 0)
 return;

 /* Parse args (with or without parens) */
 if (lex->current.type == TOK_LPAREN) {
 lexer_next(lex);
 for (i = 0; i < sd->param_count; i++) {
 BValue av;
 if (i > 0) {
 if (lex->current.type !=
 TOK_COMMA) break;
 lexer_next(lex);
 }
 av = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 set_param_by_name(rt,
 sd->params[i], av);
 }
 if (lex->current.type == TOK_RPAREN)
 lexer_next(lex);
 } else {
 for (i = 0; i < sd->param_count; i++) {
 BValue av;
 if (i > 0) {
 if (lex->current.type !=
 TOK_COMMA) break;
 lexer_next(lex);
 }
 av = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 set_param_by_name(rt,
 sd->params[i], av);
 }
 }

 rt->in_sub_index =
 (int)(sd - rt->subs);
 rt->next_index = sd->body_index;
 lexer_skip_to_end(lex);
 return;
 }
 }

 /* Regular assignment */
 parse_let(lex, rt, line_num, 0);
 return;
 }

 /*
 * If the current token is @, treat as array assignment
 * (e.g., "@(0)=5") when LET is optional.
 */
 if (lex->current.type == TOK_AT &&
 dialect_get_config()->has_let_optional) {
 parse_let(lex, rt, line_num, 0);
 return;
 }

 /* String variable bare assignment (e.g., "A$=\"HELLO\"") */
 if (lex->current.type == TOK_STRING_VAR &&
 dialect_get_config()->has_let_optional) {
 parse_let(lex, rt, line_num, 0);
 return;
 }

 /* If we reach here, it's a syntax error */
 if (lex->current.type != TOK_EOF && lex->current.type != TOK_CR) {
 error_raise(ERR_WHAT, line_num);
 }
}

/* --- Expression Parsing ---
 *
 * Expression grammar (standard BASIC precedence):
 *
 * expression = [+|-] term ((+|-) term)*
 * term = factor ((*|/) factor)*
 * factor = number
 * | variable
 * | @(expression)
 * | (expression)
 * | ABS(expression)
 * | RND(expression)
 * | SIZE
 *
 * This gives correct precedence: * and / bind tighter than + and -.
 * Parentheses override precedence.
 *
 * Recursion depth: expression -> term -> factor -> expression (via
 * parentheses or function calls). Maximum depth equals the nesting
 * depth of parentheses in the source, which is bounded by line
 * length (255 chars max -> at most ~127 nesting levels, but in
 * practice programs use 3-5 levels).
 */

/*
 * parse_factor - Parse an atomic expression (highest precedence).
 */
static long parse_factor(Lexer *lex, RuntimeState *rt, int line_num)
{
 long value = 0;

 if (error_occurred()) return 0;

 switch (lex->current.type) {
 case TOK_NUMBER:
 value = lex->current.value.num_value;
 lexer_next(lex);
 return value;

 case TOK_VARIABLE:
 value = runtime_get_var(rt, lex->current.value.var_name);
 lexer_next(lex);
 return value;

 case TOK_NAMED_VAR:
 /* Extended variable - look up by name */
 value = runtime_get_named_var(rt,
 lex->current.str_start, lex->current.str_length);
 lexer_next(lex);
 return value;

 case TOK_AT:
 /* @(expression) - array access */
 {
 long index;
 lexer_next(lex); /* consume @ */
 if (!lexer_expect(lex, TOK_LPAREN)) return 0;
 index = parse_expression(lex, rt, line_num);
 if (error_occurred()) return 0;
 if (!lexer_expect(lex, TOK_RPAREN)) return 0;
 return runtime_get_array(rt, index);
 }

 case TOK_LPAREN:
 /* (expression) - parenthesized sub-expression */
 lexer_next(lex); /* consume ( */
 value = parse_expression(lex, rt, line_num);
 if (error_occurred()) return 0;
 if (!lexer_expect(lex, TOK_RPAREN)) return 0;
 return value;

 case TOK_KEYWORD:
 /*
 * User-defined function dispatch.
 * FN<name>(...) calls are handled before the registry.
 */
 if (lex->current.value.keyword == KW_FN) {
 BValue fnr;
 lexer_next(lex); /* consume FN */
 fnr = eval_user_fn(lex, rt, line_num);
 if (error_occurred()) return 0;
 return bval_to_int(&fnr);
 }

 /*
 * Registry-based function dispatch.
 *
 * Look up the keyword in the function registry. If found,
 * parse arguments, call the handler, and convert the
 * BValue result to long for the integer expression path.
 */
 {
 const FunctionEntry *fn;
 fn = funcreg_find_by_keyword(
 lex->current.value.keyword);
 if (fn != NULL) {
 BValue args[4];
 int argc = 0;
 BValue result;

 lexer_next(lex); /* consume function name */

 if (fn->max_args > 0) {
 /* Parse (arg1, arg2, ...) */
 if (!lexer_expect(lex, TOK_LPAREN)) return 0;
 args[argc] = bval_int(
 parse_expression(lex, rt, line_num));
 if (error_occurred()) return 0;
 argc++;

 while (argc < fn->max_args &&
 lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 args[argc] = bval_int(
 parse_expression(lex, rt, line_num));
 if (error_occurred()) return 0;
 argc++;
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return 0;
 }
 /* else: zero-arg function (SIZE) */

 result = fn->handler(args, argc, (void *)rt);
 if (error_occurred()) return 0;
 return bval_to_int(&result);
 }
 }
 /* Fall through to error */
 error_raise(ERR_WHAT, line_num);
 return 0;

 default:
 error_raise(ERR_WHAT, line_num);
 return 0;
 }
}

/*
 * parse_power - Parse exponentiation (^).
 *
 * power = factor (^ factor)*
 * Note: ^ is right-associative in QBasic, but we implement
 * left-to-right for simplicity (covers 99% of cases).
 */
static long parse_power(Lexer *lex, RuntimeState *rt, int line_num)
{
 long left;
 left = parse_factor(lex, rt, line_num);
 if (error_occurred()) return 0;

 while (lex->current.type == TOK_CARET) {
 long right;
 lexer_next(lex); /* consume ^ */
 right = parse_factor(lex, rt, line_num);
 if (error_occurred()) return 0;
 left = (long)pow((double)left, (double)right);
 }

 return left;
}

/*
 * parse_term - Parse a multiplicative expression.
 *
 * term = power ((*|/|\|MOD) power)*
 */
static long parse_term(Lexer *lex, RuntimeState *rt, int line_num)
{
 long left;
 TokenType op;

 left = parse_power(lex, rt, line_num);
 if (error_occurred()) return 0;

 for (;;) {
 int is_mod = 0;
 int is_intdiv = 0;

 if (lex->current.type == TOK_STAR ||
 lex->current.type == TOK_SLASH) {
 op = lex->current.type;
 } else if (lex->current.type == TOK_BACKSLASH) {
 is_intdiv = 1;
 op = TOK_SLASH; /* placeholder */
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_MOD) {
 is_mod = 1;
 op = TOK_SLASH; /* placeholder */
 } else {
 break;
 }

 lexer_next(lex); /* consume operator */

 {
 long right;
 right = parse_power(lex, rt, line_num);
 if (error_occurred()) return 0;

 if (is_mod) {
 if (right == 0) {
 error_raise(ERR_HOW, line_num);
 return 0;
 }
 left = left % right;
 } else if (is_intdiv) {
 if (right == 0) {
 error_raise(ERR_HOW, line_num);
 return 0;
 }
 left = left / right;
 } else if (op == TOK_STAR) {
 left = left * right;
 } else {
 /* Division - check for divide by zero */
 if (right == 0) {
 error_raise(ERR_HOW, line_num);
 return 0;
 }
 left = left / right;
 }
 }
 }

 return left;
}

/*
 * parse_expression - Parse an additive expression.
 *
 * expression = [+|-|NOT] term ((+|-) term)*
 * ((AND|OR|XOR|EQV|IMP) expr)*
 *
 * The optional leading +/- handles unary plus/minus.
 * NOT is unary prefix (bitwise complement).
 * AND/OR/XOR/EQV/IMP are lowest-precedence binary.
 */
long parse_expression(Lexer *lex, RuntimeState *rt, int line_num)
{
 long left;
 int negate = 0;
 int do_not = 0;

 if (error_occurred()) return 0;

 /* Optional leading sign or NOT */
 if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NOT) {
 do_not = 1;
 lexer_next(lex);
 /* Handle sign after NOT: NOT -1, NOT +5 */
 if (lex->current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 }
 }

 left = parse_term(lex, rt, line_num);
 if (error_occurred()) return 0;

 if (negate) {
 left = -left;
 }
 if (do_not) {
 left = ~left; /* bitwise NOT */
 }

 /* Additive: + - */
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 long right;
 TokenType op = lex->current.type;
 lexer_next(lex); /* consume operator */

 right = parse_term(lex, rt, line_num);
 if (error_occurred()) return 0;

 if (op == TOK_PLUS) {
 left = left + right;
 } else {
 left = left - right;
 }
 }

 /* Logical/bitwise: AND OR XOR EQV IMP */
 while (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;
 long right;
 if (kw != KW_AND && kw != KW_OR &&
 kw != KW_XOR && kw != KW_EQV &&
 kw != KW_IMP) {
 break;
 }
 lexer_next(lex); /* consume operator */

 /* Parse the right side at additive level */
 {
 int rn = 0, rn2 = 0;
 if (lex->current.type == TOK_MINUS) {
 rn = 1; lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NOT) {
 rn2 = 1; lexer_next(lex);
 }
 right = parse_term(lex, rt, line_num);
 if (error_occurred()) return 0;
 if (rn) right = -right;
 if (rn2) right = ~right;

 /* Inner additive loop */
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 long r2;
 TokenType op2 = lex->current.type;
 lexer_next(lex);
 r2 = parse_term(lex, rt, line_num);
 if (error_occurred()) return 0;
 if (op2 == TOK_PLUS)
 right = right + r2;
 else
 right = right - r2;
 }
 }

 switch (kw) {
 case KW_AND: left = left & right; break;
 case KW_OR: left = left | right; break;
 case KW_XOR: left = left ^ right; break;
 case KW_EQV:
 left = ~(left ^ right); break;
 case KW_IMP:
 left = (~left) | right; break;
 default: break;
 }
 }

 return left;
}

/* --- Line Execution ---
 */

/*
 * parser_execute_line - Parse and execute a complete BASIC line.
 *
 * Handles multiple statements separated by the dialect's separator
 * character (';' for PATB). Stops on:
 * - End of line (TOK_EOF)
 * - Error
 * - Flow control that changes execution (GOTO, GOSUB, etc.)
 * set rt->next_index, which signals the caller.
 *
 * The lexer should be initialized on the line text, positioned
 * AFTER the line number (if any). The caller is responsible for
 * skipping the line number.
 */
void parser_execute_line(Lexer *lex, RuntimeState *rt, int line_num)
{
 char sep = dialect_get_separator();

 while (!error_occurred() && lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 int old_next = rt->next_index;

 parse_statement(lex, rt, line_num);

 /* If flow control changed next_index, stop processing
 * this line - the caller will jump to the new target */
 if (rt->next_index != old_next) {
 return;
 }

 /* If program was stopped, don't continue */
 if (!rt->running && rt->stopped) {
 return;
 }

 /* Check for statement separator (dialect-specific) */
 if (lex->current.type == TOK_SEMICOLON && sep == ';') {
 lexer_next(lex); /* consume separator */
 continue;
 }
 if (lex->current.type == TOK_COLON && sep == ':') {
 lexer_next(lex); /* consume separator */
 continue;
 }

 /* Any other token at this point is the end of statements */
 break;
 }
}

/* --- SAVE/LOAD Command Implementations ---
 * These are here rather than in fileio.c because they need access
 * to the parser's token stream for the filename argument. The
 * actual file operations delegate to fileio.c functions.
 */

/*
 * parse_save_cmd - SAVE "filename"
 */
static void parse_save_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char filename[MAX_LINE_LENGTH + 1];

 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /* Copy filename from token (zero-terminate) */
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 fileio_save(&rt->memory->program, filename);
}

/*
 * parse_load_cmd - LOAD "filename"
 */
static void parse_load_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char filename[MAX_LINE_LENGTH + 1];

 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 /* LOAD clears the current program first */
 program_clear(&rt->memory->program);
 runtime_reset(rt);

 fileio_load(&rt->memory->program, filename);
}

/* --- DIM Statement Handler ---
 * Syntax:
 * DIM name(size) - 1D array
 * DIM name(size1,size2) - 2D array
 */
static void parse_dim(Lexer *lex, RuntimeState *rt, int line_num)
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
 }

 if (!lexer_expect(lex, TOK_RPAREN)) return;

 runtime_dim(rt, name, name_len, dim1, dim2, line_num);
 if (error_occurred()) return;

 /* Check for comma -> more arrays to DIM */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); /* consume comma, loop */
 } else {
 break;
 }
 } while (1);
}

/* --- BValue Expression System ---
 * These are the BValue-returning versions of parse_factor, parse_term,
 * and parse_expression. They handle integers, floats, strings, and
 * all functions.
 *
 * The old long-returning versions are preserved for backward
 * compatibility with Phases 1-3 code paths.
 */

/*
 * parse_factor_bval - BValue atom parser.
 *
 * Handles: integers, floats, string literals, variables (A-Z, named),
 * string variables (A$-Z$), @() arrays, DIM array access, parenthesized
 * expressions, and all built-in functions.
 */
static BValue parse_factor_bval(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue val;

 if (error_occurred()) return bval_int(0);

 switch (lex->current.type) {
 case TOK_NUMBER:
 val = bval_int(lex->current.value.num_value);
 lexer_next(lex);
 return val;

 case TOK_FLOAT_LIT:
 val = bval_float(lex->current.value.fval);
 lexer_next(lex);
 return val;

 case TOK_STRING:
 {
 /* String literal - store in pool */
 char *ptr = strpool_store(&rt->strpool,
 lex->current.str_start, lex->current.str_length);
 int slen = lex->current.str_length;
 lexer_next(lex);
 if (ptr == NULL) {
 error_raise(ERR_SORRY, line_num);
 return bval_string(NULL, 0);
 }
 return bval_string(ptr, slen);
 }

 case TOK_VARIABLE:
 {
 char vname = lex->current.value.var_name;
 lexer_next(lex);
 /* Check if this is a DIM array access: A(...) */
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays) {
 DimArray *arr = runtime_find_dim(rt, &vname, 1);
 if (arr != NULL) {
 int idx1, idx2 = 0;
 lexer_next(lex); /* consume ( */
 val = parse_expression_bval(lex, rt, line_num);
 idx1 = (int)bval_to_subscript(&val);
 if (error_occurred()) return bval_int(0);
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 val = parse_expression_bval(lex, rt,
 line_num);
 idx2 = (int)bval_to_subscript(&val);
 if (error_occurred()) return bval_int(0);
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return runtime_get_dim(rt, &vname, 1,
 idx1, idx2, line_num);
 }
 }
 return runtime_get_var_bval(rt, vname);
 }

 case TOK_STRING_VAR:
 {
 char vname = lex->current.value.var_name;
 lexer_next(lex);
 /*
 * Check for DIM string array access: A$(index)
 * The DIM name for single-char string arrays is
 * stored as "A$" (2 chars).
 */
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays) {
 char sname[3];
 sname[0] = vname;
 sname[1] = '$';
 sname[2] = '\0';
 {
 DimArray *arr = runtime_find_dim(
 rt, sname, 2);
 if (arr != NULL) {
 int idx1, idx2 = 0;
 lexer_next(lex); /* consume ( */
 val = parse_expression_bval(
 lex, rt, line_num);
 idx1 = (int)bval_to_subscript(
 &val);
 if (error_occurred())
 return bval_int(0);
 if (lex->current.type ==
 TOK_COMMA) {
 lexer_next(lex);
 val = parse_expression_bval(
 lex, rt, line_num);
 idx2 = (int)bval_to_subscript(
 &val);
 if (error_occurred())
 return bval_int(0);
 }
 if (!lexer_expect(lex,
 TOK_RPAREN))
 return bval_int(0);
 return runtime_get_dim(rt,
 sname, 2, idx1, idx2,
 line_num);
 }
 }
 }
 return runtime_get_string_var(rt, vname);
 }

 case TOK_NAMED_VAR:
 {
 const char *nm = lex->current.str_start;
 int nlen = lex->current.str_length;
 lexer_next(lex);

 /*
 * Check for FUNCTION call.
 * If name(args) matches a FUNCTION def,
 * execute it and return fn_return_value.
 */
 if (lex->current.type == TOK_LPAREN &&
 nm != NULL && nlen > 0) {
 SubDef *sd = runtime_find_sub(
 rt, nm, nlen);
 if (sd != NULL && sd->is_function) {
 StackFrame frame;
 int i;
 int save_idx, save_next;

 /* Push FRAME_SUB */
 frame.type = FRAME_SUB;
 frame.data.sub_call.return_index =
 rt->current_index;
 frame.data.sub_call.sub_index =
 (int)(sd - rt->subs);
 for (i = 0; i < MAX_VARIABLES; i++){
 frame.data.sub_call
 .saved_vars[i] =
 rt->variables[i];
 }
 for (i=0; i<MAX_STRING_VARS; i++){
 frame.data.sub_call
 .saved_strvars[i] =
 rt->string_vars[i];
 }
 if (runtime_push(rt, &frame) != 0)
 return bval_int(0);

 /* Parse args */
 lexer_next(lex); /* consume ( */
 for (i = 0; i < sd->param_count;
 i++) {
 BValue av;
 if (i > 0) {
 if (lex->current.type !=
 TOK_COMMA) break;
 lexer_next(lex);
 }
 av = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 set_param_by_name(rt,
 sd->params[i], av);
 }
 if (lex->current.type == TOK_RPAREN)
 lexer_next(lex);

 /*
 * Execute FUNCTION body inline.
 * Save/restore execution position.
 */
 rt->fn_return_value = bval_int(0);
 rt->in_sub_index =
 (int)(sd - rt->subs);
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
 lexer_init(&fl,
 fline->text);
 if (fl.current.type ==
 TOK_NUMBER)
 lexer_next(&fl);

 rt->current_index = fi;
 rt->next_index = -1;

 parser_execute_line(
 &fl, rt, fln);

 if (error_occurred())
 return bval_int(0);

 /* Check if END SUB/FUNCTION
 * popped our frame */
 if (rt->in_sub_index < 0) {
 break;
 }

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

 /*
 * Check for single-line DEF FN
 * called as FNA(x) in extended-vars
 * mode (where FNA is TOK_NAMED_VAR).
 */
 if (nlen >= 3 &&
 (nm[0] == 'F' || nm[0] == 'f') &&
 (nm[1] == 'N' || nm[1] == 'n')) {
 /* Extract fn letter(s) after FN */
 char fn_ch = nm[2];
 char fn_buf[2];
 UserFunction *ufn;
 if (fn_ch >= 'a' && fn_ch <= 'z')
 fn_ch = (char)(fn_ch - 32);
 fn_buf[0] = fn_ch;
 fn_buf[1] = '\0';
 ufn = runtime_find_fn(rt,
 fn_buf, 1);
 if (ufn != NULL) {
 /* Evaluate inline */
 BValue args[MAX_FN_PARAMS];
 BValue saved[MAX_FN_PARAMS];
 int ac = 0, pi;
 Lexer bl;
 BValue res;
 lexer_next(lex); /* ( */
 if (ufn->param_count > 0) {
 args[ac] =
 parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 ac++;
 while (ac <
 ufn->param_count &&
 lex->current.type
 == TOK_COMMA) {
 lexer_next(lex);
 args[ac] =
 parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 ac++;
 }
 }
 if (!lexer_expect(lex,
 TOK_RPAREN))
 return bval_int(0);
 /* Save & bind params */
 for (pi = 0;
 pi < ufn->param_count;
 pi++) {
 int vi =
 ufn->params[pi] - 'A';
 saved[pi] =
 rt->variables[vi];
 if (pi < ac)
 rt->variables[vi] =
 args[pi];
 }
 /* Eval body */
 lexer_init(&bl, ufn->body);
 res = parse_expression_bval(
 &bl, rt, line_num);
 /* Restore */
 for (pi = 0;
 pi < ufn->param_count;
 pi++) {
 int vi =
 ufn->params[pi] - 'A';
 rt->variables[vi] =
 saved[pi];
 }
 return res;
 }
 }
 }

 /* Check for DIM array access */
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays) {
 DimArray *arr = runtime_find_dim(rt, nm, nlen);
 if (arr != NULL) {
 int idx1, idx2 = 0;
 lexer_next(lex);
 val = parse_expression_bval(lex, rt, line_num);
 idx1 = (int)bval_to_subscript(&val);
 if (error_occurred()) return bval_int(0);
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 val = parse_expression_bval(lex, rt,
 line_num);
 idx2 = (int)bval_to_subscript(&val);
 if (error_occurred()) return bval_int(0);
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return runtime_get_dim(rt, nm, nlen,
 idx1, idx2, line_num);
 }
 }
 /* Check CONST table before named vars */
 {
 int ci;
 for (ci = 0; ci < rt->const_count; ci++){
 int cl = rt->constants[ci].name_len;
 if (cl == nlen) {
 /* Case-insensitive compare */
 int j, match = 1;
 for (j = 0; j < nlen; j++) {
 char a = nm[j];
 char b = rt->constants[ci]
 .name[j];
 if (a >= 'a' && a <= 'z')
 a = (char)(a - 32);
 if (b >= 'a' && b <= 'z')
 b = (char)(b - 32);
 if (a != b) {
 match = 0; break;
 }
 }
 if (match) {
 return rt->constants[ci]
 .value;
 }
 }
 }
 }
 return runtime_get_named_var_bval(rt, nm, nlen);
 }

 case TOK_AT:
 {
 long index;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
 index = parse_expression(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
 return bval_int(runtime_get_array(rt, index));
 }

 case TOK_LPAREN:
 lexer_next(lex);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
 return val;

 case TOK_KEYWORD:
 /*
 * User-defined function dispatch.
 */
 if (lex->current.value.keyword == KW_FN) {
 lexer_next(lex); /* consume FN */
 return eval_user_fn(lex, rt, line_num);
 }

 /*
 * Registry-based BValue function dispatch.
 *
 * Look up the keyword in the function registry. Parse
 * arguments into BValue array using the BValue expression
 * parser (preserving float/string types). Call the handler
 * and return the BValue result directly.
 */
 {
 KeywordId kw = lex->current.value.keyword;
 const FunctionEntry *fn;
 fn = funcreg_find_by_keyword(kw);
 if (fn != NULL) {
 BValue args[4];
 int argc = 0;
 BValue result;

 lexer_next(lex); /* consume function name */

 if (fn->max_args > 0) {
 /* Parse (arg1, arg2, ...) */
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);

 args[argc] = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 argc++;

 while (argc < fn->max_args &&
 lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 args[argc] = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 argc++;
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 }
 /* else: zero-arg function (SIZE) */

 /* Validate argument count */
 if (argc < fn->min_args) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }

 result = fn->handler(args, argc, (void *)rt);
 return result;
 }

 /*
 * TIMER - returns seconds since midnight (float).
 */
 if (kw == KW_TIMER) {
 time_t t;
 struct tm *tm;
 lexer_next(lex);
 t = time(NULL);
 tm = localtime(&t);
 return bval_float(
 (double)(tm->tm_hour * 3600 +
 tm->tm_min * 60 +
 tm->tm_sec));
 }

 /*
 * DATE$ - returns current date as "MM-DD-YYYY".
 * TIME$ - returns current time as "HH:MM:SS".
 */
 if (kw == KW_DATE_FUNC) {
 char buf[16];
 char *ptr;
 time_t t;
 struct tm *tm;
 lexer_next(lex);
 /* $ already consumed by lexer */
 t = time(NULL);
 tm = localtime(&t);
 sprintf(buf, "%02d-%02d-%04d",
 tm->tm_mon + 1, tm->tm_mday,
 tm->tm_year + 1900);
 ptr = strpool_store(&rt->strpool, buf, 10);
 return bval_string(ptr, 10);
 }
 if (kw == KW_TIME_FUNC) {
 char buf[16];
 char *ptr;
 time_t t;
 struct tm *tm;
 lexer_next(lex);
 t = time(NULL);
 tm = localtime(&t);
 sprintf(buf, "%02d:%02d:%02d",
 tm->tm_hour, tm->tm_min,
 tm->tm_sec);
 ptr = strpool_store(&rt->strpool, buf, 8);
 return bval_string(ptr, 8);
 }

 /*
 * CINT(x) - round to integer.
 */
 if (kw == KW_CINT) {
 double v;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 v = bval_to_float(&val);
 return bval_int((long)(v >= 0 ? v+0.5 : v-0.5));
 }

 /*
 * CSNG(x) - convert to single-precision float.
 * CDBL(x) - convert to double-precision float.
 * Both return double since BASIC++ uses double internally.
 */
 if (kw == KW_CSNG || kw == KW_CDBL) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return bval_float(bval_to_float(&val));
 }

 /*
 * CSRLIN - return current cursor row.
 * No parentheses needed (variable-like).
 */
 if (kw == KW_CSRLIN) {
 lexer_next(lex);
 return bval_int((long)rt->cursor_row);
 }

 /*
 * ERL - last error line number.
 * Variable-like (no parentheses).
 */
 if (kw == KW_ERL) {
 lexer_next(lex);
 return bval_int((long)rt->last_err_line);
 }

 /*
 * ERR - last error code.
 * Variable-like (no parentheses).
 */
 if (kw == KW_ERR_VAR) {
 lexer_next(lex);
 return bval_int((long)rt->last_err_code);
 }

 /*
 * EXTERR(n) - DOS extended error.
 * Returns 0 (not applicable on modern OS).
 */
 if (kw == KW_EXTERR) {
 lexer_next(lex);
 if (lex->current.type == TOK_LPAREN) {
 lexer_next(lex);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 }
 return bval_int(0);
 }

 /*
 * ERDEV - device error code.
 * Returns 0 (not applicable).
 */
 if (kw == KW_ERDEV) {
 lexer_next(lex);
 return bval_int(0);
 }

 /*
 * FRE(n) - Free memory query.
 *
 * GW-BASIC compatible:
 * FRE(0) = free string space
 * FRE("") = free string space
 * FRE(x$) = free string space
 * FRE(-1) = free stack space
 * FRE(-2) = free array/variable space
 *
 * BASIC++ extension:
 * FRE(-3) = variable pool free
 * FRE(n) for n>0 = total free (all pools)
 */
 if (kw == KW_FRE) {
 long arg;
 long result;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 /* Accept string arg: FRE("") or FRE(x$) */
 if (lex->current.type == TOK_STRING ||
 lex->current.type == TOK_STRING_VAR) {
 lexer_next(lex);
 arg = 0;
 } else {
 arg = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 if (arg == 0) {
 /* GW-BASIC: free string space */
 result = rt->strpool.size
 - rt->strpool.used;
 } else if (arg == -1) {
 /* GW-BASIC: free stack space */
 result = mem_pool_available(
 &rt->memory->scratch);
 } else if (arg == -2) {
 /* GW-BASIC: free array/var space */
 result = mem_pool_available(
 &rt->memory->variable);
 } else if (arg == -3) {
 /* BASIC++ ext: variable pool */
 result = mem_pool_available(
 &rt->memory->variable);
 } else {
 /* Total free (all pools) */
 result = mem_pool_available(
 &rt->memory->variable)
 + (rt->strpool.size
 - rt->strpool.used)
 + mem_pool_available(
 &rt->memory->scratch);
 }
 return bval_int(result);
 }

 /*
 * INP(port) - Read I/O port.
 * No direct port access; returns 0.
 */
 if (kw == KW_INP) {
 long port;
 int paddr;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 port = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 /*
 * Read from virtual memory segment.
 * On memory-mapped platforms (C64,
 * Atari, Apple, etc.) I/O ports live
 * in the address space, so INP and
 * PEEK are equivalent. On x86 (MSDOS)
 * the port space is separate, but we
 * map it into the same 64K array for
 * compatibility.
 */
 paddr = (int)(port & 0xFFFF);
 if (paddr >= 0 &&
 paddr < MAX_MEM_SEGMENT)
 return bval_int(
 (long)rt->mem_segment[paddr]);
 return bval_int(0);
 }

 /*
 * SHELL$(command$) - Capture command output.
 *
 * Runs the command via popen/_ popen and
 * returns stdout as a string. Max 32K.
 */
 if (kw == KW_SHELL) {
 BValue sv;
 char cmd[512];
 int cl;
 static char outbuf[32768];
 int outlen = 0;
 FILE *pp;
 char *poolbuf;

 lexer_next(lex);

 /* Expect $( */
 if (lex->current.type != TOK_LPAREN) {
 /* No parens = not SHELL$, error */
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 lexer_next(lex);

 sv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 if (!bval_is_string(&sv)) {
 error_raise(ERR_WHAT, line_num);
 return bval_string(NULL, 0);
 }

 cl = sv.v.sval.length;
 if (cl > 510) cl = 510;
 if (sv.v.sval.data)
 memcpy(cmd, sv.v.sval.data,
 (size_t)cl);
 cmd[cl] = '\0';

#ifdef _WIN32
 pp = _popen(cmd, "r");
#else
 pp = popen(cmd, "r");
#endif
 if (pp == NULL)
 return bval_string(NULL, 0);

 while (outlen < 32760) {
 int ch = fgetc(pp);
 if (ch == EOF) break;
 outbuf[outlen++] = (char)ch;
 }

#ifdef _WIN32
 rt->last_shell_exitcode =
 _pclose(pp);
#else
 rt->last_shell_exitcode =
 pclose(pp);
#endif
 /* Strip trailing newline */
 while (outlen > 0 &&
 (outbuf[outlen - 1] == '\n' ||
 outbuf[outlen - 1] == '\r'))
 outlen--;

 poolbuf = strpool_alloc(
 &rt->strpool, outlen);
 if (poolbuf && outlen > 0)
 memcpy(poolbuf, outbuf,
 (size_t)outlen);
 return bval_string(poolbuf, outlen);
 }

 /*
 * ERRORLEVEL - Return last SHELL exit code.
 * Used as a pseudo-variable in expressions.
 */
 if (kw == KW_ERRORLEVEL) {
 lexer_next(lex);
 return bval_int(
 (long)rt->last_shell_exitcode);
 }

 /*
 * LOC(n) - File position.
 * Returns current byte position in file.
 */
 if (kw == KW_LOC) {
 long chan;
 long result = 0;
 FILE *fp;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 chan = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 fp = fileio_get_fp((int)chan);
 if (fp) {
 result = (long)ftell(fp);
 if (result < 0) result = 0;
 }
 return bval_int(result);
 }

 /*
 * LPOS(n) - Printer head position.
 * Returns column position on printer.
 * No printer support; returns 0.
 */
 if (kw == KW_LPOS) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return bval_int(0);
 }

 /*
 * POS(x) - Return current cursor column.
 * Argument is a dummy (GW-BASIC compat).
 */
 if (kw == KW_POS_FUNC) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return bval_int(
 (long)rt->cursor_col);
 }

 /*
 * PMAP(coordinate, function)
 * Map between physical and view coords.
 * Stub: returns the input coordinate.
 */
 if (kw == KW_PMAP) {
 long coord, pmap_fn;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 coord = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 pmap_fn = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 (void)pmap_fn;
 return bval_int(coord);
 }

 /*
 * PLAY(n) - Return number of notes in
 * background music buffer.
 * No sound hardware; always returns 0.
 */
 if (kw == KW_PLAY) {
 lexer_next(lex);
 if (lex->current.type == TOK_LPAREN) {
 lexer_next(lex);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex,
 TOK_RPAREN))
 return bval_int(0);
 return bval_int(0);
 }
 /* If no parens, fall through to
 * statement PLAY handling */
 return bval_int(0);
 }

 /*
 * STICK(n) - Return joystick position.
 * n=0: x of joystick A, n=1: y of A
 * n=2: x of joystick B, n=3: y of B
 * No joystick hardware; always returns 0.
 */
 if (kw == KW_STICK) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return bval_int(0);
 }

 /*
 * USR(n) - Call machine language routine.
 * In GW-BASIC, calls a user assembly routine
 * at the DEF USR address. No machine code
 * execution in this interpreter; consume
 * the argument and return 0.
 */
 if (kw == KW_USR) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return bval_int(0);
 }

 /*
 * VARPTR(var) - Return pointer to variable.
 * In GW-BASIC, returns the memory address
 * of a variable. We return a pseudo-index
 * based on the variable name (A=1..Z=26).
 */
 if (kw == KW_VARPTR) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 if (lex->current.type == TOK_VARIABLE) {
 long idx = lex->current
 .value.var_name
 - 'A' + 1;
 lexer_next(lex);
 if (!lexer_expect(lex,
 TOK_RPAREN))
 return bval_int(0);
 return bval_int(idx);
 }
 /* Named var or string var */
 lexer_skip_to_end(lex);
 return bval_int(0);
 }

 /*
 * VARPTR$(var) - Return string pointer.
 * Returns a string representation of
 * the variable pointer. Stub: returns
 * empty string.
 */
 if (kw == KW_VARPTR_STR) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_string("", 0);
 /* Consume variable argument */
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_string("", 0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_string("", 0);
 return bval_string("", 0);
 }

 /*
 * SCREEN(row, col [, flag])
 * Read character or attribute at screen pos.
 * flag=0 or omitted: return ASCII code.
 * flag=1: return color attribute.
 * No screen buffer; returns 32 (space).
 */
 if (kw == KW_SCREEN) {
 lexer_next(lex);
 if (lex->current.type == TOK_LPAREN) {
 lexer_next(lex);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(32);
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(32);
 /* Optional 3rd arg (flag) */
 if (lex->current.type ==
 TOK_COMMA) {
 lexer_next(lex);
 (void)parse_expression(
 lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 }
 if (!lexer_expect(lex,
 TOK_RPAREN))
 return bval_int(32);
 return bval_int(32);
 }
 /* No parens = SCREEN statement,
 * fall through */
 return bval_int(0);
 }

 /*
 * MKI$(n) - Pack integer into 2-byte string.
 * MKS$(n) - Pack single into 4-byte string.
 * MKD$(n) - Pack double into 8-byte string.
 * Used with FIELD/PUT for random-access files.
 */
 if (kw == KW_MKI_FUNC ||
 kw == KW_MKS_FUNC ||
 kw == KW_MKD_FUNC) {
 double mkval;
 char buf[8];
 int blen;
 char *ptr;

 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_string("", 0);
 {
 BValue mkarg;
 mkarg = parse_expression_bval(
 lex, rt, line_num);
 mkval = bval_to_float(&mkarg);
 }
 if (error_occurred())
 return bval_string("", 0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_string("", 0);

 if (kw == KW_MKI_FUNC) {
 /* 2-byte integer (little-endian) */
 short sv = (short)(long)mkval;
 memcpy(buf, &sv, 2);
 blen = 2;
 } else if (kw == KW_MKS_FUNC) {
 /* 4-byte single float */
 float fv = (float)mkval;
 memcpy(buf, &fv, 4);
 blen = 4;
 } else {
 /* 8-byte double */
 memcpy(buf, &mkval, 8);
 blen = 8;
 }
 ptr = strpool_store(
 &rt->strpool, buf, blen);
 if (ptr)
 return bval_string(ptr, blen);
 return bval_string("", 0);
 }

 /*
 * INPUT$(n [, #channel])
 * Read n characters from keyboard or file.
 * From keyboard: reads n chars without echo.
 * From file: reads n bytes from channel.
 */
 if (kw == KW_INPUT_FUNC) {
 long nchars;
 int chan = 0;
 char buf[256];
 int i;
 char *ptr;
 BValue sv;

 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_string("", 0);
 nchars = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_string("", 0);
 if (nchars < 1) nchars = 1;
 if (nchars > 255) nchars = 255;
 /* Optional channel: , #n */
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred())
 return bval_string("", 0);
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_string("", 0);

 if (chan > 0) {
 /* File: read n bytes */
 FILE *fp = fileio_get_fp(chan);
 for (i = 0; i < (int)nchars; i++) {
 int ch;
 if (!fp) break;
 ch = fgetc(fp);
 if (ch == EOF) break;
 buf[i] = (char)ch;
 }
 } else {
 /* Keyboard: read n chars */
 for (i = 0; i < (int)nchars; i++) {
 int ch = getchar();
 if (ch == EOF) break;
 buf[i] = (char)ch;
 }
 }
 buf[i] = '\0';
 ptr = strpool_store(&rt->strpool,
 buf, i);
 if (!ptr) {
 error_raise(ERR_SORRY, line_num);
 return bval_string("", 0);
 }
 sv.type = VAL_STRING;
 sv.v.sval.data = ptr;
 sv.v.sval.length = i;
 return sv;
 }

 /*
 * IOCTL$(#n) - Read device control string.
 * Device-specific; returns empty string.
 */
 if (kw == KW_IOCTL_FUNC) {
 /*
 * IOCTL$(#n) - Return device status.
 * Returns the channel mode as a string.
 */
 int chan;
 int cmode;
 const char *st;
 char *p;
 int sl;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_string("", 0);
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred())
 return bval_string("", 0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_string("", 0);
 cmode = fileio_get_channel_mode(chan);
 switch (cmode) {
 case FCHAN_INPUT: st="I"; break;
 case FCHAN_OUTPUT: st="O"; break;
 case FCHAN_APPEND: st="A"; break;
 case FCHAN_RANDOM: st="R"; break;
 case FCHAN_BINARY: st="B"; break;
 default: st=""; break;
 }
 sl = (int)strlen(st);
 p = strpool_store(&rt->strpool,
 st, sl);
 return bval_string(p, sl);
 }

 /*
 * CVI(string$) - 2-byte string to integer.
 */
 if (kw == KW_CVI) {
 BValue sv;
 int cvi_val;
 unsigned char lo, hi;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 sv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 if (!bval_is_string(&sv) ||
 sv.v.sval.data == NULL ||
 sv.v.sval.length < 2) {
 return bval_int(0);
 }
 lo = (unsigned char)sv.v.sval.data[0];
 hi = (unsigned char)sv.v.sval.data[1];
 cvi_val = (int)(lo | (hi << 8));
 if (cvi_val > 32767) cvi_val -= 65536;
 return bval_int((long)cvi_val);
 }

 /*
 * CVS(string$) - 4-byte string to float.
 */
 if (kw == KW_CVS) {
 BValue sv;
 float f;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 sv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 if (!bval_is_string(&sv) ||
 sv.v.sval.data == NULL ||
 sv.v.sval.length < 4) {
 return bval_int(0);
 }
 memcpy(&f, sv.v.sval.data,
 sizeof(float));
 return bval_float((double)f);
 }

 /*
 * CVD(string$) - 8-byte string to double.
 */
 if (kw == KW_CVD) {
 BValue sv;
 double d;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 sv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 if (!bval_is_string(&sv) ||
 sv.v.sval.data == NULL ||
 sv.v.sval.length < 8) {
 return bval_int(0);
 }
 memcpy(&d, sv.v.sval.data,
 sizeof(double));
 return bval_float(d);
 }

 /*
 * INKEY$ - non-blocking keyboard read.
 * Returns empty string if no key, or 1-char string.
 */
 if (kw == KW_INKEY) {
 int ch;
 lexer_next(lex);
 ch = vdev_inkey();
 if (ch > 0) {
 char kb[2];
 char *ptr;
 kb[0] = (char)ch;
 kb[1] = '\0';
 ptr = strpool_store(&rt->strpool, kb, 1);
 return bval_string(ptr, 1);
 }
 return bval_string(NULL, 0);
 }

 /*
 * CONST lookup: check if keyword matches a
 * stored constant name.
 */
 {
 const char *knm = lex->current.str_start;
 int knl = lex->current.str_length;
 int ci;
 if (knm != NULL && knl > 0) {
 for (ci = 0; ci < rt->const_count;
 ci++) {
 if (rt->constants[ci].name_len ==
 knl &&
 memcmp(rt->constants[ci].name,
 knm, (size_t)knl)==0) {
 lexer_next(lex);
 return rt->constants[ci].value;
 }
 }
 }
 }

 /*
 * LCASE$(s$) - lowercase.
 * UCASE$(s$) - uppercase.
 * LTRIM$(s$) - trim left spaces.
 * RTRIM$(s$) - trim right spaces.
 */
 if (kw == KW_LCASE || kw == KW_UCASE ||
 kw == KW_LTRIM || kw == KW_RTRIM) {
 char buf[256];
 char *ptr;
 const char *s;
 int slen, i;

 lexer_next(lex);
 /* $ already consumed by lexer */
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 s = val.v.sval.data;
 slen = val.v.sval.length;
 if (s == NULL) { s = ""; slen = 0; }
 if (slen > 255) slen = 255;

 if (kw == KW_LCASE) {
 for (i = 0; i < slen; i++)
 buf[i] = (char)tolower(
 (unsigned char)s[i]);
 ptr = strpool_store(&rt->strpool,
 buf, slen);
 return bval_string(ptr, slen);
 }
 if (kw == KW_UCASE) {
 for (i = 0; i < slen; i++)
 buf[i] = (char)toupper(
 (unsigned char)s[i]);
 ptr = strpool_store(&rt->strpool,
 buf, slen);
 return bval_string(ptr, slen);
 }
 if (kw == KW_LTRIM) {
 i = 0;
 while (i < slen && s[i] == ' ') i++;
 ptr = strpool_store(&rt->strpool,
 s + i, slen - i);
 return bval_string(ptr, slen - i);
 }
 if (kw == KW_RTRIM) {
 i = slen;
 while (i > 0 && s[i-1] == ' ') i--;
 ptr = strpool_store(&rt->strpool,
 s, i);
 return bval_string(ptr, i);
 }
 }

 /* Unknown keyword in expression context */
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }

 default:
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
}

/*
 * parse_power_bval - BValue exponentiation (^) parser.
 */
static BValue parse_power_bval(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue left;
 left = parse_factor_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 while (lex->current.type == TOK_CARET) {
 BValue right;
 double base_d, exp_d;
 lexer_next(lex); /* consume ^ */
 right = parse_factor_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 base_d = bval_to_float(&left);
 exp_d = bval_to_float(&right);
 left = bval_float(pow(base_d, exp_d));
 }

 return left;
}

/*
 * parse_term_bval - BValue multiplicative expression parser.
 *
 * term = power ((*|/|\|MOD) power)*
 */
static BValue parse_term_bval(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue left;
 TokenType op;

 left = parse_power_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 for (;;) {
 int is_mod = 0;
 int is_intdiv = 0;

 if (lex->current.type == TOK_STAR ||
 lex->current.type == TOK_SLASH) {
 op = lex->current.type;
 } else if (lex->current.type == TOK_BACKSLASH) {
 is_intdiv = 1;
 op = TOK_SLASH;
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_MOD) {
 is_mod = 1;
 op = TOK_SLASH; /* placeholder */
 } else {
 break;
 }

 lexer_next(lex);

 {
 BValue right;
 right = parse_power_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 if (is_mod) {
 left = bval_mod(&left, &right, line_num);
 } else if (is_intdiv) {
 /* Integer division: truncate both to int */
 long a = bval_to_int(&left);
 long b = bval_to_int(&right);
 if (b == 0) {
 error_raise(ERR_HOW, line_num);
 return bval_int(0);
 }
 left = bval_int(a / b);
 } else if (op == TOK_STAR) {
 left = bval_mul(&left, &right, line_num);
 } else {
 left = bval_div(&left, &right, line_num);
 }
 if (error_occurred()) return bval_int(0);
 }
 }

 return left;
}

/*
 * parse_expression_bval - BValue additive expression parser.
 *
 * expression = [+|-|NOT] term ((+|-) term)*
 * ((AND|OR|XOR|EQV|IMP) expr)*
 *
 * String concatenation: when both operands are strings and the
 * operator is +, performs string concatenation instead of addition.
 */
BValue parse_expression_bval(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue left;
 int negate = 0;
 int do_not = 0;

 if (error_occurred()) return bval_int(0);

 /* Optional leading sign or NOT */
 if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NOT) {
 do_not = 1;
 lexer_next(lex);
 /* Handle sign after NOT: NOT -1, NOT +5 */
 if (lex->current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 }
 }

 left = parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 if (negate) {
 left = bval_neg(&left, line_num);
 if (error_occurred()) return bval_int(0);
 }
 if (do_not) {
 long v = bval_to_int(&left);
 left = bval_int(~v);
 }

 /* Additive: + - */
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 BValue right;
 TokenType op = lex->current.type;
 lexer_next(lex);

 right = parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 if (op == TOK_PLUS) {
 /* Check for string concatenation */
 if (bval_is_string(&left) && bval_is_string(&right)) {
 /*
 * ECMA-55 does not support string concatenation.
 * In strict mode, reject it. In union/normal
 * mode, allow it for GW-BASIC/QBasic compat.
 */
 if (dialect_is_strict()) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 left = bval_concat(&left, &right, line_num,
 &rt->strpool);
 } else {
 left = bval_add(&left, &right, line_num);
 }
 } else {
 left = bval_sub(&left, &right, line_num);
 }
 if (error_occurred()) return bval_int(0);
 }

 /*
 * Comparison operators: = < > <= >= <>
 * These return -1 for true, 0 for false (QBasic convention).
 * Precedence: between additive and logical.
 */
 if (lex->current.type == TOK_EQUALS ||
 lex->current.type == TOK_LT ||
 lex->current.type == TOK_GT ||
 lex->current.type == TOK_LT_EQ ||
 lex->current.type == TOK_GT_EQ ||
 lex->current.type == TOK_NOT_EQ) {
 TokenType cmp_op = lex->current.type;
 BValue right;
 int result = 0;

 lexer_next(lex); /* consume comparison operator */

 /* Parse right side: unary + additive */
 {
 int rn = 0;
 if (lex->current.type == TOK_MINUS) {
 rn = 1; lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 }
 right = parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (rn) {
 right = bval_neg(&right, line_num);
 if (error_occurred()) return bval_int(0);
 }
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 BValue r2;
 TokenType op2 = lex->current.type;
 lexer_next(lex);
 r2 = parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (op2 == TOK_PLUS)
 right = bval_add(&right, &r2, line_num);
 else
 right = bval_sub(&right, &r2, line_num);
 if (error_occurred()) return bval_int(0);
 }
 }

 /* Compare left and right */
 if (bval_is_string(&left) && bval_is_string(&right)) {
 const char *ld = left.v.sval.data;
 int ll = left.v.sval.length;
 const char *rd = right.v.sval.data;
 int rl = right.v.sval.length;
 int cmp, minlen;
 if (ld == NULL) { ld = ""; ll = 0; }
 if (rd == NULL) { rd = ""; rl = 0; }
 minlen = ll < rl ? ll : rl;
 cmp = memcmp(ld, rd, (size_t)minlen);
 if (cmp == 0) {
 if (ll < rl) cmp = -1;
 else if (ll > rl) cmp = 1;
 }
 switch (cmp_op) {
 case TOK_EQUALS: result=(cmp==0); break;
 case TOK_LT: result=(cmp<0); break;
 case TOK_GT: result=(cmp>0); break;
 case TOK_LT_EQ: result=(cmp<=0); break;
 case TOK_GT_EQ: result=(cmp>=0); break;
 case TOK_NOT_EQ:result=(cmp!=0); break;
 default: break;
 }
 } else if (left.type == VAL_FLOAT ||
 right.type == VAL_FLOAT) {
 double lv = bval_to_float(&left);
 double rv = bval_to_float(&right);
 switch (cmp_op) {
 case TOK_EQUALS: result=(lv==rv); break;
 case TOK_LT: result=(lv<rv); break;
 case TOK_GT: result=(lv>rv); break;
 case TOK_LT_EQ: result=(lv<=rv); break;
 case TOK_GT_EQ: result=(lv>=rv); break;
 case TOK_NOT_EQ:result=(lv!=rv); break;
 default: break;
 }
 } else {
 long lv = bval_to_int(&left);
 long rv = bval_to_int(&right);
 switch (cmp_op) {
 case TOK_EQUALS: result=(lv==rv); break;
 case TOK_LT: result=(lv<rv); break;
 case TOK_GT: result=(lv>rv); break;
 case TOK_LT_EQ: result=(lv<=rv); break;
 case TOK_GT_EQ: result=(lv>=rv); break;
 case TOK_NOT_EQ:result=(lv!=rv); break;
 default: break;
 }
 }
 left = bval_int(result ? -1 : 0);
 }

 /* Logical/bitwise: AND OR XOR EQV IMP */
 while (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;
 long lv, rv;
 if (kw != KW_AND && kw != KW_OR &&
 kw != KW_XOR && kw != KW_EQV &&
 kw != KW_IMP) {
 break;
 }
 lexer_next(lex); /* consume operator */

 /* Parse right side as full additive expr */
 {
 BValue right;
 int rn = 0, rn2 = 0;
 if (lex->current.type == TOK_MINUS) {
 rn = 1; lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NOT) {
 rn2 = 1; lexer_next(lex);
 }
 right = parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (rn) {
 right = bval_neg(&right, line_num);
 if (error_occurred()) return bval_int(0);
 }
 if (rn2) {
 long v = bval_to_int(&right);
 right = bval_int(~v);
 }

 /* Inner additive loop */
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 BValue r2;
 TokenType op2 = lex->current.type;
 lexer_next(lex);
 r2 = parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (op2 == TOK_PLUS)
 right = bval_add(&right, &r2, line_num);
 else
 right = bval_sub(&right, &r2, line_num);
 if (error_occurred()) return bval_int(0);
 }

 /* Inner comparison check */
 if (lex->current.type == TOK_EQUALS ||
 lex->current.type == TOK_LT ||
 lex->current.type == TOK_GT ||
 lex->current.type == TOK_LT_EQ ||
 lex->current.type == TOK_GT_EQ ||
 lex->current.type == TOK_NOT_EQ) {
 TokenType cop = lex->current.type;
 BValue rr;
 int cmp_res = 0;
 int rn3 = 0;
 lexer_next(lex);
 if (lex->current.type == TOK_MINUS) {
 rn3 = 1; lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 }
 rr = parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (rn3) {
 rr = bval_neg(&rr, line_num);
 if (error_occurred()) return bval_int(0);
 }
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 BValue r3;
 TokenType op3 = lex->current.type;
 lexer_next(lex);
 r3 = parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (op3 == TOK_PLUS)
 rr = bval_add(&rr, &r3, line_num);
 else
 rr = bval_sub(&rr, &r3, line_num);
 if (error_occurred()) return bval_int(0);
 }
 if (right.type == VAL_FLOAT ||
 rr.type == VAL_FLOAT) {
 double lf = bval_to_float(&right);
 double rf = bval_to_float(&rr);
 switch (cop) {
 case TOK_EQUALS: cmp_res=(lf==rf); break;
 case TOK_LT: cmp_res=(lf<rf); break;
 case TOK_GT: cmp_res=(lf>rf); break;
 case TOK_LT_EQ: cmp_res=(lf<=rf); break;
 case TOK_GT_EQ: cmp_res=(lf>=rf); break;
 case TOK_NOT_EQ:cmp_res=(lf!=rf); break;
 default: break;
 }
 } else {
 long li = bval_to_int(&right);
 long ri = bval_to_int(&rr);
 switch (cop) {
 case TOK_EQUALS: cmp_res=(li==ri); break;
 case TOK_LT: cmp_res=(li<ri); break;
 case TOK_GT: cmp_res=(li>ri); break;
 case TOK_LT_EQ: cmp_res=(li<=ri); break;
 case TOK_GT_EQ: cmp_res=(li>=ri); break;
 case TOK_NOT_EQ:cmp_res=(li!=ri); break;
 default: break;
 }
 }
 right = bval_int(cmp_res ? -1 : 0);
 }

 rv = bval_to_int(&right);
 }

 lv = bval_to_int(&left);
 switch (kw) {
 case KW_AND: left = bval_int(lv & rv); break;
 case KW_OR: left = bval_int(lv | rv); break;
 case KW_XOR: left = bval_int(lv ^ rv); break;
 case KW_EQV: left = bval_int(~(lv ^ rv)); break;
 case KW_IMP: left = bval_int((~lv) | rv); break;
 default: break;
 }
 }

 return left;
}


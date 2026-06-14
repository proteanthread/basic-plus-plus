/*
 * ---
 * BASIC++ Interpreter - parser_streamio.c
 * ---
 *
 * Stream I/O & printer output commands.
 *
 * LPRINT, LLIST, WRITE.
 *
 * ---
 */

#include "parser_internal.h"

/*
 * pi_parse_lprint - Handle LPRINT command.
 */
void pi_parse_lprint(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_llist - Handle LLIST command.
 */
void pi_parse_llist(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_write - Handle WRITE command.
 */
void pi_parse_write(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}


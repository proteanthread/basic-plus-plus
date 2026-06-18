 // ---
 // BASIC++ Interpreter - parser_streamio.c
 // ---
 //
 // Stream I/O & printer output commands.
 //
 // LPRINT, LLIST, WRITE.
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

 // pi_parse_lprint - Handle LPRINT command.
void pi_parse_lprint(Lexer *lex, RuntimeState *rt, int line_num)
{
 // LPRINT USING "format"; expr [; expr...]
 // LPRINT [expr] [; expr...] [;]
 // Print to stderr (portable printer substitute).

 // Check for LPRINT USING
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_USING) {
 const char *fmt;
 int flen;
 lexer_next(lex); // consume USING

 if (lex->current.type == TOK_STRING) {
 fmt = lex->current.str_start;
 flen = lex->current.str_length;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 // Consume semicolon or comma after format
 if (lex->current.type == TOK_SEMICOLON)
 lexer_next(lex);
 else if (lex->current.type == TOK_COMMA)
 lexer_next(lex);

 format_using_process(stderr, fmt, flen,
 lex, rt, line_num);
 fprintf(stderr, "\n");
 fflush(stderr);
 return;
 }

 // Regular LPRINT (no USING)
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

 // pi_parse_llist - Handle LLIST command.
void pi_parse_llist(Lexer *lex, RuntimeState *rt, int line_num)
{
 // LLIST [start] [-] [end]
 // List program to printer. In our
 // terminal environment, same as LIST.
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
 // Reuse LIST output
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

 // pi_parse_write - Handle WRITE command.
void pi_parse_write(Lexer *lex, RuntimeState *rt, int line_num)
{
 // WRITE [#n,] expr [, expr ...]
 // Output data items separated by commas.
 // Strings are enclosed in double quotes.
 // A newline is printed at the end.
 //
 // WRITE (no args) prints a blank line.
 // WRITE #n, ... writes to file channel.
 {
 int wfile = 0; // file channel
 int wfirst = 1; // first item?

 // WRITE #n, ...
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex);
 wfile = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 }

 // Parse expression list
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type !=
 TOK_COLON) {

 // Print comma between items
 if (!wfirst) {
 if (wfile > 0)
 fileio_print(wfile,
 ",", line_num);
 else
 printf(",");
 }
 wfirst = 0;

 // String literal or expression
 if (lex->current.type ==
 TOK_STRING) {
 // Print quoted string
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
 // Numeric expression
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

 // Consume comma separator
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 else
 break;
 }

 // Newline at end
 if (wfile > 0)
 fileio_print_newline(wfile,
 line_num);
 else
 printf("\n");
 }
 return;
}

 // pi_parse_display - Handle DISPLAY command.
 //
 // DISPLAY "filename"
 //
 // BBC BASIC-style file-to-screen output. Reads a file and
 // outputs its contents to stdout. For text files, prints
 // each line. This is a file display command, NOT a synonym
 // for PRINT.
 //
 // Usage:
 //   DISPLAY "readme.txt"    - show file on screen
 //   DISPLAY "data.csv"      - show CSV on screen
void pi_parse_display(Lexer *lex, RuntimeState *rt, int line_num)
{
 char fname[MAX_LINE_LENGTH + 1];
 FILE *fp;
 char line_buf[1024];
 int flen;

 (void)rt;

 // Expect filename
 if (lex->current.type == TOK_STRING) {
 flen = lex->current.str_length;
 if (flen > MAX_LINE_LENGTH) flen = MAX_LINE_LENGTH;
 memcpy(fname, lex->current.str_start, (size_t)flen);
 fname[flen] = '\0';
 lexer_next(lex);
 } else {
 BValue v = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&v) && v.v.sval.data) {
 flen = v.v.sval.length;
 if (flen > MAX_LINE_LENGTH) flen = MAX_LINE_LENGTH;
 memcpy(fname, v.v.sval.data, (size_t)flen);
 fname[flen] = '\0';
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }

 // Security check
 if (security_check(SECOP_FILE_READ, line_num))
 return;

 fp = fopen(fname, "r");
 if (fp == NULL) {
 error_raise(ERR_SORRY, line_num);
 return;
 }

 // Output file contents to stdout
 while (fgets(line_buf, sizeof(line_buf), fp) != NULL) {
 printf("%s", line_buf);
 }
 fclose(fp);
}

 // pi_parse_type_cmd - Handle TYPE command.
 //
 // TYPE "filename"
 // TYPE USING "format"; "filename"
 //
 // Read file input, then output to terminal/console/printer.
 // Without USING: same as DISPLAY (show file on screen).
 // With USING: read file lines and apply format specifiers,
 // then output formatted text to stdout.
 //
 // Usage:
 //   TYPE "data.txt"                  - show file
 //   TYPE USING "###.##"; "data.csv"  - format each CSV field
void pi_parse_type_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char fname[MAX_LINE_LENGTH + 1];
 FILE *fp;
 char line_buf[1024];
 int flen;

 // Check for TYPE USING
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_USING) {
 const char *fmt;
 int fmt_len;
 Lexer line_lex;

 lexer_next(lex); // consume USING

 // Parse format string
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 fmt = lex->current.str_start;
 fmt_len = lex->current.str_length;
 lexer_next(lex);

 // Consume separator
 if (lex->current.type == TOK_SEMICOLON ||
 lex->current.type == TOK_COMMA)
 lexer_next(lex);

 // Parse filename
 if (lex->current.type == TOK_STRING) {
 flen = lex->current.str_length;
 if (flen > MAX_LINE_LENGTH) flen = MAX_LINE_LENGTH;
 memcpy(fname, lex->current.str_start, (size_t)flen);
 fname[flen] = '\0';
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 if (security_check(SECOP_FILE_READ, line_num))
 return;

 fp = fopen(fname, "r");
 if (fp == NULL) {
 error_raise(ERR_SORRY, line_num);
 return;
 }

 // Read each line, lex it, apply format
 while (fgets(line_buf, sizeof(line_buf), fp) != NULL) {
 // Strip trailing newline
 int ll = (int)strlen(line_buf);
 while (ll > 0 && (line_buf[ll-1] == '\n' ||
 line_buf[ll-1] == '\r'))
 ll--;
 line_buf[ll] = '\0';

 // Create a temporary lexer for the line data
 lexer_init(&line_lex, line_buf);
 format_using_process(stdout, fmt, fmt_len,
 &line_lex, rt, line_num);
 printf("\n");
 }
 fclose(fp);
 return;
 }

 // Plain TYPE "filename" = display file (same as DISPLAY)
 if (lex->current.type == TOK_STRING) {
 flen = lex->current.str_length;
 if (flen > MAX_LINE_LENGTH) flen = MAX_LINE_LENGTH;
 memcpy(fname, lex->current.str_start, (size_t)flen);
 fname[flen] = '\0';
 lexer_next(lex);
 } else {
 BValue v = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&v) && v.v.sval.data) {
 flen = v.v.sval.length;
 if (flen > MAX_LINE_LENGTH) flen = MAX_LINE_LENGTH;
 memcpy(fname, v.v.sval.data, (size_t)flen);
 fname[flen] = '\0';
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }

 if (security_check(SECOP_FILE_READ, line_num))
 return;

 fp = fopen(fname, "r");
 if (fp == NULL) {
 error_raise(ERR_SORRY, line_num);
 return;
 }

 while (fgets(line_buf, sizeof(line_buf), fp) != NULL) {
 printf("%s", line_buf);
 }
 fclose(fp);
}

/*
 * ---
 * BASIC++ Interpreter - parser_io.c
 * ---
 *
 * I/O statement handlers: PRINT, INPUT, DATA, READ, RESTORE.
 *
 * Handles all console I/O operations including formatted output,
 * user input parsing, and the DATA/READ/RESTORE mechanism.
 *
 * ---
 */

#include "parser_internal.h"

/*
 * print_margin_check - Auto-wrap when cursor exceeds margin.
 *
 * Both ECMA-55 and GW-BASIC/QBasic wrap output to the next
 * line when the cursor position exceeds the defined margin
 * (screen_width). This is a no-op for file channel output.
 */
void pi_print_margin_check(RuntimeState *rt)
{
 if (rt->print_col > rt->screen_width) {
 printf("\n");
 rt->print_col = 1;
 rt->cursor_row++;
 }
}

void pi_parse_print(Lexer *lex, RuntimeState *rt, int line_num)
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
#if defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
 rfp = NULL; /* pipes not supported on DOS */
#elif defined(_WIN32)
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
#if defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
 /* no pipe on DOS */
#elif defined(_WIN32)
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
	int flen;
 lexer_next(lex); /* consume USING */

 /* Parse format: string literal or IMAGE line ref */
 if (lex->current.type == TOK_STRING) {
 /* PRINT USING "###.##"; expr */
 fmt = lex->current.str_start;
 flen = lex->current.str_length;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NUMBER) {
 /*
  * PRINT USING 100, expr
  * Line 100 must be an IMAGE line. Extract
  * the format text after the IMAGE keyword.
  */
 int img_line = (int)lex->current.value.num_value;
 int img_idx;
 const char *img_text;
 const char *p;
 lexer_next(lex); /* consume line number */

 img_idx = program_find(
 rt->program, img_line);
 if (img_idx < 0) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 img_text = rt->program->lines[img_idx].text;

 /* Skip past line number and whitespace */
 p = img_text;
 while (*p >= '0' && *p <= '9') p++;
 while (*p == ' ' || *p == '\t') p++;

 /* Verify it's an IMAGE statement */
 if (!((*p == 'I' || *p == 'i') &&
 (*(p+1) == 'M' || *(p+1) == 'm') &&
 (*(p+2) == 'A' || *(p+2) == 'a') &&
 (*(p+3) == 'G' || *(p+3) == 'g') &&
 (*(p+4) == 'E' || *(p+4) == 'e'))) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 p += 5; /* skip IMAGE */
 while (*p == ' ' || *p == '\t') p++;

 fmt = p;
 flen = (int)strlen(p);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

	/* Expect semicolon or comma after format */
	if (lex->current.type == TOK_SEMICOLON)
	lexer_next(lex);
	else if (lex->current.type == TOK_COMMA)
	lexer_next(lex);

	/*
	 * Delegate to the format engine.
	 * The engine reads values from the lexer as needed,
	 * outputs formatted text to the target stream.
	 */
	{
	FILE *target = (file_chan > 0)
	? fileio_get_fp(file_chan) : stdout;
	if (target == NULL) target = stdout;
	format_using_process(target, fmt, flen,
	lex, rt, line_num);
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

 /* KEYWORD PRINT PREFIX - prepend to output */
 {
 const char *pfx =
 keyword_prop_get(KW_PRINT,
 "PREFIX");
 if (pfx && pfx[0] != '\0' &&
 file_chan == 0) {
 printf("%s", pfx);
 }
 }

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
 if (rt->tab_mode == 1) {
 /* REAL: emit HT character */
 putchar('\t');
 rt->print_col = col;
 } else {
 /* SPACES: pad with spaces */
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
 } else if (bval_is_complex(&val)) {
 bval_to_string_buf(&val, buf, 64);
 fileio_print(file_chan, buf, line_num);
 } else if (bval_is_float(&val)) {
 sprintf(buf, "%.14G", val.v.fval);
 fileio_print(file_chan, buf, line_num);
 } else {
 sprintf(buf, "%ld", val.v.ival);
 fileio_print(file_chan, buf, line_num);
 }
 } else {
 /* Print to stdout (original behavior) */
 if (bval_is_string(&val)) {
 int si;
 /* KEYWORD PRINT UPPERCASE/LOWERCASE */
 int force_up =
 keyword_prop_is_on(KW_PRINT,
 "UPPERCASE");
 int force_lo =
 keyword_prop_is_on(KW_PRINT,
 "LOWERCASE");
 for (si = 0; si < val.v.sval.length; si++) {
 char ch = val.v.sval.data[si];
 if (force_up && ch >= 'a' && ch <= 'z')
 ch = (char)(ch - 32);
 else if (force_lo && ch >= 'A' && ch <= 'Z')
 ch = (char)(ch + 32);
 putchar(ch);
 rt->print_col++;
 pi_print_margin_check(rt);
 }
 } else if (bval_is_complex(&val)) {
 /* Complex: print as (real+imag·i) */
 int nc;
 char cbuf[64];
 bval_to_string_buf(&val, cbuf, 64);
 nc = printf("%s", cbuf);
 rt->print_col += nc;
 pi_print_margin_check(rt);
 } else if (bval_is_float(&val)) {
 int nc;
 /* ECMA-55: leading space for positive */
 if (val.v.fval >= 0.0) {
 putchar(' ');
 rt->print_col++;
 }
 nc = printf("%.14G", val.v.fval);
 rt->print_col += nc;
 /* ECMA-55: trailing space */
 putchar(' ');
 rt->print_col++;
 pi_print_margin_check(rt);
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
 pi_print_margin_check(rt);
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
 int prop_zone =
 keyword_prop_get_int(KW_PRINT,
 "ZONE", -1);
 if (prop_zone > 0)
 zone = prop_zone;
 if (rt->zone_override > 0)
 zone = rt->zone_override;
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
void pi_parse_input(Lexer *lex, RuntimeState *rt, int line_num)
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

 /*
 * INPUT USING "format"; var-list
 * Validated input: reads input, checks against format,
 * re-prompts if invalid (Microsoft-style protection).
 */
 if (file_chan == 0 && lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_USING) {
 const char *ufmt;
 int uflen;
 char ubuf[INPUT_BUFFER_SIZE];
 int ulen;
 lexer_next(lex); /* consume USING */

 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 ufmt = lex->current.str_start;
 uflen = lex->current.str_length;
 lexer_next(lex);

 /* Consume semicolon or comma after format */
 if (lex->current.type == TOK_SEMICOLON ||
 lex->current.type == TOK_COMMA)
 lexer_next(lex);

 /* Read and validate */
 ulen = format_input_using(ubuf, INPUT_BUFFER_SIZE,
 ufmt, uflen, "? ");
 if (ulen < 0) {
 error_raise(ERR_HOW, line_num);
 return;
 }

 /* Store in the target variable */
 if (lex->current.type == TOK_STRING_VAR) {
 char svar = lex->current.value.var_name;
 char *ptr;
 lexer_next(lex);
 ptr = strpool_store(&rt->strpool, ubuf, ulen);
 if (ptr == NULL) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 runtime_set_string_var(rt, svar,
 bval_string(ptr, ulen));
 } else if (lex->current.type == TOK_VARIABLE) {
 char vn = lex->current.value.var_name;
 char *endp;
 long val = strtol(ubuf, &endp, 10);
 lexer_next(lex);
 runtime_set_var(rt, vn, val);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;
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
 const char *kp =
 keyword_prop_get(KW_INPUT,
 "PROMPT");
 printf("%s", kp ? kp : "? ");
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
 const char *kp =
 keyword_prop_get(KW_INPUT,
 "PROMPT");
 printf("%s", kp ? kp : "? ");
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


void pi_parse_data(Lexer *lex, RuntimeState *rt, int line_num)
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
void pi_parse_read(Lexer *lex, RuntimeState *rt, int line_num)
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
void pi_parse_restore(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)lex;
 (void)line_num;
 runtime_restore_data(rt);
}

/*
 * parse_merge_cmd - Parse MERGE "filename"
 */

/*
 * pi_parse_line_input - Handle LINE INPUT statement.
 *
 * Syntax:
 *   LINE INPUT [;] ["prompt";] var$
 *   LINE INPUT #channel, var$
 *
 * Reads an entire line (including commas and
 * quotes) into a string variable. Unlike INPUT,
 * no splitting on commas.
 */
void pi_parse_line_input(Lexer *lex, RuntimeState *rt,
 int line_num)
{
 int channel = -1;
 int suppress_cr = 0;
 const char *prompt = NULL;
 int prompt_len = 0;
 char buf[MAX_LINE_LENGTH + 1];
 int blen;
 char *ptr;

 /* Check for semicolon (suppress CR after input) */
 if (lex->current.type == TOK_SEMICOLON) {
 suppress_cr = 1;
 lexer_next(lex);
 }

 /* Check for #channel */
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex);
 channel = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 }

 /* Check for optional "prompt"; */
 if (lex->current.type == TOK_STRING) {
 prompt = lex->current.str_start;
 prompt_len = lex->current.str_length;
 lexer_next(lex);
 /* Expect semicolon after prompt */
 if (lex->current.type == TOK_SEMICOLON)
 lexer_next(lex);
 }

 /* Target variable: string var or named var */
 if (channel >= 0) {
 /* File LINE INPUT */
 if (fileio_input_line(channel, buf,
  MAX_LINE_LENGTH, line_num) != 0) {
  if (!error_occurred())
   error_raise(ERR_HOW, line_num);
  return;
 }
 blen = (int)strlen(buf);
 } else {
 /* Console LINE INPUT */
 if (prompt && prompt_len > 0) {
 fwrite(prompt, 1, (size_t)prompt_len, stdout);
 fflush(stdout);
 } else if (!suppress_cr) {
 printf("? ");
 fflush(stdout);
 }
 if (!fgets(buf, MAX_LINE_LENGTH, stdin)) {
 buf[0] = '\0';
 blen = 0;
 } else {
 blen = (int)strlen(buf);
 /* Strip trailing newline */
 while (blen > 0 &&
 (buf[blen-1] == '\n' ||
 buf[blen-1] == '\r'))
 blen--;
 }
 }

 buf[blen] = '\0';
 ptr = strpool_store(&rt->strpool, buf, blen);

 /* Assign to target variable */
 if (lex->current.type == TOK_STRING_VAR) {
 char vn = lex->current.value.var_name;
 lexer_next(lex);
 runtime_set_string_var(rt, vn,
 bval_string(ptr, blen));
 } else if (lex->current.type == TOK_NAMED_VAR) {
 const char *name = lex->current.str_start;
 int nlen = lex->current.str_length;
 lexer_next(lex);
 runtime_set_named_var_bval(rt, name, nlen,
 bval_string(ptr, blen));
 } else {
 error_raise(ERR_WHAT, line_num);
 }
}

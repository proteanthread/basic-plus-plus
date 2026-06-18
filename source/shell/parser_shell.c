/*
 * ---
 * BASIC++ Interpreter - parser_shell.c
 * ---
 *
 * Shell & external execution commands.
 *
 * SHELL, EXEC, SYS, SYSTEM, ENVIRON.
 *
 * ---
 */

#include "parser_internal.h"

/*
 * pi_parse_shell - Handle SHELL command.
 */
void pi_parse_shell(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_exec - Handle EXEC command.
 */
void pi_parse_exec(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_sys - Handle SYS command.
 */
void pi_parse_sys(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_system - Handle SYSTEM command.
 */
void pi_parse_system(Lexer *lex, RuntimeState *rt, int line_num)
{
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
 if (pi_str_case_equal(qname, "PLATFORM")) {
 printf("%s (%s)\n",
 platform_name(),
 platform_short_name());
 } else if (pi_str_case_equal(qname,
 "VERSION")) {
 printf("%s %s\n",
 BASICPP_NAME,
 BASICPP_VERSION);
 } else if (pi_str_case_equal(qname,
 "MEMORY")) {
 platform_print_memory(rt);
 } else if (pi_str_case_equal(qname,
 "COMPILER")) {
 const PlatformInfo *pi;
 pi = platform_get_info();
 printf("%s %s\n",
 pi->compiler,
 pi->compiler_ver);
 } else if (pi_str_case_equal(qname,
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
}

/*
 * pi_parse_environ - Handle ENVIRON command.
 */
void pi_parse_environ(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * ENVIRON "NAME=VALUE"
  * Set an environment variable. The string
  * must contain '=' as NAME=VALUE.
  * Uses _putenv (MSVC) or putenv (POSIX).
  */
 {
 BValue ev = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&ev) &&
 ev.v.sval.length > 0) {
 char envbuf[520];
 int el = ev.v.sval.length;
 if (el > 510) el = 510;
 memcpy(envbuf,
 ev.v.sval.data,
 (size_t)el);
 envbuf[el] = '\0';
 /* Validate: must contain '=' */
 if (strchr(envbuf, '=') == NULL) {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
#ifdef _MSC_VER
 _putenv(envbuf);
#else
 putenv(envbuf);
#endif
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 }
 return;
}


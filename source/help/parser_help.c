/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_help.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Self-describing HELP interactive system, OPTION overrides, and security scope permissions.
 *
 * 2. WHAT TO EXPECT:
 *    Help prints keyword usage. Overrides rewrite token attributes at parse time.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Command summaries, help topics, scope presets mappings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Help databases lookup logic, keyword gating routines.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If help command fails, ensure help databases are sorted. Check override loop guards.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_help.c
 // ---
 //
 // Help & catalog commands.
 //
 // HELP, CATALOG, VER.
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
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

 // pi_parse_help - Handle HELP command.
void pi_parse_help(Lexer *lex, RuntimeState *rt, int line_num)
{
 {
 // HELP - Show command summary
 // HELP "keyword" - Show help for keyword
 // HELP RENUM - bare keyword works
 // HELP ABS - function name works
 // HELP FPRINT - compound word works
 //
 // STRATEGY:
 // 1. If next token is TOK_STRING, use
 // the quoted value directly.
 // 2. Otherwise, scan the raw source for
 // the remaining word(s) on the line.
 // This avoids issues where the lexer
 // splits compound names like FPRINT
 // into TOK_VARIABLE('F') + KW_PRINT.
 char topic[64];
 topic[0] = '\0';

 if (lex->current.type == TOK_STRING) {
 // Quoted: HELP "PRINT"
 int len = lex->current.str_length;
 if (len > 63) len = 63;
 memcpy(topic,
 lex->current.str_start,
 (size_t)len);
 topic[len] = '\0';
 lexer_next(lex);
 } else if (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 // Grab the remaining text on the line
 // as a raw string. Take the first
 // contiguous word (letters and $ only).
 int tlen = 0;

 // Back up: lex->pos points past the
 // current token. The token itself
 // starts earlier. Use the source text
 // starting from where the current
 // token's first character is.
 //
 // For keyword tokens, the start is at
 // lex->pos minus the keyword length.
 // For variables, it's pos-1.
 // Simplest: scan backwards to find the
 // start of the current word.
 {
 const char *src = lex->source;
 int spos = lex->pos;
 int wstart;
 int wend;
 char c;

 // Back up to start of current
 // token 
 if (spos > 0) spos--;
 while (spos > 0 &&
 ((src[spos-1] >= 'A' &&
 src[spos-1] <= 'Z') ||
 (src[spos-1] >= 'a' &&
 src[spos-1] <= 'z'))) {
 spos--;
 }
 wstart = spos;

 // Find end of word (letters + $)
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

 // Uppercase the topic
 {
 int ti;
 for (ti = 0; topic[ti]; ti++) {
 if (topic[ti] >= 'a' &&
 topic[ti] <= 'z') {
 topic[ti] -= 32;
 }
 }
 }

 // Skip past all remaining tokens
 lexer_skip_to_end(lex);
 }

 help_show(topic[0] ? topic : NULL);
 return;
 }
}

#ifndef BPP_LITE_BUILD
 // pi_parse_catalog - Handle CATALOG command.
void pi_parse_catalog(Lexer *lex, RuntimeState *rt, int line_num)
{
  // CATALOG - List registered functions.
  help_catalog();
  return;

  // ===== Virtual Subsystem Introspection =====
}
#endif

 // pi_parse_ver - Handle VER command.
void pi_parse_ver(Lexer *lex, RuntimeState *rt, int line_num)
{
  (void)lex; (void)rt; (void)line_num;
  // VER - Display version information.
  printf("%s %s\n",
  BASICPP_NAME, BASICPP_VERSION);
  printf("%s\n", BASICPP_COPYRIGHT);
  printf("Build: %s\n", __DATE__);
  return;
}

void pi_parse_hostname(Lexer *lex, RuntimeState *rt, int line_num)
{
  (void)lex; (void)rt; (void)line_num;
  const char *h = getenv("COMPUTERNAME");
  if (!h) h = getenv("HOSTNAME");
  if (!h) h = "localhost";
  printf("%s\n", h);
}

void pi_parse_username(Lexer *lex, RuntimeState *rt, int line_num)
{
  (void)lex; (void)rt; (void)line_num;
  const char *u = getenv("USERNAME");
  if (!u) u = getenv("USER");
  if (!u) u = "user";
  printf("%s\n", u);
}

#ifdef BPP_LITE_BUILD
void pi_parse_pwd(Lexer *lex, RuntimeState *rt, int line_num)
{
  (void)lex; (void)rt; (void)line_num;
  char cwdbuf[512];
#ifdef _WIN32
  if (_getcwd(cwdbuf, sizeof(cwdbuf)) == NULL) cwdbuf[0] = '\0';
#else
  if (getcwd(cwdbuf, sizeof(cwdbuf)) == NULL) cwdbuf[0] = '\0';
#endif
  printf("%s\n", cwdbuf);
}
#endif

void pi_parse_cwd(Lexer *lex, RuntimeState *rt, int line_num)
{
  (void)lex; (void)rt; (void)line_num;
  char cwdbuf[512];
#ifdef _WIN32
  if (_getcwd(cwdbuf, sizeof(cwdbuf)) == NULL) cwdbuf[0] = '\0';
#else
  if (getcwd(cwdbuf, sizeof(cwdbuf)) == NULL) cwdbuf[0] = '\0';
#endif
  char *last_comp = cwdbuf;
  for (int i = 0; cwdbuf[i] != '\0'; i++) {
      if (cwdbuf[i] == '/' || cwdbuf[i] == '\\') {
          if (cwdbuf[i+1] != '\0') {
              last_comp = &cwdbuf[i+1];
          }
      }
  }
  printf("%s\n", last_comp);
}


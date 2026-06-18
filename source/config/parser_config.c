 // ---
 // BASIC++ Interpreter - parser_config.c
 // ---
 //
 // Configuration, meta & customization commands.
 //
 // OPTION, ALIAS, SCOPE, KEYWORD, OVERRIDE,
 // SECURITY, MODULE.
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

 // pi_parse_option - Handle OPTION command.
void pi_parse_option(Lexer *lex, RuntimeState *rt, int line_num)
{
 // OPTION BASE 0 | 1
 // Set default array lower bound.
 //
 // OPTION STRICT
 // Enable strict dialect mode: only
 // keywords belonging to the active
 // dialect are allowed.
 //
 // OPTION STRICT OFF
 // Disable strict mode (union mode).
 // OPTION ANGLE DEGREES | RADIANS
 // Set trig function angle mode.
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 5 &&
 lex->current.str_start != NULL) {
 const char *a = lex->current.str_start;
 if ((a[0]=='A'||a[0]=='a') &&
 (a[1]=='N'||a[1]=='n') &&
 (a[2]=='G'||a[2]=='g') &&
 (a[3]=='L'||a[3]=='l') &&
 (a[4]=='E'||a[4]=='e')) {
 lexer_next(lex); // consume ANGLE
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
  // OPTION TAB REAL | SPACES
  // Set TAB function behavior: REAL emits
  // HT characters, SPACES (default) emits
  // space characters to reach the column.
 if (lex->current.type == TOK_KEYWORD &&
  lex->current.value.keyword == KW_TAB_FUNC) {
  lexer_next(lex); // consume TAB
  if (lex->current.type == TOK_NAMED_VAR &&
   lex->current.str_length >= 4 &&
   lex->current.str_start != NULL) {
   const char *m = lex->current.str_start;
   if ((m[0]=='R'||m[0]=='r') &&
    (m[1]=='E'||m[1]=='e') &&
    (m[2]=='A'||m[2]=='a') &&
    (m[3]=='L'||m[3]=='l')) {
    rt->tab_mode = 1;
    printf("TAB mode: REAL (HT)\n");
    lexer_next(lex);
   } else if (
    (m[0]=='S'||m[0]=='s') &&
    (m[1]=='P'||m[1]=='p') &&
    (m[2]=='A'||m[2]=='a') &&
    (m[3]=='C'||m[3]=='c')) {
    rt->tab_mode = 0;
    printf("TAB mode: SPACES\n");
    lexer_next(lex);
   } else {
    error_raise(ERR_WHAT, line_num);
   }
  } else {
   // No argument: show current
   printf("TAB mode: %s\n",
    rt->tab_mode ? "REAL (HT)"
    : "SPACES");
  }
  return;
 }
  // OPTION ZONE n
  // Override PRINT zone width for comma
  // separator. 0 = reset to dialect default.
 if (lex->current.type == TOK_NAMED_VAR &&
  lex->current.str_length == 4 &&
  lex->current.str_start != NULL) {
  const char *z = lex->current.str_start;
  if ((z[0]=='Z'||z[0]=='z') &&
   (z[1]=='O'||z[1]=='o') &&
   (z[2]=='N'||z[2]=='n') &&
   (z[3]=='E'||z[3]=='e')) {
   int zw;
   lexer_next(lex); // consume ZONE
   zw = (int)parse_expression(
    lex, rt, line_num);
   if (error_occurred()) return;
   if (zw <= 0) {
    rt->zone_override = -1;
    printf("ZONE: dialect default"
     " (%d)\n",
     dialect_get_zone_width());
   } else {
    if (zw > 255) zw = 255;
    rt->zone_override = zw;
    printf("ZONE: %d\n", zw);
   }
   return;
  }
 }
  // OPTION ALIASES n
  // Show alias capacity.
 if (lex->current.type == TOK_NAMED_VAR &&
  lex->current.str_length == 7 &&
  lex->current.str_start != NULL) {
  const char *al = lex->current.str_start;
  if ((al[0]=='A'||al[0]=='a') &&
   (al[1]=='L'||al[1]=='l') &&
   (al[2]=='I'||al[2]=='i') &&
   (al[3]=='A'||al[3]=='a') &&
   (al[4]=='S'||al[4]=='s') &&
   (al[5]=='E'||al[5]=='e') &&
   (al[6]=='S'||al[6]=='s')) {
   lexer_next(lex); // consume ALIASES
   printf("Alias capacity: %d / %d\n",
    lexer_alias_count(),
    MAX_ALIASES);
   return;
  }
 }
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 6 &&
 lex->current.str_start != NULL) {
 // Check for "STRICT"
 const char *s = lex->current.str_start;
 if ((s[0]=='S'||s[0]=='s') &&
 (s[1]=='T'||s[1]=='t') &&
 (s[2]=='R'||s[2]=='r') &&
 (s[3]=='I'||s[3]=='i') &&
 (s[4]=='C'||s[4]=='c') &&
 (s[5]=='T'||s[5]=='t')) {
 lexer_next(lex); // consume STRICT
 // Check for OFF
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
  // OPTION MIXED "GWBS,QBAS,..."
  // Enable mixed dialect mode with specified dialects.
  // Only available in deferred (program) mode.
  // OPTION MIXED OFF - revert to previous mode.
  // OPTION MIXED (bare) - show status.
  if (lex->current.type == TOK_NAMED_VAR &&
  lex->current.str_length == 5 &&
  lex->current.str_start != NULL) {
  const char *mx = lex->current.str_start;
  if ((mx[0]=='M'||mx[0]=='m') &&
  (mx[1]=='I'||mx[1]=='i') &&
  (mx[2]=='X'||mx[2]=='x') &&
  (mx[3]=='E'||mx[3]=='e') &&
  (mx[4]=='D'||mx[4]=='d')) {
  lexer_next(lex); // consume MIXED
  // OPTION MIXED OFF
  if (lex->current.type == TOK_NAMED_VAR &&
  lex->current.str_length == 3 &&
  lex->current.str_start != NULL &&
  (lex->current.str_start[0]=='O'||
  lex->current.str_start[0]=='o')) {
  dialect_clear_mixed();
  printf("Mixed mode: OFF\n");
  lexer_next(lex);
  return;
  }
  // OPTION MIXED "GWBS,QBAS,..."
  if (lex->current.type == TOK_STRING &&
  lex->current.str_start != NULL &&
  lex->current.str_length > 0) {
  char spec[128];
  int slen = lex->current.str_length;
  unsigned int mask;
  if (!rt->running) {
  printf("SORRY. OPTION MIXED is"
  " only available in"
  " program mode.\n");
  error_raise(ERR_SORRY, line_num);
  return;
  }
  if (slen > 127) slen = 127;
  memcpy(spec,
  lex->current.str_start,
  (size_t)slen);
  spec[slen] = '\0';
  lexer_next(lex);
  mask = dialect_build_mask(spec);
  if (mask == 0) {
  printf("WHAT? No valid dialects"
  " in \"%s\"\n", spec);
  error_raise(ERR_WHAT, line_num);
  return;
  }
  dialect_set_mixed(mask);
  printf("Mixed mode: ON"
  " (mask 0x%04X)\n", mask);
  return;
  }
  // Bare OPTION MIXED: show status
  if (dialect_is_mixed()) {
  printf("Mixed mode: ON"
  " (mask 0x%04X)\n",
  dialect_get_mixed_mask());
  } else {
  printf("Mixed mode: OFF\n");
  }
  return;
  }
  }
  // Fallback for dialects without extended vars (e.g., PATB):
  // "STRICT" tokenizes as single-letter variable 'S' + raw text "TRICT".
  // Check raw source to handle OPTION STRICT / OPTION STRICT OFF.
 if (lex->current.type == TOK_VARIABLE &&
  lex->current.value.var_name == 'S' &&
  !dialect_get_config()->has_extended_vars) {
  int p = lex->pos;
  if (p + 4 < lex->length &&
   (lex->source[p]=='T'||lex->source[p]=='t') &&
   (lex->source[p+1]=='R'||lex->source[p+1]=='r') &&
   (lex->source[p+2]=='I'||lex->source[p+2]=='i') &&
   (lex->source[p+3]=='C'||lex->source[p+3]=='c') &&
   (lex->source[p+4]=='T'||lex->source[p+4]=='t') &&
   (p+5 >= lex->length ||
    lex->source[p+5]==' ' || lex->source[p+5]=='\t' ||
    lex->source[p+5]=='\0' || lex->source[p+5]=='\r' ||
    lex->source[p+5]=='\n')) {
   // Skip past "TRICT" and re-prime lexer
   lex->pos = p + 5;
   lexer_next(lex);
   // Check for OFF keyword (KW_OFF is not defined, check ON)
   if (lex->current.type == TOK_KEYWORD &&
    lex->current.value.keyword == KW_ON) {
    // OPTION STRICT ON
    dialect_set_strict(1);
    printf("Strict mode: ON"
     " (%s only)\n",
     dialect_get_short_name());
    lexer_next(lex);
   } else if (lex->current.type == TOK_VARIABLE &&
    lex->current.value.var_name == 'O' &&
    lex->pos < lex->length &&
    (lex->source[lex->pos]=='F'||
     lex->source[lex->pos]=='f') &&
    lex->pos+1 < lex->length &&
    (lex->source[lex->pos+1]=='F'||
     lex->source[lex->pos+1]=='f')) {
    // OPTION STRICT OFF (raw parse)
    lex->pos += 2;
    lexer_next(lex);
    dialect_set_strict(0);
    printf("Strict mode: OFF"
     " (union)\n");
   } else if (lex->current.type == TOK_NAMED_VAR &&
    lex->current.str_length == 3 &&
    lex->current.str_start != NULL &&
    (lex->current.str_start[0]=='O'||
     lex->current.str_start[0]=='o')) {
    // OFF as named var (extended vars dialect)
    dialect_set_strict(0);
    printf("Strict mode: OFF"
     " (union)\n");
    lexer_next(lex);
   } else {
    // Bare OPTION STRICT - enable
    dialect_set_strict(1);
    printf("Strict mode: ON"
     " (%s only)\n",
     dialect_get_short_name());
   }
   return;
  }
 }
  // OPTION KEYWORD UPPER | LOWER | TITLE | MIXED
  // Set keyword display/storage case mode.
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_KEYWORD) {
 lexer_next(lex); // consume KEYWORD
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length >= 3 &&
 lex->current.str_start != NULL) {
 const char *m = lex->current.str_start;
 if ((m[0]=='U'||m[0]=='u') &&
 (m[1]=='P'||m[1]=='p') &&
 (m[2]=='P'||m[2]=='p')) {
 lexer_set_keyword_case(KWCASE_UPPER);
 printf("Keyword case: UPPER\n");
 lexer_next(lex);
 } else if (
 (m[0]=='L'||m[0]=='l') &&
 (m[1]=='O'||m[1]=='o') &&
 (m[2]=='W'||m[2]=='w')) {
 lexer_set_keyword_case(KWCASE_LOWER);
 printf("Keyword case: LOWER\n");
 lexer_next(lex);
 } else if (
 (m[0]=='T'||m[0]=='t') &&
 (m[1]=='I'||m[1]=='i') &&
 (m[2]=='T'||m[2]=='t')) {
 lexer_set_keyword_case(KWCASE_TITLE);
 printf("Keyword case: TITLE\n");
 lexer_next(lex);
 } else if (
 (m[0]=='M'||m[0]=='m') &&
 (m[1]=='I'||m[1]=='i') &&
 (m[2]=='X'||m[2]=='x')) {
 lexer_set_keyword_case(KWCASE_MIXED);
 printf("Keyword case: MIXED"
 " (as-is)\n");
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 } else {
 // No argument: show current mode
 const char *names[] = {
 "MIXED", "UPPER", "LOWER", "TITLE"
 };
 printf("Keyword case: %s\n",
 names[lexer_get_keyword_case()]);
 }
 return;
 }
  // OPTION RPN [ON | OFF]
  // Enable or disable Forth-style RPN calculator mode.
  // When active, the REPL evaluates input as RPN
  // expressions instead of BASIC statements.
 if (lex->current.type == TOK_NAMED_VAR &&
  lex->current.str_length == 3 &&
  lex->current.str_start != NULL) {
  const char *r = lex->current.str_start;
  if ((r[0]=='R'||r[0]=='r') &&
   (r[1]=='P'||r[1]=='p') &&
   (r[2]=='N'||r[2]=='n')) {
   lexer_next(lex); // consume RPN
   // Check for OFF
   if (lex->current.type == TOK_NAMED_VAR &&
    lex->current.str_length == 3 &&
    lex->current.str_start != NULL &&
    (lex->current.str_start[0]=='O'||
     lex->current.str_start[0]=='o') &&
    (lex->current.str_start[1]=='F'||
     lex->current.str_start[1]=='f') &&
    (lex->current.str_start[2]=='F'||
     lex->current.str_start[2]=='f')) {
    rpn_set_active(&rt->rpn, 0);
    lexer_next(lex);
   } else {
    rpn_set_active(&rt->rpn, 1);
   }
   return;
  }
 }
 // OPTION ARITHMETIC NATIVE | DECIMAL
 // ECMA-116 arithmetic mode. NATIVE uses
 // hardware floating point (our default).
 // DECIMAL would require base-10 math.
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
 lexer_next(lex); // ARITHMETIC
 if (lex->current.type == TOK_NAMED_VAR
 && lex->current.str_length >= 3
 && lex->current.str_start
 != NULL) {
 const char *m =
 lex->current.str_start;
 if ((m[0]=='N'||m[0]=='n') &&
 (m[1]=='A'||m[1]=='a') &&
 (m[2]=='T'||m[2]=='t')) {
 // NATIVE - no-op (default)
 lexer_next(lex);
 } else if (
 (m[0]=='D'||m[0]=='d') &&
 (m[1]=='E'||m[1]=='e') &&
 (m[2]=='C'||m[2]=='c')) {
 // DECIMAL - not supported
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
 // Consume BASE (may be keyword, named var,
 // or multiple single-letter variables when
 // extended vars are off). Skip everything
 // until we reach the numeric argument.
 while (lex->current.type != TOK_NUMBER &&
 lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 lexer_next(lex);
 }
 // Consume the 0 or 1 and set option_base
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
}

 // pi_parse_alias - Handle ALIAS command.
void pi_parse_alias(Lexer *lex, RuntimeState *rt, int line_num)
{
  // ALIAS keyword = "newname"
  // ALIAS LIST / CLEAR / COUNT
  // ALIAS REMOVE "name"
  // ALIAS SAVE "file" / LOAD "file"
  // ALIAS LANG "code" / LANG LIST / LANG CLEAR
  // ALIAS OPER LIST
  // ALIAS "op" = "name" (operator alias, quoted)
  // ALIAS op = "name" (operator alias, symbolic)
 {
 KeywordId target_kw;
 AliasScope scope;

 // Choose scope based on running state
 scope = rt->running ? ASCOPE_PROGRAM
 : ASCOPE_GLOBAL;

 // ALIAS LIST
 if (lexer_match_keyword(lex,
 KW_LIST)) {
 lexer_next(lex);
 lexer_list_aliases();
 return;
 }

 // ALIAS CLEAR / REMOVE / COUNT / LANG / OPER
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *nv =
 lex->current.str_start;
 int nlen =
 lex->current.str_length;

 // CLEAR
 if (nlen == 5 &&
 (nv[0]=='C'||nv[0]=='c') &&
 (nv[1]=='L'||nv[1]=='l') &&
 (nv[2]=='E'||nv[2]=='e') &&
 (nv[3]=='A'||nv[3]=='a') &&
 (nv[4]=='R'||nv[4]=='r')) {
 lexer_next(lex);
 lexer_clear_aliases();
 lexer_clear_op_aliases();
 printf("All aliases cleared.\n");
 return;
 }
 // REMOVE
 if (nlen == 6 &&
 (nv[0]=='R'||nv[0]=='r') &&
 (nv[1]=='E'||nv[1]=='e') &&
 (nv[2]=='M'||nv[2]=='m') &&
 (nv[3]=='O'||nv[3]=='o') &&
 (nv[4]=='V'||nv[4]=='v') &&
 (nv[5]=='E'||nv[5]=='e')) {
 lexer_next(lex);
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lexer_remove_alias(
 lex->current.str_start,
 lex->current.str_length) == 0) {
 printf("Alias removed.\n");
 } else {
 printf("Alias not found.\n");
 }
 lexer_next(lex);
 return;
 }
 // COUNT
 if (nlen == 5 &&
 (nv[0]=='C'||nv[0]=='c') &&
 (nv[1]=='O'||nv[1]=='o') &&
 (nv[2]=='U'||nv[2]=='u') &&
 (nv[3]=='N'||nv[3]=='n') &&
 (nv[4]=='T'||nv[4]=='t')) {
 lexer_next(lex);
 printf("Aliases: %d / %d slots"
 " used.\n",
 lexer_alias_count(),
 MAX_ALIASES);
 if (lexer_op_alias_count() > 0) {
 printf("Operator aliases: %d"
 " / %d\n",
 lexer_op_alias_count(),
 MAX_OP_ALIASES);
 }
 return;
 }
 // LANG
 if (nlen == 4 &&
 (nv[0]=='L'||nv[0]=='l') &&
 (nv[1]=='A'||nv[1]=='a') &&
 (nv[2]=='N'||nv[2]=='n') &&
 (nv[3]=='G'||nv[3]=='g')) {
 lexer_next(lex);
 // ALIAS LANG LIST
 if (lexer_match_keyword(lex,
 KW_LIST)) {
 lexer_next(lex);
 alias_lang_list();
 return;
 }
 // ALIAS LANG CLEAR (named var)
 if (lex->current.type ==
 TOK_NAMED_VAR &&
 lex->current.str_length == 5) {
 const char *cv =
 lex->current.str_start;
 if ((cv[0]=='C'||cv[0]=='c') &&
 (cv[1]=='L'||cv[1]=='l') &&
 (cv[2]=='E'||cv[2]=='e') &&
 (cv[3]=='A'||cv[3]=='a') &&
 (cv[4]=='R'||cv[4]=='r')) {
 lexer_next(lex);
 alias_lang_clear();
 return;
 }
 }
 if (lexer_match_keyword(lex,
 KW_CLEAR)) {
 lexer_next(lex);
 alias_lang_clear();
 return;
 }
 // ALIAS LANG "code"
 if (lex->current.type == TOK_STRING) {
 char code[8];
 int clen =
 lex->current.str_length;
 if (clen > 7) clen = 7;
 memcpy(code,
 lex->current.str_start,
 (size_t)clen);
 code[clen] = '\0';
 lexer_next(lex);
 if (alias_lang_load(code) < 0) {
 printf("Unknown language:"
 " \"%s\"\n"
 "Use ALIAS LANG LIST\n",
 code);
 }
 return;
 }
 alias_lang_list();
 return;
 }
 // OPER
 if (nlen == 4 &&
 (nv[0]=='O'||nv[0]=='o') &&
 (nv[1]=='P'||nv[1]=='p') &&
 (nv[2]=='E'||nv[2]=='e') &&
 (nv[3]=='R'||nv[3]=='r')) {
 lexer_next(lex);
 if (lexer_match_keyword(lex,
 KW_LIST)) {
 lexer_next(lex);
 lexer_list_op_aliases();
 return;
 }
 if (lexer_match_keyword(lex,
 KW_CLEAR)) {
 lexer_next(lex);
 lexer_clear_op_aliases();
 printf("Operator aliases"
 " cleared.\n");
 return;
 }
 lexer_list_op_aliases();
 return;
 }
 }

 // ALIAS CLEAR (keyword)
 if (lexer_match_keyword(lex,
 KW_CLEAR)) {
 lexer_next(lex);
 lexer_clear_aliases();
 lexer_clear_op_aliases();
 printf("All aliases cleared.\n");
 return;
 }

 // ALIAS SAVE "file"
 if (lexer_match_keyword(lex,
 KW_SAVE)) {
 char fname[256];
 int flen;
 lexer_next(lex);
 if (security_check(SECOP_FILE_WRITE,
 line_num)) return;
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 flen = lex->current.str_length;
 if (flen > 255) flen = 255;
 memcpy(fname,
 lex->current.str_start,
 (size_t)flen);
 fname[flen] = '\0';
 lexer_next(lex);
 {
 int saved = lexer_alias_save(fname);
 if (saved < 0)
 printf("Cannot write %s\n",
 fname);
 else
 printf("%d aliases saved to"
 " %s\n", saved, fname);
 }
 return;
 }

 // ALIAS LOAD "file"
 if (lexer_match_keyword(lex,
 KW_LOAD)) {
 char fname[256];
 int flen;
 lexer_next(lex);
 if (security_check(SECOP_FILE_READ,
 line_num)) return;
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 flen = lex->current.str_length;
 if (flen > 255) flen = 255;
 memcpy(fname,
 lex->current.str_start,
 (size_t)flen);
 fname[flen] = '\0';
 lexer_next(lex);
 {
 int loaded =
 lexer_alias_load(fname);
 if (loaded < 0)
 printf("Cannot read %s\n",
 fname);
 else
 printf("%d aliases loaded"
 " from %s\n",
 loaded, fname);
 }
 return;
 }

 // ALIAS op = "name" (symbolic operator alias)
  // Accepts bare operator tokens: ALIAS >= = "a"
 if (lex->current.type == TOK_GT_EQ ||
  lex->current.type == TOK_LT_EQ ||
  lex->current.type == TOK_NOT_EQ ||
  lex->current.type == TOK_EQUALS ||
  lex->current.type == TOK_PLUS ||
  lex->current.type == TOK_MINUS ||
  lex->current.type == TOK_STAR ||
  lex->current.type == TOK_SLASH) {
  int tok_type = lex->current.type;
  lexer_next(lex);
  if (lex->current.type != TOK_EQUALS) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  lexer_next(lex);
  if (lex->current.type != TOK_STRING) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  if (lexer_add_op_alias(
   lex->current.str_start,
   lex->current.str_length,
   tok_type) != 0) {
   printf("Operator alias table"
    " full.\n");
  } else {
   printf("Operator alias set.\n");
  }
  lexer_next(lex);
  return;
 }

 // ALIAS "op" = "name" (quoted operator alias)
 if (lex->current.type == TOK_STRING) {
 const char *opstr =
 lex->current.str_start;
 int oplen =
 lex->current.str_length;
 int tok_type = -1;

 // Identify operator
 if (oplen == 2) {
 if (opstr[0]=='>' && opstr[1]=='=')
 tok_type = TOK_GT_EQ;
 else if (opstr[0]=='<' &&
 opstr[1]=='=')
 tok_type = TOK_LT_EQ;
 else if (opstr[0]=='<' &&
 opstr[1]=='>')
 tok_type = TOK_NOT_EQ;
 } else if (oplen == 1) {
 if (opstr[0] == '=')
 tok_type = TOK_EQUALS;
 else if (opstr[0] == '+')
 tok_type = TOK_PLUS;
 else if (opstr[0] == '-')
 tok_type = TOK_MINUS;
 else if (opstr[0] == '*')
 tok_type = TOK_STAR;
 else if (opstr[0] == '/')
 tok_type = TOK_SLASH;
 }
 if (tok_type >= 0) {
 lexer_next(lex);
 if (lex->current.type != TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lexer_add_op_alias(
 lex->current.str_start,
 lex->current.str_length,
 tok_type) != 0) {
 printf("Operator alias table"
 " full.\n");
 } else {
 printf("Operator alias set.\n");
 }
 lexer_next(lex);
 return;
 }
 // Not a recognized operator string
 error_raise(ERR_WHAT, line_num);
 return;
 }

 // Expect a keyword to alias
 if (lex->current.type !=
 TOK_KEYWORD) {
 printf(
 "Usage:\n"
 " ALIAS keyword = \"newname\"\n"
 " ALIAS LIST | CLEAR | COUNT\n"
 " ALIAS REMOVE \"name\"\n"
 " ALIAS SAVE \"file\"\n"
 " ALIAS LOAD \"file\"\n"
 " ALIAS LANG \"code\" | LIST |"
 " CLEAR\n"
 " ALIAS OPER LIST | CLEAR\n"
 " ALIAS \">= \" = \"name\"\n"
 " ALIAS >= = \"name\"\n");
 return;
 }
 target_kw =
 lex->current.value.keyword;
 lexer_next(lex);

 // Expect =
 if (lex->current.type !=
 TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // Expect string with new name
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

 lexer_next(lex); {
  int arc = lexer_add_alias_scoped(
  aname, alen, target_kw,
  scope, NULL);
  if (arc != 0) {
  if (arc == -2)
  printf("Name conflicts with"
  " a keyword.\n");
  else if (arc == -3)
  printf("Cannot alias"
  " protected keyword.\n");
  else
  printf("Alias table full.\n");
  return;
  }
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
 printf("\"");
 if (scope != ASCOPE_GLOBAL)
 printf(" [PROGRAM]");
 printf("\n");
 }
 }
 return;
}

 // pi_parse_scope - Handle SCOPE command.
void pi_parse_scope(Lexer *lex, RuntimeState *rt, int line_num)
{
  // SCOPE DISABLE keyword
  // SCOPE ENABLE keyword
  // SCOPE BEFORE keyword GOSUB line
  // SCOPE AFTER keyword GOSUB line
  // SCOPE OVERRIDE keyword GOSUB line
  // SCOPE RESTORE keyword
  // SCOPE LIST / RESET
  // SCOPE \"preset\"
  //
  // Security: blocked at LEVEL 2+ except
  // for LIST (read-only introspection).
 {
 // SCOPE LIST
 if (lexer_match_keyword(lex, KW_LIST)) {
 lexer_next(lex);
 scope_list();
 return;
 }
 // Security check for all SCOPE
  // sub-commands 
 if (security_check(
  SECOP_SYSTEM, line_num))
  return;

 // SCOPE RESET (keyword form)
 if (lexer_match_keyword(lex,
  KW_RESET)) {
  lexer_next(lex);
  scope_reset();
  return;
 }

 // SCOPE RESTORE keyword (keyword form)
 if (lexer_match_keyword(lex,
 KW_RESTORE)) {
 lexer_next(lex);
 if (lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 scope_restore(
 lex->current.value.keyword);
 printf("SCOPE: %s restored.\n",
 lexer_keyword_name(
 lex->current.value.keyword));
 lexer_next(lex);
 return;
 }

 // SCOPE \"preset\"
 if (lex->current.type == TOK_STRING) {
 char pname[32];
 int plen = lex->current.str_length;
 if (plen > 31) plen = 31;
 memcpy(pname,
 lex->current.str_start,
 (size_t)plen);
 pname[plen] = '\0';
 lexer_next(lex);
 if (scope_load_preset(pname) < 0) {
 printf("Unknown preset: \"%s\"\n",
 pname);
 scope_list_presets();
 }
 return;
 }

 // Sub-commands via named vars (GWBS)
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *nv =
 lex->current.str_start;
 int nlen = lex->current.str_length;

 // RESET
 if (nlen == 5 &&
 (nv[0]=='R'||nv[0]=='r') &&
 (nv[1]=='E'||nv[1]=='e') &&
 (nv[2]=='S'||nv[2]=='s') &&
 (nv[3]=='E'||nv[3]=='e') &&
 (nv[4]=='T'||nv[4]=='t')) {
 lexer_next(lex);
 scope_reset();
 return;
 }

 // DISABLE
 if (nlen == 7 &&
 (nv[0]=='D'||nv[0]=='d') &&
 (nv[1]=='I'||nv[1]=='i') &&
 (nv[2]=='S'||nv[2]=='s') &&
 (nv[3]=='A'||nv[3]=='a') &&
 (nv[4]=='B'||nv[4]=='b') &&
 (nv[5]=='L'||nv[5]=='l') &&
 (nv[6]=='E'||nv[6]=='e')) {
 lexer_next(lex);
 if (lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 scope_disable(
 lex->current.value.keyword);
 printf("SCOPE: %s disabled.\n",
 lexer_keyword_name(
 lex->current.value.keyword));
 lexer_next(lex);
 return;
 }

 // ENABLE
 if (nlen == 6 &&
 (nv[0]=='E'||nv[0]=='e') &&
 (nv[1]=='N'||nv[1]=='n') &&
 (nv[2]=='A'||nv[2]=='a') &&
 (nv[3]=='B'||nv[3]=='b') &&
 (nv[4]=='L'||nv[4]=='l') &&
 (nv[5]=='E'||nv[5]=='e')) {
 lexer_next(lex);
 if (lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 scope_enable(
 lex->current.value.keyword);
 printf("SCOPE: %s enabled.\n",
 lexer_keyword_name(
 lex->current.value.keyword));
 lexer_next(lex);
 return;
 }

 // BEFORE
 if (nlen == 6 &&
 (nv[0]=='B'||nv[0]=='b') &&
 (nv[1]=='E'||nv[1]=='e') &&
 (nv[2]=='F'||nv[2]=='f') &&
 (nv[3]=='O'||nv[3]=='o') &&
 (nv[4]=='R'||nv[4]=='r') &&
 (nv[5]=='E'||nv[5]=='e')) {
 KeywordId tkw;
 lexer_next(lex);
 if (lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 tkw = lex->current.value.keyword;
 lexer_next(lex);
 if (!lexer_match_keyword(lex,
 KW_GOSUB)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 {
 long ln = parse_expression(lex,
 rt, line_num);
 if (error_occurred()) return;
 scope_set_before(tkw, (int)ln);
 printf("SCOPE: BEFORE %s"
 " GOSUB %ld\n",
 lexer_keyword_name(tkw), ln);
 }
 return;
 }

 // AFTER
 if (nlen == 5 &&
 (nv[0]=='A'||nv[0]=='a') &&
 (nv[1]=='F'||nv[1]=='f') &&
 (nv[2]=='T'||nv[2]=='t') &&
 (nv[3]=='E'||nv[3]=='e') &&
 (nv[4]=='R'||nv[4]=='r')) {
 KeywordId tkw;
 lexer_next(lex);
 if (lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 tkw = lex->current.value.keyword;
 lexer_next(lex);
 if (!lexer_match_keyword(lex,
 KW_GOSUB)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 {
 long ln = parse_expression(lex,
 rt, line_num);
 if (error_occurred()) return;
 scope_set_after(tkw, (int)ln);
 printf("SCOPE: AFTER %s"
 " GOSUB %ld\n",
 lexer_keyword_name(tkw), ln);
 }
 return;
 }

 // OVERRIDE
 if (nlen == 8 &&
 (nv[0]=='O'||nv[0]=='o') &&
 (nv[1]=='V'||nv[1]=='v') &&
 (nv[2]=='E'||nv[2]=='e') &&
 (nv[3]=='R'||nv[3]=='r') &&
 (nv[4]=='R'||nv[4]=='r') &&
 (nv[5]=='I'||nv[5]=='i') &&
 (nv[6]=='D'||nv[6]=='d') &&
 (nv[7]=='E'||nv[7]=='e')) {
 KeywordId tkw;
 lexer_next(lex);
 if (lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 tkw = lex->current.value.keyword;
 lexer_next(lex);
 if (!lexer_match_keyword(lex,
 KW_GOSUB)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 {
 long ln = parse_expression(lex,
 rt, line_num);
 if (error_occurred()) return;
 scope_set_override(tkw, (int)ln);
 printf("SCOPE: OVERRIDE %s"
 " GOSUB %ld\n",
 lexer_keyword_name(tkw), ln);
 }
 return;
 }

 // RESTORE
 if (nlen == 7 &&
 (nv[0]=='R'||nv[0]=='r') &&
 (nv[1]=='E'||nv[1]=='e') &&
 (nv[2]=='S'||nv[2]=='s') &&
 (nv[3]=='T'||nv[3]=='t') &&
 (nv[4]=='O'||nv[4]=='o') &&
 (nv[5]=='R'||nv[5]=='r') &&
 (nv[6]=='E'||nv[6]=='e')) {
 lexer_next(lex);
 if (lex->current.type != TOK_KEYWORD) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 scope_restore(
 lex->current.value.keyword);
 printf("SCOPE: %s restored.\n",
 lexer_keyword_name(
 lex->current.value.keyword));
 lexer_next(lex);
 return;
 }
 }

 // Fallback usage
 printf(
 "Usage:\n"
 " SCOPE DISABLE keyword\n"
 " SCOPE ENABLE keyword\n"
 " SCOPE BEFORE keyword GOSUB line\n"
 " SCOPE AFTER keyword GOSUB line\n"
 " SCOPE OVERRIDE keyword GOSUB line\n"
 " SCOPE RESTORE keyword\n"
 " SCOPE LIST | RESET\n"
 " SCOPE \"STRUCTURED\" | \"SAFE\" |"
 " \"MINIMAL\" | \"EDUCATIONAL\"\n");
 }
 return;
}

 // pi_parse_keyword - Handle KEYWORD command.
void pi_parse_keyword(Lexer *lex, RuntimeState *rt, int line_num)
{
  // KEYWORD kw PROP value - set property
  // KEYWORD kw PROP OFF   - remove property
  // KEYWORD kw             - show properties
  // KEYWORD kw DESCRIBE    - show available props
  // KEYWORD kw RESET       - clear all props
  // KEYWORD LIST            - show all
  // KEYWORD RESET           - clear everything
  //
  // Security: blocked at LEVEL 2+
 {
 if (security_check(SECOP_SYSTEM,
  line_num)) return;
 // KEYWORD LIST
 if (lexer_match_keyword(lex, KW_LIST)) {
 lexer_next(lex);
 keyword_prop_list_all();
 return;
 }

 // KEYWORD RESET (keyword form)
 if (lexer_match_keyword(lex,
 KW_RESET)) {
 lexer_next(lex);
 keyword_props_reset();
 return;
 }

 // Expect a keyword next
 if (lex->current.type != TOK_KEYWORD) {
 // Usage
 printf(
 "Usage:\n"
 " KEYWORD kw PROP value\n"
 " KEYWORD kw DESCRIBE\n"
 " KEYWORD kw RESET\n"
 " KEYWORD LIST | RESET\n");
 return;
 }

 {
 KeywordId tkw =
 lex->current.value.keyword;
 const char *tkname =
 lexer_keyword_name(tkw);
 lexer_next(lex);

 // KEYWORD kw (no args)
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 lex->current.type ==
 TOK_SEMICOLON ||
 lex->current.type ==
 TOK_COLON) {
 keyword_prop_list(tkw);
 return;
 }

 // KEYWORD kw RESET (keyword form)
 if (lexer_match_keyword(lex,
 KW_RESET)) {
 lexer_next(lex);
 keyword_prop_clear(tkw);
 printf("%s: properties"
 " cleared.\n", tkname);
 return;
 }

 // Named var sub-commands
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *nv =
 lex->current.str_start;
 int nlen =
 lex->current.str_length;

 // DESCRIBE
 if (nlen == 8 &&
 (nv[0]=='D'||nv[0]=='d') &&
 (nv[1]=='E'||nv[1]=='e') &&
 (nv[2]=='S'||nv[2]=='s') &&
 (nv[3]=='C'||nv[3]=='c') &&
 (nv[4]=='R'||nv[4]=='r') &&
 (nv[5]=='I'||nv[5]=='i') &&
 (nv[6]=='B'||nv[6]=='b') &&
 (nv[7]=='E'||nv[7]=='e')) {
 lexer_next(lex);
 keyword_prop_describe(tkw);
 return;
 }

 // RESET (as named var)
 if (nlen == 5 &&
 (nv[0]=='R'||nv[0]=='r') &&
 (nv[1]=='E'||nv[1]=='e') &&
 (nv[2]=='S'||nv[2]=='s') &&
 (nv[3]=='E'||nv[3]=='e') &&
 (nv[4]=='T'||nv[4]=='t')) {
 lexer_next(lex);
 keyword_prop_clear(tkw);
 printf("%s: properties"
 " cleared.\n", tkname);
 return;
 }

  // Property name is the named var.
  // Value follows.
 {
 char pname[MAX_PROP_NAME];
 char pval[MAX_PROP_VALUE];
 int pi;

 if (nlen > MAX_PROP_NAME - 1)
 nlen = MAX_PROP_NAME - 1;
 memcpy(pname, nv, (size_t)nlen);
 pname[nlen] = '\0';
 for (pi = 0; pi < nlen; pi++) {
 if (pname[pi] >= 'a' &&
 pname[pi] <= 'z')
 pname[pi] =
 (char)(pname[pi] - 32);
 }
 lexer_next(lex);

 // Value: string, number,
  // keyword (ON/OFF), named var 
 if (lex->current.type ==
 TOK_STRING) {
 int slen =
 lex->current.str_length;
 if (slen > MAX_PROP_VALUE - 1)
 slen = MAX_PROP_VALUE - 1;
 memcpy(pval,
 lex->current.str_start,
 (size_t)slen);
 pval[slen] = '\0';
 lexer_next(lex);
 } else if (lex->current.type ==
 TOK_NUMBER) {
 sprintf(pval, "%ld",
 lex->current.value.num_value);
 lexer_next(lex);
 } else if (lex->current.type ==
 TOK_KEYWORD) {
 // ON / OFF keywords
 const char *kn =
 lexer_keyword_name(
 lex->current.value.keyword);
 int ki;
 int kl = (int)strlen(kn);
 if (kl > MAX_PROP_VALUE - 1)
 kl = MAX_PROP_VALUE - 1;
 memcpy(pval, kn, (size_t)kl);
 pval[kl] = '\0';
 for (ki = 0; ki < kl; ki++) {
 if (pval[ki] >= 'a' &&
 pval[ki] <= 'z')
 pval[ki] =
 (char)(pval[ki] - 32);
 }
 lexer_next(lex);
 } else if (lex->current.type ==
 TOK_NAMED_VAR) {
 // ON, OFF, YES, NO etc
 int vl =
 lex->current.str_length;
 if (vl > MAX_PROP_VALUE - 1)
 vl = MAX_PROP_VALUE - 1;
 memcpy(pval,
 lex->current.str_start,
 (size_t)vl);
 pval[vl] = '\0';
 for (pi = 0; pi < vl; pi++) {
 if (pval[pi] >= 'a' &&
 pval[pi] <= 'z')
 pval[pi] =
 (char)(pval[pi] - 32);
 }
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }

 // Check for OFF = remove
 if (pi_prop_eq_ci(pval, "OFF")) {
 keyword_prop_remove(tkw,
 pname);
 printf("%s.%s removed.\n",
 tkname, pname);
 } else {
 keyword_prop_set(tkw,
 pname, pval);
 printf("%s.%s = %s\n",
 tkname, pname, pval);
 }
 return;
 }
 }
 }

 error_raise(ERR_WHAT, line_num);
 }
 return;
}

 // pi_parse_override - Handle OVERRIDE command.
void pi_parse_override(Lexer *lex, RuntimeState *rt, int line_num)
{
  // OVERRIDE keyword "text"  - change interpretation
  // OVERRIDE keyword CLEAR   - restore default
  // OVERRIDE LIST             - show active
  // OVERRIDE RESET            - clear all
  //
  // Changes how the parser interprets a keyword
  // without modifying the user's source code.
  // Security: blocked at LEVEL 2+
 {
  if (security_check(SECOP_SYSTEM,
   line_num)) return;

  // OVERRIDE LIST
  if (lexer_match_keyword(lex, KW_LIST)) {
   lexer_next(lex);
   override_list();
   return;
  }

  // OVERRIDE RESET (keyword form)
  if (lexer_match_keyword(lex,
   KW_RESET)) {
   lexer_next(lex);
   override_reset();
   return;
  }

  // OVERRIDE RESET via named var
  if (lex->current.type ==
   TOK_NAMED_VAR) {
   const char *nv =
    lex->current.str_start;
   int nlen =
    lex->current.str_length;
   if (nlen == 5 &&
    (nv[0]=='R'||nv[0]=='r') &&
    (nv[1]=='E'||nv[1]=='e') &&
    (nv[2]=='S'||nv[2]=='s') &&
    (nv[3]=='E'||nv[3]=='e') &&
    (nv[4]=='T'||nv[4]=='t')) {
    lexer_next(lex);
    override_reset();
    return;
   }
  }

  // Expect a keyword to override
  if (lex->current.type !=
   TOK_KEYWORD) {
   printf(
    "Usage:\n"
    " OVERRIDE keyword \"text\"\n"
    " OVERRIDE keyword CLEAR\n"
    " OVERRIDE LIST\n"
    " OVERRIDE RESET\n");
   return;
  }

  {
   KeywordId tkw =
    lex->current.value.keyword;
   const char *tkname =
    lexer_keyword_name(tkw);
   lexer_next(lex);

   // OVERRIDE keyword CLEAR
    // (via CLEAR keyword) 
   if (lexer_match_keyword(lex,
    KW_CLEAR)) {
    lexer_next(lex);
    override_clear(tkw);
    printf("OVERRIDE %s cleared.\n",
     tkname);
    return;
   }

   // OVERRIDE keyword CLEAR
    // (via named var) 
   if (lex->current.type ==
    TOK_NAMED_VAR) {
    const char *cv =
     lex->current.str_start;
    int cl =
     lex->current.str_length;
    if (cl == 5 &&
     (cv[0]=='C'||cv[0]=='c') &&
     (cv[1]=='L'||cv[1]=='l') &&
     (cv[2]=='E'||cv[2]=='e') &&
     (cv[3]=='A'||cv[3]=='a') &&
     (cv[4]=='R'||cv[4]=='r')) {
     lexer_next(lex);
     override_clear(tkw);
     printf(
      "OVERRIDE %s cleared.\n",
      tkname);
     return;
    }
   }

   // OVERRIDE keyword "text"
   if (lex->current.type ==
    TOK_STRING &&
    lex->current.str_start != NULL &&
    lex->current.str_length > 0) {
    char otxt[MAX_OVERRIDE_TEXT];
    int olen =
     lex->current.str_length;
    if (olen >= MAX_OVERRIDE_TEXT)
     olen = MAX_OVERRIDE_TEXT - 1;
    memcpy(otxt,
     lex->current.str_start,
     (size_t)olen);
    otxt[olen] = '\0';
    lexer_next(lex);

    if (override_set(tkw,
     otxt) == 0) {
     printf(
      "OVERRIDE %s = \"%s\"\n",
      tkname, otxt);
    }
    return;
   }

   error_raise(ERR_WHAT, line_num);
  }
 }
 return;
}

 // pi_parse_security - Handle SECURITY command.
void pi_parse_security(Lexer *lex, RuntimeState *rt, int line_num)
{
  // SECURITY "level"   - Set by name
  // SECURITY n         - Set by number (0/1/2)
  // SECURITY LEVEL n   - Set by number
  // SECURITY           - Show current level
  //
  // One-way ratchet: level can only be raised,
  // never lowered. Once RESTRICTED, it stays.
 {
  SecLevel cur = security_get_level();

  // SECURITY "name"
  if (lex->current.type == TOK_STRING) {
  char lname[32];
  int si, slen;
  slen = lex->current.str_length;
  if (slen > 31) slen = 31;
  memcpy(lname, lex->current.str_start,
   (size_t)slen);
  lname[slen] = '\0';
  lexer_next(lex);
  for (si = 0; si < SEC_COUNT; si++) {
   if (pi_str_case_equal(lname,
    security_level_name(
    (SecLevel)si))) {
   if ((SecLevel)si < cur) {
    printf("Cannot lower security"
    " from %s to %s.\n",
    security_level_name(cur),
    security_level_name(
    (SecLevel)si));
    return;
   }
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
  return;
  }

  // SECURITY n (numeric)
  if (lex->current.type == TOK_NUMBER) {
  int nlev = (int)lex->current.value.num_value;
  lexer_next(lex);
  if (nlev < 0 || nlev >= SEC_COUNT) {
   printf("Invalid level %d. "
   "Use 0 (OPEN), 1 (STANDARD), "
   "2 (RESTRICTED).\n", nlev);
   return;
  }
  if ((SecLevel)nlev < cur) {
   printf("Cannot lower security"
   " from %s to %s.\n",
   security_level_name(cur),
   security_level_name(
   (SecLevel)nlev));
   return;
  }
  security_set_level((SecLevel)nlev);
  printf("Security: %s\n",
   security_level_name(
   (SecLevel)nlev));
  return;
  }

  // SECURITY LEVEL n (named var form)
  if (lex->current.type ==
   TOK_NAMED_VAR) {
  const char *nv =
   lex->current.str_start;
  int nlen =
   lex->current.str_length;
  if (nlen == 5 &&
   (nv[0]=='L'||nv[0]=='l') &&
   (nv[1]=='E'||nv[1]=='e') &&
   (nv[2]=='V'||nv[2]=='v') &&
   (nv[3]=='E'||nv[3]=='e') &&
   (nv[4]=='L'||nv[4]=='l')) {
   int nlev;
   lexer_next(lex);
   if (lex->current.type !=
    TOK_NUMBER) {
   error_raise(ERR_WHAT,
    line_num);
   return;
   }
   nlev = (int)
    lex->current.value.num_value;
   lexer_next(lex);
   if (nlev < 0 ||
    nlev >= SEC_COUNT) {
   printf("Invalid level %d.\n",
    nlev);
   return;
   }
   if ((SecLevel)nlev < cur) {
   printf("Cannot lower security"
    " from %s to %s.\n",
    security_level_name(cur),
    security_level_name(
    (SecLevel)nlev));
   return;
   }
   security_set_level(
    (SecLevel)nlev);
   printf("Security: %s\n",
    security_level_name(
    (SecLevel)nlev));
   return;
  }
  }

  // SECURITY (no args) - show level
  printf("Security: %s",
  security_level_name(cur));
  switch (cur) {
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

 // ===== Cross-platform =====
}

 // pi_parse_module - Handle MODULE command.
 //
 // Sub-commands:
 //   MODULE                     - List all registered modules
 //   MODULE "name"              - Activate a registered module
 //   MODULE LOAD "path.dll"     - Load an external dynamic module
 //   MODULE UNLOAD "name"       - Deactivate and unload a module
 //   MODULE INFO "name"         - Show detailed info for a module
 //
 // SECURITY:
 //   Loading and activating modules requires SECOP_MODULE permission.
 //   Under RESTRICTED mode, no modules with I/O, file, system, graphics,
 //   sound, network, or USB capabilities can be activated.
 //   Under STANDARD mode, modules with SYSTEM capabilities are blocked.
 //   Under OPEN mode, all modules are permitted.
 //
 //   Dynamic LOAD additionally requires SECOP_SYSTEM because it
 //   executes arbitrary native code from a shared library.
 //
 // HOW TO LOAD EXTERNAL MODULES:
 //   MODULE LOAD "my_extension.dll"    (Windows)
 //   MODULE LOAD "./my_extension.so"   (Linux)
 //
 // HOW TO UNLOAD:
 //   MODULE UNLOAD "MY_EXTENSION"
 //   This calls the module's cleanup callback and marks it inactive.
 //   On dynamic modules, the shared library handle is freed.
 //
 // HOW MODULES AFFECT SECURITY:
 //   Each module declares a capability bitmask (CAP_IO, CAP_FILE, etc.).
 //   The interpreter dynamically checks these against the current security
 //   level. If the module requests capabilities that exceed the security
 //   level, activation is denied and the module is either:
 //   - Blocked entirely (RESTRICTED mode), or
 //   - Downgraded: only safe capabilities are honored (STANDARD mode).
 //   This is dynamic, not absolute a" changing the security level at
 //   runtime affects which modules can be activated.
void pi_parse_module(Lexer *lex, RuntimeState *rt, int line_num)
{
  // Check for sub-commands via keywords first.
  // LOAD is a keyword (KW_LOAD), so we check for it.
 if (lexer_match_keyword(lex, KW_LOAD)) {
   // MODULE LOAD "path" - Load an external .dll/.so module.
   //
   // Requires both SECOP_MODULE and SECOP_SYSTEM because
   // loading native code is a system-level operation.
  char mpath[MAX_LINE_LENGTH + 1];
  lexer_next(lex); // consume LOAD

  if (security_check(SECOP_MODULE, line_num)) return;
  if (security_check(SECOP_SYSTEM, line_num)) return;

  if (lex->current.type != TOK_STRING) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  if (lex->current.str_length >= MAX_LINE_LENGTH) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  memcpy(mpath, lex->current.str_start,
   (size_t)lex->current.str_length);
  mpath[lex->current.str_length] = '\0';
  lexer_next(lex);

  if (module_load_dynamic(mpath) == 0) {
   printf("Module loaded: %s\n", mpath);
  }
  return;
 }

 // MODULE INFO "name" - via keyword (INFO is a registered keyword)
 if (lexer_match_keyword(lex, KW_INFO)) {
  char mname[MAX_LINE_LENGTH + 1];
  const ModuleInfo *m;
  char caps[16];
  lexer_next(lex); // consume INFO

  if (lex->current.type != TOK_STRING) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  if (lex->current.str_length >= MAX_LINE_LENGTH) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  memcpy(mname, lex->current.str_start,
   (size_t)lex->current.str_length);
  mname[lex->current.str_length] = '\0';
  lexer_next(lex);

  m = module_find(mname);
  if (!m) {
   printf("Module '%s' not found.\n", mname);
   return;
  }
  module_caps_string(m->capabilities,
   caps, (int)sizeof(caps));
  printf("Module:       %s\n", m->name);
  printf("Version:      %s\n", m->version);
  printf("Description:  %s\n", m->description);
  printf("Class:        %s\n",
   module_class_name(m->mod_class));
  printf("Capabilities: %s (0x%04X)\n",
   caps, m->capabilities);
  printf("Status:       %s\n",
   module_is_active(mname)
   ? "ACTIVE" : "INACTIVE");
  printf("Security:     %s at current level %s\n",
   security_module_allowed(m->capabilities)
   ? "ALLOWED" : "BLOCKED",
   security_level_name(
   security_get_level()));
  return;
 }

 // Check for sub-commands via named variables (UNLOAD)
 if (lex->current.type == TOK_NAMED_VAR) {
  const char *nv = lex->current.str_start;
  int nlen = lex->current.str_length;

  // MODULE UNLOAD "name"
  if (nlen == 6 &&
   (nv[0]=='U'||nv[0]=='u') &&
   (nv[1]=='N'||nv[1]=='n') &&
   (nv[2]=='L'||nv[2]=='l') &&
   (nv[3]=='O'||nv[3]=='o') &&
   (nv[4]=='A'||nv[4]=='a') &&
   (nv[5]=='D'||nv[5]=='d')) {
   char mname[MAX_LINE_LENGTH + 1];
   lexer_next(lex); // consume UNLOAD

   if (lex->current.type != TOK_STRING) {
    error_raise(ERR_WHAT, line_num);
    return;
   }
   if (lex->current.str_length >= MAX_LINE_LENGTH) {
    error_raise(ERR_WHAT, line_num);
    return;
   }
   memcpy(mname, lex->current.str_start,
    (size_t)lex->current.str_length);
   mname[lex->current.str_length] = '\0';
   lexer_next(lex);

   if (module_deactivate(mname) == 0) {
    printf("Module unloaded: %s\n", mname);
   } else {
    printf("Module '%s' not found or not active.\n",
     mname);
   }
   return;
  }

  // MODULE INFO "name"
  if (nlen == 4 &&
   (nv[0]=='I'||nv[0]=='i') &&
   (nv[1]=='N'||nv[1]=='n') &&
   (nv[2]=='F'||nv[2]=='f') &&
   (nv[3]=='O'||nv[3]=='o')) {
   char mname[MAX_LINE_LENGTH + 1];
   const ModuleInfo *m;
   char caps[16];
   lexer_next(lex); // consume INFO

   if (lex->current.type != TOK_STRING) {
    error_raise(ERR_WHAT, line_num);
    return;
   }
   if (lex->current.str_length >= MAX_LINE_LENGTH) {
    error_raise(ERR_WHAT, line_num);
    return;
   }
   memcpy(mname, lex->current.str_start,
    (size_t)lex->current.str_length);
   mname[lex->current.str_length] = '\0';
   lexer_next(lex);

   m = module_find(mname);
   if (!m) {
    printf("Module '%s' not found.\n", mname);
    return;
   }
   module_caps_string(m->capabilities,
    caps, (int)sizeof(caps));
   printf("Module:       %s\n", m->name);
   printf("Version:      %s\n", m->version);
   printf("Description:  %s\n", m->description);
   printf("Class:        %s\n",
    module_class_name(m->mod_class));
   printf("Capabilities: %s (0x%04X)\n",
    caps, m->capabilities);
   printf("Status:       %s\n",
    module_is_active(mname)
    ? "ACTIVE" : "INACTIVE");
   printf("Security:     %s at current level %s\n",
    security_module_allowed(m->capabilities)
    ? "ALLOWED" : "BLOCKED",
    security_level_name(
    security_get_level()));
   return;
  }
 }

 // MODULE "name" - Activate a registered module
 if (lex->current.type == TOK_STRING) {
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
  return;
 }

 // MODULE LIST (via keyword)
 if (lexer_match_keyword(lex, KW_LIST)) {
  lexer_next(lex);
  // fall through to listing below
 }

  // MODULE (no args) or MODULE LIST - List all modules.
 {
  int mi, mc;
  mc = module_count();
  if (mc == 0) {
   printf("No modules registered.\n");
  } else {
   printf("%-16s %-10s %-8s %-6s %s\n",
    "Module", "Class", "Version",
    "Caps", "Status");
   printf("%-16s %-10s %-8s %-6s %s\n",
    "------", "-----", "-------",
    "----", "------");
   for (mi = 0; mi < mc; mi++) {
    const ModuleInfo *m;
    char caps[12];
    m = module_get(mi);
    if (!m) continue;
    module_caps_string(
     m->capabilities,
     caps, (int)sizeof(caps));
    printf("%-16s %-10s %-8s %-6s %s\n",
     m->name,
     module_class_name(
      m->mod_class),
     m->version,
     caps,
     module_is_loaded(mi)
      ? "ACTIVE"
      : "INACTIVE");
   }
  }
 }
 return;

 // ===== Security =====
}


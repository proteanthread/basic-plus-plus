/*
 * ---
 * BASIC++ Interpreter - parser_progmgmt.c
 * ---
 *
 * Program editing & management commands.
 *
 * RENUM, DELETE, EDIT, AUTO, COMPILE, BSAVE, BLOAD.
 *
 * ---
 */

#include "parser_internal.h"
#include "pcode.h"

/*
 * pi_parse_renum - Handle RENUM command.
 */
void pi_parse_renum(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_delete - Handle DELETE command.
 */
void pi_parse_delete(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_edit - Handle EDIT command.
 */
void pi_parse_edit(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * EDIT [line_number]
 *
 * Display a program line for editing.
 * The user can then retype the line number
 * followed by new text to replace it, or
 * press Enter to keep it unchanged.
 *
 * EDIT without a number displays the first
 * program line. EDIT n displays line n.
 *
 * This integrates with the REPL's existing
 * line-entry model: typing a numbered line
 * replaces the stored version.
 */
 {
 int target = -1;
 int i, found = 0;

 /* Parse optional line number */
 if (lex->current.type == TOK_NUMBER) {
  target = (int)lex->current.value
   .num_value;
  lexer_next(lex);
 } else if (lex->current.type != TOK_EOF &&
  lex->current.type != TOK_CR &&
  lex->current.type != TOK_COLON) {
  target = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
 }

 if (rt->program == NULL ||
  rt->program->count == 0) {
  printf("No program.\n");
  return;
 }

 /* Find the target line */
 if (target < 0) {
  /* No argument: show first line */
  printf("%s\n",
   rt->program->lines[0].text);
  found = 1;
 } else {
  for (i = 0; i < rt->program->count;
   i++) {
   if (rt->program->lines[i]
    .line_number == target) {
    printf("%s\n",
     rt->program->lines[i].text);
    found = 1;
    break;
   }
  }
 }

 if (!found) {
  printf("Line %d not found.\n", target);
 }
 }
 return;
}


/*
 * pi_parse_auto - Handle AUTO command.
 */
void pi_parse_auto(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_compile - Handle COMPILE command.
 */
void pi_parse_compile(Lexer *lex, RuntimeState *rt, int line_num)
{
 if (security_check(SECOP_COMPILE, line_num))
 return;
 {
 char fname[MAX_LINE_LENGTH + 1];
 char target[MAX_LINE_LENGTH + 1] = "";
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

  if (lex->current.type == TOK_COMMA) {
      lexer_next(lex);
      if (lex->current.type != TOK_STRING || lex->current.str_length >= MAX_LINE_LENGTH) {
          error_raise(ERR_WHAT, line_num);
          return;
      }
      memcpy(target, lex->current.str_start, (size_t)lex->current.str_length);
      target[lex->current.str_length] = '\0';
      lexer_next(lex);
  }

  {
      int len = (int)strlen(fname);
      if (len >= 4 && (strcasecmp(&fname[len - 4], ".bpp") == 0)) {
          PCodeProgram pcode;
          if (pcode_compile(rt->program, &pcode) == 0) {
              printf("Compiled %d PCode instructions.\n", pcode.count);
              pcode_free(&pcode);
          } else {
              printf("Failed to compile PCode.\n");
          }
      } else {
          compiler_compile(rt->program, fname, target);
      }
  }
  return;
 }
}/* ===== Bytecode commands ===== */

/*
 * pi_parse_bsave - Handle BSAVE command.
 */
void pi_parse_bsave(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_bload - Handle BLOAD command.
 */
void pi_parse_bload(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}


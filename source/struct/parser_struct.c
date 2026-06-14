/*
 * ---
 * BASIC++ Interpreter - parser_struct.c
 * ---
 *
 * Structured programming constructs.
 *
 * SELECT, CASE, EXIT, SUB, FUNCTION, DECLARE,
 * CALL, PROCEDURE, DEFINE, ENDDEFINE, LOCAL.
 *
 * ---
 */

#include "parser_internal.h"

/*
 * pi_parse_select - Handle SELECT command.
 */
void pi_parse_select(Lexer *lex, RuntimeState *rt, int line_num)
{
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
 if (lexer_match_keyword(lex, KW_CASE)) {
  lexer_next(lex); /* consume CASE */
 } else if (lexer_match_keyword(lex, KW_ON)) {
  lexer_next(lex); /* consume ON */
 } else {
  error_raise(ERR_WHAT, line_num);
  return;
 }
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
 if ((kk == KW_CASE || kk == KW_ON || kk == KW_REMAINDER) && depth == 0){
  lexer_next(&cl);
  /* CASE ELSE or REMAINDER */
  if (kk == KW_REMAINDER || (cl.current.type ==
  TOK_KEYWORD &&
  cl.current.value.keyword
  == KW_ELSE)) {
  found = 1;
  rt->next_index = idx + 1;
  break;
  }
  
  /* For ON, optionally skip 'var =' */
  if (kk == KW_ON) {
   if (cl.current.type == TOK_NAMED_VAR || cl.current.type == TOK_VARIABLE) {
    Lexer probe = cl;
    lexer_next(&probe);
    if (probe.current.type == TOK_EQUALS) {
     cl = probe;
     lexer_next(&cl);
    }
   }
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
}

/*
 * pi_parse_case - Handle CASE command.
 */
void pi_parse_case(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_exit - Handle EXIT command.
 */
void pi_parse_exit(Lexer *lex, RuntimeState *rt, int line_num)
{
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
 } else if (lex->current.type == TOK_NAMED_VAR || lex->current.type == TOK_VARIABLE) {
  /*
   * EXIT label - SuperBASIC REPeat exit.
   * Pop the REPeat frame and scan forward to END REPeat / ENDREPEAT.
   */
  int fi;
  lexer_next(lex); /* consume label */
  /* Find and pop the REPeat frame */
  for (fi = rt->stack_top - 1; fi >= 0; fi--) {
   if (rt->stack[fi].type == FRAME_REPEAT) {
    /* Pop down to this frame */
    rt->stack_top = fi;
    /* Scan forward for ENDREPEAT or END REPEAT */
    {
     int idx = rt->current_index + 1;
     ProgramStore *pgm = rt->program;
     int depth = 0;
     while (idx < pgm->count) {
      Lexer cl;
      const char *cl2 = pgm->lines[idx].text;
      lexer_init(&cl, cl2);
      lexer_next(&cl);
      if (cl.current.type == TOK_NUMBER)
       lexer_next(&cl);
      if (cl.current.type == TOK_KEYWORD) {
       KeywordId ek = cl.current.value.keyword;
       if (ek == KW_REPEAT) {
        depth++;
       } else if (ek == KW_ENDREPEAT) {
        if (depth > 0) {
         depth--;
        } else {
         rt->next_index = idx + 1;
         return;
        }
       } else if (ek == KW_END) {
        /* Check if it is END REPEAT */
        lexer_next(&cl);
        if (cl.current.type == TOK_KEYWORD && cl.current.value.keyword == KW_REPEAT) {
         if (depth > 0) {
          depth--;
         } else {
          rt->next_index = idx + 1;
          return;
         }
        }
       }
      }
      idx++;
     }
     /* Past program end */
     rt->next_index = pgm->count;
    }
    return;
   }
  }
  error_raise(ERR_WHAT, line_num);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;
}

/*
 * pi_parse_sub - Handle SUB command.
 */
void pi_parse_sub(Lexer *lex, RuntimeState *rt, int line_num)
{
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
 int is_func = 0; /* SUB */
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
}

/*
 * pi_parse_function - Handle FUNCTION command.
 */
void pi_parse_function(Lexer *lex, RuntimeState *rt, int line_num)
{
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
 int is_func = 1; /* FUNCTION */
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
}

/*
 * pi_parse_declare - Handle DECLARE command.
 */
void pi_parse_declare(Lexer *lex, RuntimeState *rt, int line_num)
{
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
}

/*
 * pi_parse_call - Handle CALL command.
 */
void pi_parse_call(Lexer *lex, RuntimeState *rt, int line_num)
{
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
 pi_set_param_by_name(rt,
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
 pi_set_param_by_name(rt,
 sd->params[i], av);
 }
 }

 rt->in_sub_index =
 (int)(sd - rt->subs);
 rt->next_index = sd->body_index;
 lexer_skip_to_end(lex);
 }
 return;
}

/*
 * pi_parse_procedure - Handle PROCEDURE command.
 */
void pi_parse_procedure(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * PROCedure - only valid after DEFine.
  * If encountered standalone, that's an error.
  */
 error_raise(ERR_WHAT, line_num);
 return;
}

/*
 * pi_parse_define - Handle DEFINE command.
 */
void pi_parse_define(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * DEFine PROCedure name(params) -> acts like SUB
  * DEFine FuNction name(params) -> acts like FUNCTION
  */
 if (lexer_match_keyword(lex, KW_PROCEDURE)) {
  lexer_next(lex);
  pi_parse_sub(lex, rt, line_num);
 } else if (lexer_match_keyword(lex, KW_FN) || lexer_match_keyword(lex, KW_FUNCTION)) {
  lexer_next(lex);
  pi_parse_function(lex, rt, line_num);
 } else if (lexer_match_keyword(lex, KW_CASE)) {
  lexer_next(lex); /* consume CASE */
 } else if (lexer_match_keyword(lex, KW_ON)) {
  lexer_next(lex); /* consume ON */
 } else {
  error_raise(ERR_WHAT, line_num);
  return;
 }
}

/*
 * pi_parse_enddefine - Handle ENDDEFINE command.
 */
void pi_parse_enddefine(Lexer *lex, RuntimeState *rt, int line_num)
{
 /* Acts like END SUB / END FUNCTION return */
 StackFrame frame;
 int i;
 if (runtime_pop(rt, FRAME_SUB, &frame) != 0) {
  error_raise(ERR_HOW, line_num);
  return;
 }
 for (i = 0; i < MAX_VARIABLES; i++)
  rt->variables[i] = frame.data.sub_call.saved_vars[i];
 for (i = 0; i < MAX_STRING_VARS; i++)
  rt->string_vars[i] = frame.data.sub_call.saved_strvars[i];
 rt->in_sub_index = -1;
 rt->next_index = frame.data.sub_call.return_index;
}

/*
 * pi_parse_local - Handle LOCAL command.
 */
void pi_parse_local(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * LOCal var [, var ...]
  *
  * Push current values of the listed variables
  * onto a save stack, then zero them. On
  * END DEFine / RETurn, the saved values are
  * restored (handled by scope.c if available,
  * or by the GOSUB return mechanism).
  *
  * For now, LOCal is accepted but acts as a
  * simple variable initializer (sets to 0).
  * Full scope save/restore requires scope.c.
  */
 while (lex->current.type != TOK_EOF &&
  lex->current.type != TOK_CR &&
  lex->current.type != TOK_COLON) {
  if (lex->current.type == TOK_NAMED_VAR ||
   lex->current.type == TOK_VARIABLE) {
   /* Parse and zero the variable */
   char *vn;
   int vn_len;
   if (lex->current.type ==
    TOK_NAMED_VAR) {
    vn = (char *)
     lex->current.str_start;
    vn_len =
     lex->current.str_length;
   } else {
    /* single-letter var */
    vn = NULL;
    vn_len = 0;
   }
   if (vn == NULL) {
    /* Single letter: A-Z */
    int vi =
     lex->current.value.var_name
     - 'A';
    rt->variables[vi] =
     bval_float(0.0);
   } else {
    /* Named variable: set to 0 */
    runtime_set_named_var_bval(rt, vn,
     vn_len, bval_float(0.0));
   }
   lexer_next(lex);
   if (lex->current.type == TOK_COMMA)
    lexer_next(lex);
  } else {
   lexer_next(lex); /* skip junk */
  }
 }
 return;
}


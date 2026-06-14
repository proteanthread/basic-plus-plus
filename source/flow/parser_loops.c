/*
 * ---
 * BASIC++ Interpreter - parser_loops.c
 * ---
 *
 * Loop construct handlers: FOR/NEXT, WHILE/WEND, DO/LOOP.
 *
 * Handles all loop types across all dialects, including
 * multi-line and inline FOR variants.
 *
 * ---
 */

#include "parser_internal.h"

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
void pi_parse_for(Lexer *lex, RuntimeState *rt, int line_num)
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

 /* Parse optional STEP (or BY -- SUPER BASIC alternative) */
 step_val = 1; /* default step */
 if (lexer_match_keyword(lex, KW_STEP) ||
 lexer_match_keyword(lex, KW_BY)) {
 lexer_next(lex); /* consume STEP or BY */
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
void pi_parse_next(Lexer *lex, RuntimeState *rt, int line_num)
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
void pi_parse_while(Lexer *lex, RuntimeState *rt, int line_num)
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
void pi_parse_wend(Lexer *lex, RuntimeState *rt, int line_num)
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
void pi_parse_do(Lexer *lex, RuntimeState *rt, int line_num)
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
void pi_parse_loop(Lexer *lex, RuntimeState *rt, int line_num)
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

/*
 * parse_repeat - Parse and execute REPeat statement.
 * Syntax: REPeat name
 * SuperBASIC infinite loop.
 */
void pi_parse_repeat(Lexer *lex, RuntimeState *rt, int line_num)
{
 char var_name[MAX_VAR_NAME_LEN + 1];
 int name_len = 0;
 StackFrame frame;

 /* Parse variable name */
 if (lex->current.type == TOK_NAMED_VAR) {
 name_len = lex->current.str_length;
 if (name_len > MAX_VAR_NAME_LEN) name_len = MAX_VAR_NAME_LEN;
 memcpy(var_name, lex->current.str_start, (size_t)name_len);
 var_name[name_len] = '\0';
 lexer_next(lex);
 } else if (lex->current.type == TOK_VARIABLE) {
 var_name[0] = lex->current.value.var_name;
 var_name[1] = '\0';
 name_len = 1;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /* Push REPEAT frame */
 frame.type = FRAME_REPEAT;
 memcpy(frame.data.repeat_loop.name, var_name, (size_t)(name_len + 1));
 frame.data.repeat_loop.body_index = rt->current_index + 1;

 if (runtime_push(rt, &frame) != 0) {
 return; /* ERR_SORRY already raised */
 }
}

/*
 * parse_endrepeat - Parse END REPeat
 * Syntax: END REPeat [name]
 */
void pi_parse_endrepeat(Lexer *lex, RuntimeState *rt, int line_num)
{
 char var_name[MAX_VAR_NAME_LEN + 1];
 int name_len = 0;
 StackFrame frame;

 /* Optional name */
 if (lex->current.type == TOK_NAMED_VAR) {
 name_len = lex->current.str_length;
 if (name_len > MAX_VAR_NAME_LEN) name_len = MAX_VAR_NAME_LEN;
 memcpy(var_name, lex->current.str_start, (size_t)name_len);
 var_name[name_len] = '\0';
 lexer_next(lex);
 } else if (lex->current.type == TOK_VARIABLE) {
 var_name[0] = lex->current.value.var_name;
 var_name[1] = '\0';
 name_len = 1;
 lexer_next(lex);
 }

 if (runtime_pop(rt, FRAME_REPEAT, &frame) != 0) {
 return; /* ERR_HOW: no matching REPeat */
 }

 /* Verify name matches if provided */
 if (name_len > 0) {
 if (pi_str_case_equal(var_name, frame.data.repeat_loop.name) == 0) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 }

 /* Unconditionally loop back to body */
 rt->next_index = frame.data.repeat_loop.body_index - 1; /* evaluate REPeat again to push frame */
 lexer_skip_to_end(lex);
 (void)line_num;
}

/*
 * parse_endfor - Parse END FOR
 * Syntax: END FOR var
 * Handled identically to NEXT var
 */
void pi_parse_endfor(Lexer *lex, RuntimeState *rt, int line_num)
{
 pi_parse_next(lex, rt, line_num);
}

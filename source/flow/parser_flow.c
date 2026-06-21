/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_flow.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Program flow control statements including GOTO, GOSUB/RETURN, loops (FOR/NEXT, WHILE/WEND, DO/LOOP), and event/interrupt handlers.
 *
 * 2. WHAT TO EXPECT:
 *    Maintains execution pointers and nested flow stack frames (such as FOR loop frames).
 *
 * 3. WHAT CAN BE CHANGED:
 *    Maximum loop nesting limits, flow tracing printouts.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    GOSUB stack logic, execution loop frame structure, line number target checking.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure nested flow stacks do not overflow (increase MAX_STACK_DEPTH). Verify return addresses align with target loops.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_flow.c
 // ---
 //
 // Control flow handlers: IF/THEN/ELSE, UNLESS, GOTO, GOSUB,
 // RETURN, END, STOP, REM.
 //
 // Handles both single-line and block IF, structured exception
 // handling (WHEN/USE/END WHEN), and flow transfer.
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

 // block_if_skip_to_end - Skip forward from current position to END IF.
 //
 // Used when the TRUE branch of a block IF has been executed and
 // we encounter ELSEIF or ELSE - we need to skip all remaining
 // branches to END IF.
 //
 // Tracks nesting depth to handle nested block IFs correctly.
void pi_block_if_skip_to_end(RuntimeState *rt, int line_num)
{
 ProgramStore *pgm = rt->program;
 int idx = rt->current_index + 1;
 int depth = 0;

 while (idx < pgm->count) {
 Lexer cl;
 const char *cline = pgm->lines[idx].text;
 lexer_init(&cl, cline);

 // Skip line number
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);

 if (cl.current.type == TOK_KEYWORD) {
 KeywordId kk = cl.current.value.keyword;

 // Check for nested IF (block form)
 if (kk == KW_IF) {
 // Scan to see if this IF has THEN at EOL
 lexer_next(&cl);
 // Skip condition tokens
 while (cl.current.type != TOK_EOF &&
 cl.current.type != TOK_CR) {
 if (cl.current.type == TOK_KEYWORD &&
 cl.current.value.keyword == KW_THEN) {
 lexer_next(&cl);
 // THEN at EOL = block IF
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

 // END IF at our depth
 if (kk == KW_END) {
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD &&
 cl.current.value.keyword == KW_IF) {
 if (depth > 0) {
 depth--;
 } else {
 // Found our END IF
 rt->current_index = idx;
 rt->next_index = idx;
 return;
 }
 }
 }

 // ENDIF (single word) at our depth
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

 // No END IF found
 error_raise(ERR_WHAT, line_num);
}

 // block_if_scan - Scan forward for ELSEIF, ELSE, or END IF.
 //
 // Used when a block IF condition is FALSE. Scans forward to find
 // the next ELSEIF (to try another condition), ELSE (to execute
 // the default), or END IF (to skip everything).
 //
 // Sets rt->current_index and rt->next_index to the found line.
 // Returns the keyword found: KW_ELSEIF, KW_ELSE, KW_ENDIF, or KW_END.
KeywordId pi_block_if_scan(RuntimeState *rt, int line_num)
{
 ProgramStore *pgm = rt->program;
 int idx = rt->current_index + 1;
 int depth = 0;

 while (idx < pgm->count) {
 Lexer cl;
 const char *cline = pgm->lines[idx].text;
 lexer_init(&cl, cline);

 // Skip line number
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);

 if (cl.current.type == TOK_KEYWORD) {
 KeywordId kk = cl.current.value.keyword;

 // Nested block IF
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

 // END IF
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

 // ENDIF (single word)
 if (kk == KW_ENDIF) {
 if (depth > 0) {
 depth--;
 } else {
 rt->current_index = idx;
 rt->next_index = idx;
 return KW_ENDIF;
 }
 }

 // ELSEIF at our depth
 if (kk == KW_ELSEIF && depth == 0) {
 rt->current_index = idx;
 rt->next_index = idx;
 return KW_ELSEIF;
 }

 // ELSE at our depth
 if (kk == KW_ELSE && depth == 0) {
 rt->current_index = idx;
 rt->next_index = idx;
 return KW_ELSE;
 }
 }
 idx++;
 }

 // No matching END IF
 error_raise(ERR_WHAT, line_num);
 return KW_END; // unreachable
}

 // when_exception_scan - Scan forward for USE and END WHEN.
 //
 // Starting from the WHEN EXCEPTION IN line, find the matching
 // USE (handler) and END WHEN (block end) at the same depth.
 //
 // Returns 1 on success, 0 on error. Populates use_idx and
 // end_when_idx with ProgramStore indices.
int pi_when_exception_scan(RuntimeState *rt, int when_idx,
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

 // Skip line number
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);

 if (cl.current.type == TOK_KEYWORD) {
 KeywordId kk = cl.current.value.keyword;

 // Nested WHEN EXCEPTION IN
 if (kk == KW_WHEN) {
 depth++;
 idx++;
 continue;
 }

 // USE at our depth
 if (kk == KW_USE && depth == 0) {
 found_use = idx;
 }

 // END WHEN
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

 // No matching END WHEN
 error_raise(ERR_WHAT, line_num);
 return 0;
}

void pi_parse_if(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue cond_val;
 int condition;

 // Parse condition as a full expression.
 // Comparisons (=, <, >, etc.) and logical operators
 // (AND, OR, NOT) are handled by parse_expression_bval.
 // Result: non-zero = true, zero = false.
 cond_val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 condition = (bval_to_int(&cond_val) != 0);

 // Consume optional THEN keyword (GW-BASIC uses THEN,
 // PATB does not).
 if (dialect_get_config()->has_then_keyword) {
 if (lexer_match_keyword(lex, KW_THEN)) {
 lexer_next(lex); // consume THEN
 }
 }

 // Block IF detection: if THEN is followed by end-of-line,
 // this is a multi-line block IF.
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR) {
 // ========== BLOCK IF ==========
 if (condition) {
 // TRUE: increment block_if_depth so that
 // ELSEIF/ELSE handlers know to skip to END IF.
 // Execution continues on the next line naturally.
 rt->block_if_depth++;
 return;
 } else {
 // FALSE: scan forward for ELSEIF/ELSE/END IF.
 KeywordId found = pi_block_if_scan(rt, line_num);

 if (found == KW_ELSEIF) {
 // Found ELSEIF - the exec loop will execute
 // this line, which triggers the ELSEIF handler.
 // The ELSEIF handler evaluates its condition.
 return;
 }
 if (found == KW_ELSE) {
 // Found ELSE - execution continues on the
 // line AFTER the ELSE.
 return;
 }
 // KW_ENDIF - skip past it, done
 return;
 }
 }

 // ========== SINGLE-LINE IF (original behavior) ==========

 if (condition) {
 // TRUE path: execute THEN clause.
 //
 // Special case: IF...THEN linenum
 // In GW-BASIC, "IF A=5 THEN 100" means
 // "IF A=5 THEN GOTO 100". If the next token
 // is a number, treat as implicit GOTO.
 if (lex->current.type == TOK_NUMBER) {
 int target = (int)lex->current.value.num_value;
 lexer_next(lex);
 vm_jump(rt, target, line_num);
 lexer_skip_to_end(lex);
 return;
 }
 pi_parse_statement(lex, rt, line_num);
 if (error_occurred()) return;

 // Check if there's an ELSE to skip.
 // The multi-statement handler may have stopped
 // at a colon. We need to scan forward through
 // remaining tokens looking for ELSE.
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ELSE) {
 // Found ELSE - skip rest of line
 lexer_skip_to_end(lex);
 return;
 }
 // Skip colon separators and continue
 if (lex->current.type == TOK_COLON) {
 lexer_next(lex);
 // Check for ELSE after colon
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword ==
 KW_ELSE) {
 lexer_skip_to_end(lex);
 return;
 }
 // Not ELSE - execute this statement
 // (part of THEN clause) 
 pi_parse_statement(lex, rt, line_num);
 if (error_occurred()) return;
 } else {
 break;
 }
 }
 } else {
 // FALSE path: skip THEN clause, find ELSE.
 //
 // Scan the raw source string for " ELSE "
 // boundary. We must skip over quoted strings
 // to avoid matching ELSE inside a string
 // literal like PRINT "ELSE".
 const char *src = lex->source;
 int len = lex->length;
 int p = lex->pos;
 int found_else = -1;

 while (p < len) {
 // Skip string literals
 if (src[p] == '"') {
 p++;
 while (p < len && src[p] != '"') p++;
 if (p < len) p++; // skip closing quote
 continue;
 }
 // Look for ELSE keyword boundary
 if ((src[p] == 'E' || src[p] == 'e') &&
 p + 4 <= len) {
 int ok = 1;
 // Check "ELSE" case-insensitive
 if ((src[p+1] != 'L' && src[p+1] != 'l')
 || (src[p+2] != 'S' && src[p+2] != 's')
 || (src[p+3] != 'E' &&
 src[p+3] != 'e')) {
 ok = 0;
 }
 // Check word boundary before
 if (ok && p > 0 &&
 ((src[p-1] >= 'A' && src[p-1] <= 'Z')
 || (src[p-1] >= 'a' &&
 src[p-1] <= 'z'))) {
 ok = 0;
 }
 // Check word boundary after
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
 // Reposition the lexer after "ELSE"
 // and execute the ELSE clause.
 lex->pos = found_else;
 lexer_next(lex); // prime first token

 // Skip optional whitespace already handled
 // by lexer_next 
 pi_parse_statement(lex, rt, line_num);
 } else {
 // No ELSE - skip entire line
 lexer_skip_to_end(lex);
 }
 }
}

 // parse_unless - Parse and execute UNLESS (SUPER BASIC).
 //
 // UNLESS is the negated form of IF, inspired by JOSS.
 // Syntax:
 //   UNLESS condition THEN statement  (single-line)
 //   UNLESS condition                 (block form)
 //     ...
 //   END IF
 //
 // Equivalent to: IF NOT (condition) THEN statement
 //
 // Example:
 //   10 UNLESS X = 0 THEN PRINT "nonzero"
 //   (prints "nonzero" when X is NOT zero)
void pi_parse_unless(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue cond_val;
 int condition;

 cond_val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 // Negate: UNLESS X means IF NOT X
 condition = (bval_to_int(&cond_val) == 0);

 // Consume optional THEN
 if (dialect_get_config()->has_then_keyword) {
 if (lexer_match_keyword(lex, KW_THEN)) {
 lexer_next(lex);
 }
 }

 // Block UNLESS (THEN at end of line)
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR) {
 if (condition) {
 rt->block_if_depth++;
 return;
 } else {
 pi_block_if_scan(rt, line_num);
 return;
 }
 }

 // Single-line UNLESS
 if (condition) {
 if (lex->current.type == TOK_NUMBER) {
 int target = (int)lex->current.value.num_value;
 lexer_next(lex);
 vm_jump(rt, target, line_num);
 lexer_skip_to_end(lex);
 return;
 }
 pi_parse_statement(lex, rt, line_num);
 } else {
 lexer_skip_to_end(lex);
 }
}

 // parse_goto - Parse and execute GOTO.
 //
 // Syntax: GOTO expression
 //
 // Evaluates the expression to get a target line number, then
 // sets the runtime's next_index to the target line. If the
 // target line doesn't exist, raises ERR_HOW.
 //
 // PATB rule: GOTO must be the last command on a line.
void pi_parse_goto(Lexer *lex, RuntimeState *rt, int line_num)
{
 // Check for label-style GOTO (e.g., GOTO MyLabel).
 // If the current token is an identifier (not a number),
 // try resolving it as a label first.
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

 // Use VM control flow primitive
 vm_jump(rt, (int)target, line_num);

 // ECMA-55 s10.4: GOTO must not transfer control
 // into the body of a FOR..NEXT loop. In strict
 // mode, check active FOR frames on the stack.
 // Warn if the target falls within a FOR body
 // that was entered from a different context.
 // Warning only - execution continues normally.
 if (!error_occurred() && dialect_is_strict()) {
 int fi;
 int tgt_idx = rt->next_index;
 for (fi = 0; fi < rt->stack_top; fi++) {
 if (rt->stack[fi].type == FRAME_FOR) {
 int body = rt->stack[fi].data
 .for_loop.body_index;
 // If jumping into the middle of a
 // loop body (after the FOR but the
 // jump didn't come from within the
 // loop), warn. A rough check: if
 // the target is >= body_index and
 // the jump source is outside, warn.
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

 // GOTO must be last - skip to end of line
 lexer_skip_to_end(lex);
}

 // parse_gosub - Parse and execute GOSUB.
 //
 // Syntax: GOSUB expression
 //
 // Pushes the return address (next line after current) onto the
 // stack, then transfers to the target line.
 //
 // PATB rule: GOSUB must be the last command on a line.
void pi_parse_gosub(Lexer *lex, RuntimeState *rt, int line_num)
{
 // Check for label-style GOSUB
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

 // Use VM control flow primitive
 vm_call(rt, (int)target, line_num);
 }

 // GOSUB must be last - skip to end of line
 lexer_skip_to_end(lex);
}

 // parse_return - Parse and execute RETURN.
 //
 // Pops a GOSUB frame from the stack and transfers to the
 // return address.
 //
 // PATB rule: RETURN must be the last command on a line.
void pi_parse_return(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)lex;

 // Use VM control flow primitive
 vm_return_sub(rt, line_num);

 // RETURN must be last - skip to end
 lexer_skip_to_end(lex);
}

 // parse_end - Parse END statement.
 //
 // Stops program execution and returns to the READY prompt.
void pi_parse_end(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)line_num;

 // END IF - end of block IF.
 // Decrement block_if_depth if inside a true block.
 if (lexer_match_keyword(lex, KW_IF)) {
 lexer_next(lex); // consume IF
 if (rt->block_if_depth > 0)
 rt->block_if_depth--;
 return;
 }

 // END WHEN - end of WHEN EXCEPTION block.
 // Pop the exception frame if present.
 // (May already be popped by USE fall-through or
 // CONTINUE - safe to check.)
 if (lexer_match_keyword(lex, KW_WHEN)) {
 int i;
 lexer_next(lex); // consume WHEN
 // Find and pop innermost exception frame
 for (i = rt->stack_top - 1; i >= 0; i--) {
 if (rt->stack[i].type == FRAME_EXCEPTION) {
 rt->stack_top = i;
 break;
 }
 }
 return;
 }

 // END SELECT is a no-op when reached by fall-through.
 // The CASE handler already jumped past it.
 if (lexer_match_keyword(lex, KW_SELECT)) {
 lexer_next(lex); // consume SELECT
 return;
 }

 // END SUB - return from subprogram.
 // Pop FRAME_SUB frame, restore saved variables.
 if (lexer_match_keyword(lex, KW_SUB)) {
 StackFrame frame;
 lexer_next(lex); // consume SUB

 if (runtime_pop(rt, FRAME_SUB, &frame) != 0) {
 return; // error already raised
 }

 // Pop scope stack (restores all vars including named)
 scope_stack_pop(&rt->scope_stack, rt);
 return;
 }

 // END FUNCTION - same as END SUB but for functions.
 if (lexer_match_keyword(lex, KW_FUNCTION)) {
 StackFrame frame;
 lexer_next(lex); // consume FUNCTION

 if (runtime_pop(rt, FRAME_SUB, &frame) != 0) {
 return;
 }

 // Pop scope stack (restores all vars including named)
 scope_stack_pop(&rt->scope_stack, rt);
 return;
 }

 // END DEF - end of multi-line DEF FN (ECMA-116).
 // Same as END FUNCTION - pop FRAME_SUB, restore vars.
 if (lexer_match_keyword(lex, KW_DEF)) {
 StackFrame frame;
 lexer_next(lex); // consume DEF

 if (runtime_pop(rt, FRAME_SUB, &frame) != 0) {
 return;
 }

 // Pop scope stack (restores all vars including named)
 scope_stack_pop(&rt->scope_stack, rt);
 return;
 }

 // END REPeat - end of REPeat block (SuperBASIC)
 if (lexer_match_keyword(lex, KW_REPEAT)) {
 lexer_next(lex); // consume REPEAT
 pi_parse_endrepeat(lex, rt, line_num);
 return;
 }

 // END FOR - end of FOR block (SuperBASIC)
 if (lexer_match_keyword(lex, KW_FOR)) {
 lexer_next(lex); // consume FOR
 pi_parse_endfor(lex, rt, line_num);
 return;
 }

 // END ATOMIC - commit the atomic block.
 // Finalizes all journaled writes.
 if (lexer_match_keyword(lex, KW_ATOMIC)) {
 lexer_next(lex); // consume ATOMIC
 pi_parse_end_atomic(line_num);
 return;
 }

 // Use VM state machine
 vm_halt(rt);

 lexer_skip_to_end(lex);
}

 // parse_stop - Parse STOP statement.
 //
 // Same as END but may print a message. For now, identical to END.
 // PATB rule: STOP must be the last command on a line.
void pi_parse_stop(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)line_num;

 printf("[STOP at line %d]\n", line_num);

 // Use VM_PAUSED so CONT can resume
 vm_set_state(rt, VM_PAUSED);

 lexer_skip_to_end(lex);
}

 // parse_rem - Parse REM (remark/comment).
 //
 // Skips the entire rest of the line. No execution effect.
void pi_parse_rem(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)rt;
 (void)line_num;

 lexer_skip_to_end(lex);
}

 // parse_list_cmd - Parse LIST command.
 //
 // GW-BASIC compatible syntax with BASIC++ extensions:
 // LIST -> list all lines
 // LIST n -> list single line n
 // LIST n- -> list from line n to end
 // LIST -n -> list from start to line n
 // LIST n-m -> list lines n through m
 // LIST n,m-p -> list line n, then lines m through p
 // LIST n,m,p -> list lines n, m, and p individually
 //
 // Comma separates independent segments. Each segment is
 // either a single line number or a range (with '-').


// === Control flow handlers moved from parser.c ===

 // pi_parse_on - Handle ON in control flow.
void pi_parse_on(Lexer *lex, RuntimeState *rt, int line_num)
{
 // ON expr GOTO l1,l2,l3 - computed GOTO
 // ON expr GOSUB l1,l2,l3 - computed GOSUB
 // ON ERROR GOTO n - error handler
 {
 // Peek: if next token is ERROR, handle
 // as ON ERROR GOTO. Otherwise, parse as
 // ON expr GOTO/GOSUB line-list.
  if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword ==
 KW_ERROR) {
 // ON ERROR GOTO n
 long target;
 if (!dialect_check_feature("ON ERROR",
 dialect_get_config()->has_on_error,
 line_num))
 return;
 lexer_next(lex); // consume ERROR
 if (lex->current.type != TOK_KEYWORD
 || lex->current.value.keyword !=
 KW_GOTO) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume GOTO
 target = parse_expression(
 lex, rt, line_num);
 rt->on_error_line = (int)target;
 }
   // ON COM(n) / KEY(n) / PEN / PLAY(n) /
   // STRIG(n) / TIMER(n) / BREAK GOSUB line
   //
   // Event trapping handler registration.
   // Stores the GOSUB target line in the
   // appropriate RuntimeState field for
   // event_poll() to dispatch.
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
    == KW_TIMER ||
    lex->current.value.keyword
    == KW_TRAP ||
    lex->current.value.keyword
    == KW_BREAK)) {
   KeywordId evkw =
    lex->current.value.keyword;
   long ev_arg = 0;
   lexer_next(lex);

   // Consume optional (n)
   if (lex->current.type ==
       TOK_LPAREN) {
    lexer_next(lex);
    ev_arg = parse_expression(
     lex, rt, line_num);
    if (error_occurred()) return;
    if (lex->current.type ==
        TOK_RPAREN)
     lexer_next(lex);
   }

   // Expect GOSUB or GOTO
   if (!lexer_match_keyword(lex,
    KW_GOSUB) &&
       !lexer_match_keyword(lex,
    KW_GOTO)) {
    error_raise(ERR_WHAT,
     line_num);
    return;
   }
   lexer_next(lex);

   // Parse target line
   {
   long tgt = parse_expression(
    lex, rt, line_num);
   if (error_occurred()) return;

   // Register handler by event type
   if (evkw == KW_TIMER) {
    rt->timer_interval =
     (double)ev_arg;
    rt->on_timer_line =
     (int)tgt;
   } else if (evkw == KW_KEY) {
    if (ev_arg >= 1 &&
        ev_arg <= MAX_KEY_TRAPS) {
     rt->on_key_line[
         (int)ev_arg - 1] =
      (int)tgt;
    }
   } else if (evkw == KW_COM) {
    if (ev_arg >= 1 &&
        ev_arg <= MAX_COM_PORTS) {
     rt->on_com_line[
         (int)ev_arg - 1] =
      (int)tgt;
    }
   } else if (evkw == KW_PEN) {
    rt->on_pen_line =
     (int)tgt;
   } else if (evkw == KW_PLAY) {
    rt->on_play_line =
     (int)tgt;
   } else if (evkw == KW_STRIG) {
    if (ev_arg >= 0 &&
        ev_arg < MAX_STRIG_BUTTONS) {
     rt->on_strig_line[
         (int)ev_arg] =
      (int)tgt;
    }
   } else if (evkw == KW_TRAP) {
    rt->on_error_line =
     (int)tgt;
   } else if (evkw == KW_BREAK) {
    rt->on_break_line =
     (int)tgt;
    rt->break_event_state =
     EVT_ON;
   }
   }
  } else {
 // ON expr GOTO/GOSUB line-list.
 // Evaluate expr, then parse the
 // GOTO/GOSUB keyword, then collect
 // comma-separated line numbers.
 // Branch to the Nth target.
 long selector;
 int is_gosub = 0;
 int targets[20];
 int count = 0;

 selector = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 // Expect GOTO or GOSUB
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

 // Parse line number list
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

 // Branch if selector is valid
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
 // else: fall through to next line
 // (GW-BASIC behavior) 
 }
 }
 return;
}

 // pi_parse_else - Handle ELSE in control flow.
void pi_parse_else(Lexer *lex, RuntimeState *rt, int line_num)
{
 // ELSE encountered during execution:
 //
 // Single-line IF: ELSE has content on same
 // line - skip rest of line (true branch ran).
 //
 // Block IF with block_if_depth > 0: true
 // branch already ran - skip to END IF.
 //
 // Block IF with block_if_depth == 0: reached
 // from false path via scan - fall through to
 // execute the ELSE body.
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR) {
 // Block ELSE
 if (rt->block_if_depth > 0) {
 // True branch ran - skip to END IF
 pi_block_if_skip_to_end(rt, line_num);
 } else {
 // False path - enter ELSE body
 rt->block_if_depth++;
 }
 } else {
 // Single-line ELSE - skip rest
 lexer_skip_to_end(lex);
 }
 return;
}

 // pi_parse_elseif - Handle ELSEIF in control flow.
void pi_parse_elseif(Lexer *lex, RuntimeState *rt, int line_num)
{
 // ELSEIF encountered during execution.
 //
 // If block_if_depth > 0, we fell through from
 // a TRUE block - skip to END IF.
 //
 // If block_if_depth == 0, we were scanned to
 // from a FALSE path - evaluate the condition.
 if (rt->block_if_depth > 0) {
 // True branch already ran - skip to END IF
 pi_block_if_skip_to_end(rt, line_num);
 return;
 }
 {
 BValue elif_val;
 int elif_cond;

 elif_val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 elif_cond = (bval_to_int(&elif_val) != 0);

 // Consume optional THEN
 if (lexer_match_keyword(lex, KW_THEN))
 lexer_next(lex);

 if (elif_cond) {
 // TRUE: increment depth and let
 // execution continue into the body.
 rt->block_if_depth++;
 return;
 } else {
 // FALSE: scan forward for next
 // ELSEIF/ELSE/END IF.
 pi_block_if_scan(rt, line_num);
 return;
 }
 }
}

 // pi_parse_endif - Handle ENDIF in control flow.
void pi_parse_endif(Lexer *lex, RuntimeState *rt, int line_num)
{
 // ENDIF (single word) - end of block IF.
 // Decrement depth if inside a true block.
 if (rt->block_if_depth > 0)
 rt->block_if_depth--;
 return;
}


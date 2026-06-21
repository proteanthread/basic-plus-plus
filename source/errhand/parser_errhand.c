/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_errhand.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Error catching, exception handlers, and exception recovery (ON ERROR, RESUME, WHEN/USE).
 *
 * 2. WHAT TO EXPECT:
 *    Enforces execution rollback using nested error handling scopes.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Custom error codes, debug tracking messages.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Exception scope stack traversal logic, error recovery pathways.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If error handlers loop infinitely, check RESUME target line numbers and error registry states.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_errhand.c
 // ---
 //
 // Error handling & exception commands.
 //
 // ERROR, CAUSE, RESUME, WHEN, USE, RETRY,
 // CONTINUE, TRAP.
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

 // pi_parse_error - Handle ERROR command.
void pi_parse_error(Lexer *lex, RuntimeState *rt, int line_num)
{
 // ERROR n
 // Trigger a simulated error with code n.
 // Used for testing error handlers.
 {
 long errcode;
 errcode = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 // Map to internal error codes or use
 // ERR_HOW as generic runtime error 
 error_raise(ERR_HOW, line_num);
 (void)errcode;
 }
 return;
}

 // pi_parse_cause - Handle CAUSE command.
void pi_parse_cause(Lexer *lex, RuntimeState *rt, int line_num)
{
 // CAUSE EXCEPTION n
 // ECMA-116: Raise a user exception with code n.
 //
 // Sets ERR = n and ERL = current line, then
 // raises a runtime error. Compatible with
 // ON ERROR GOTO error trapping.
 //
 // The word EXCEPTION is consumed as a named
 // variable token (not a keyword).
 {
 long exc_code;
 // Consume EXCEPTION (optional for compat)
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length >= 3) {
 const char *e =
 lex->current.str_start;
 if ((e[0]=='E'||e[0]=='e') &&
 (e[1]=='X'||e[1]=='x') &&
 (e[2]=='C'||e[2]=='c')) {
 lexer_next(lex);
 }
 }
 exc_code = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 rt->last_err_code = (int)exc_code;
 rt->last_err_line = line_num;
 error_raise(ERR_HOW, line_num);
 }
 return;
}

 // pi_parse_resume - Handle RESUME command.
void pi_parse_resume(Lexer *lex, RuntimeState *rt, int line_num)
{
 // RESUME [NEXT | line]
 // Return from ON ERROR GOTO handler.
 // RESUME - retry the failed line.
 // RESUME NEXT - continue at next line after error.
 // RESUME n - jump to line n.
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_COLON) {
 // RESUME - re-execute the error line
 if (rt->last_err_line > 0) {
 vm_jump(rt, rt->last_err_line, line_num);
 }
 } else if (lex->current.type ==
 TOK_KEYWORD &&
 lex->current.value.keyword ==
 KW_NEXT) {
 lexer_next(lex);
 // RESUME NEXT: find the error line's program
 // index, then advance to the next line after it 
 if (rt->last_err_line > 0) {
 int idx;
 ProgramStore *pgm = rt->program;
 for (idx = 0; idx < pgm->count; idx++) {
 if (pgm->lines[idx].line_number ==
 rt->last_err_line) {
 rt->next_index = idx + 1;
 break;
 }
 }
 }
 } else {
 // RESUME linenum
 long target;
 target = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 vm_jump(rt, (int)target, line_num);
 }
 // Re-enable error handler after RESUME
 error_clear();
 return;
}

 // pi_parse_when - Handle WHEN command.
void pi_parse_when(Lexer *lex, RuntimeState *rt, int line_num)
{
 // WHEN EXCEPTION IN
 // ECMA-116: Start a structured exception block.
 //
 // Forward-scan to find USE and END WHEN, push
 // a FRAME_EXCEPTION, then continue executing
 // the protected block.
 //
 // The words EXCEPTION and IN are consumed as
 // named variable tokens (not keywords).
 {
 int use_idx, end_when_idx;
 StackFrame *f;

 // Consume EXCEPTION IN tokens
 if (lex->current.type == TOK_NAMED_VAR)
 lexer_next(lex); // EXCEPTION
 if (lex->current.type == TOK_NAMED_VAR)
 lexer_next(lex); // IN

 // Forward-scan for USE and END WHEN
 if (!pi_when_exception_scan(
 rt, rt->current_index,
 &use_idx, &end_when_idx,
 line_num)) {
 return; // error already raised
 }

 // Push exception frame
 if (rt->stack_top >= MAX_STACK_DEPTH) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 f = &rt->stack[rt->stack_top++];
 f->type = FRAME_EXCEPTION;
 f->data.exception.when_index =
 rt->current_index;
 f->data.exception.use_index = use_idx;
 f->data.exception.end_when_index =
 end_when_idx;
 f->data.exception.err_index = -1;
 }
 return;
}

 // pi_parse_use - Handle USE command.
void pi_parse_use(Lexer *lex, RuntimeState *rt, int line_num)
{
 // USE - ECMA-116 exception handler marker.
 //
 // If reached by fall-through (no error or
 // after CONTINUE), skip to END WHEN.
 // If exception frame exists, pop it first.
 // If frame was already popped (CONTINUE),
 // scan forward for END WHEN.
 {
 int i;
 // Find the innermost exception frame
 for (i = rt->stack_top - 1; i >= 0; i--) {
 if (rt->stack[i].type ==
 FRAME_EXCEPTION) {
 int end_idx = rt->stack[i]
 .data.exception
 .end_when_index;
 // Pop all frames down to
 // and including this one 
 rt->stack_top = i;
 // Jump to END WHEN
 rt->next_index = end_idx;
 return;
 }
 }
 // No frame (popped by CONTINUE).
 // Scan forward for END WHEN.
 {
 int idx = rt->current_index + 1;
 ProgramStore *pgm = rt->program;
 int depth = 0;
 while (idx < pgm->count) {
 Lexer cl;
 lexer_init(&cl,
 pgm->lines[idx].text);
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_KEYWORD) {
 if (cl.current.value.keyword
 == KW_WHEN) {
 depth++;
 } else if (
 cl.current.value.keyword
 == KW_END) {
 lexer_next(&cl);
 if (cl.current.type ==
 TOK_KEYWORD &&
 cl.current.value
 .keyword ==
 KW_WHEN) {
 if (depth > 0)
 depth--;
 else {
 rt->next_index =
 idx;
 return;
 }
 }
 }
 }
 idx++;
 }
 error_raise(ERR_WHAT, line_num);
 }
 }
 return;
}

 // pi_parse_retry - Handle RETRY command.
void pi_parse_retry(Lexer *lex, RuntimeState *rt, int line_num)
{
 // RETRY - ECMA-116: Re-enter protected block.
 //
 // Only valid inside a USE handler. Jumps back
 // to WHEN EXCEPTION IN + 1 (first line of the
 // protected block). The exception frame stays
 // on the stack.
 {
 int i;
 for (i = rt->stack_top - 1; i >= 0; i--) {
 if (rt->stack[i].type ==
 FRAME_EXCEPTION) {
 int when_idx = rt->stack[i]
 .data.exception.when_index;
 rt->next_index = when_idx + 1;
 return;
 }
 }
 error_raise(ERR_WHAT, line_num);
 }
 return;
}

 // pi_parse_continue - Handle CONTINUE command.
void pi_parse_continue(Lexer *lex, RuntimeState *rt, int line_num)
{
 // CONTINUE - ECMA-116: Resume after error.
 //
 // Only valid inside a USE handler. Jumps to the
 // line after the one that caused the exception.
 // Pops the exception frame.
 {
 int i;
 for (i = rt->stack_top - 1; i >= 0; i--) {
 if (rt->stack[i].type ==
 FRAME_EXCEPTION) {
 int err_idx = rt->stack[i]
 .data.exception.err_index;
 if (err_idx < 0) {
 error_raise(ERR_HOW,
 line_num);
 return;
 }
 // Pop exception frame
 rt->stack_top = i;
 // Resume after error line
 rt->next_index = err_idx + 1;
 return;
 }
 }
 error_raise(ERR_WHAT, line_num);
 }
 return;
}

 // pi_parse_trap - Handle TRAP command.
 //
 // Three syntax forms:
 //
 // 1. TRAP n
 //    Atari BASIC style. Sets the error handler
 //    to line n. Equivalent to ON ERROR GOTO n.
 //    TRAP 0 disables the handler.
 //
 // 2. TRAP event, source, destination
 //    Explicit event routing. The event identifier
 //    names the event class, source is the event
 //    source (e.g. device number), and destination
 //    is the handler line number.
 //
 // 3. ON TRAP (event) GOSUB / GOTO line
 //    Handled by pi_parse_on via the ON dispatcher.
void pi_parse_trap(Lexer *lex, RuntimeState *rt, int line_num)
{
 long val;

 // If empty (bare TRAP), show current trap status
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR) {
 if (rt->on_error_line > 0) {
 printf("TRAP active -> line %d\n",
 rt->on_error_line);
 } else {
 printf("TRAP inactive\n");
 }
 return;
 }

 // Parse the first argument
 val = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 // Check for comma: TRAP event, source, dest
 if (lex->current.type == TOK_COMMA) {
 long dest;
 lexer_next(lex); // consume first comma

 // Source (consumed but not used - no
  // hardware events to bind) 
 (void)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume second comma

 // Destination line number
 dest = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 rt->on_error_line = (int)dest;
 return;
 }

  // Simple form: TRAP n
  // n = 0 disables, n > 0 sets handler line.
 rt->on_error_line = (int)val;
 return;
}

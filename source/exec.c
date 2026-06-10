/*
 * ---
 * BASIC++ Interpreter - exec.c
 * ---
 *
 * Program execution loop implementation.
 *
 * DESIGN RATIONALE:
 * The execution loop iterates through program lines in stored
 * order, tokenizes each line on-the-fly, and delegates to the
 * parser for execution. Flow control (GOTO, GOSUB, RETURN)
 * works by modifying rt->next_index, which the loop checks
 * after each line.
 *
 * VM Formalization
 * The execution loop now uses the formal VM state machine
 * (VMState) instead of the ad-hoc rt->running flag. State
 * transitions go through vm_set_state()/vm_get_state().
 * The opcode is resolved via vm_resolve_opcode() for trace
 * output, and the control flow primitives (vm_jump, vm_call,
 * vm_return_sub) are available to the parser.
 *
 * ---
 */

#include <stdio.h>
#include "exec.h"
#include "parser.h"
#include "lexer.h"
#include "runtime.h"
#include "errors.h"
#include "vdev.h"
#include "vm.h"

/*
 * exec_run - Main program execution loop.
 *
 * Execution flow:
 * 1. Reset runtime state (variables, stack, position).
 * 2. Collect DATA values from all program lines.
 * 3. Set VM state to VM_RUNNING.
 * 4. Loop:
 * a. Get line at current_index.
 * b. Tokenize line.
 * c. Skip line number token.
 * d. Resolve opcode via VM dispatch table (for trace).
 * e. Execute via parser_execute_line().
 * f. Check for errors -> transition to VM_ERROR.
 * g. Check next_index -> jump or advance.
 * h. Check bounds -> halt if past last line.
 * 5. On exit: set VM state to VM_STOPPED.
 */
/*
 * exec_run_from - Internal execution loop starting from a given index.
 *
 * Shared by exec_run (fresh start) and exec_cont (resume).
 */
static void exec_run_from(RuntimeState *rt, int start_index)
{
 Lexer lex;
 int skip_first_break = 1; /* Skip breakpoint on first line (CONT resume) */

 vm_set_state(rt, VM_RUNNING);
 rt->current_index = start_index;

 while (vm_get_state(rt) == VM_RUNNING &&
 rt->current_index < rt->program->count &&
 !error_occurred()) {
 ProgramLine *line;
 int line_num;

 /* Get the current line */
 line = &rt->program->lines[rt->current_index];
 line_num = line->line_number;

 /*
 * Breakpoint / single-step check.
 * If we hit a breakpoint or single_step is on,
 * pause and return to REPL so user can inspect.
 * Skip the check on the first line after CONT.
 */
 if (!skip_first_break) {
 if (rt->single_step ||
 (rt->breakpoint_count > 0 &&
 runtime_is_breakpoint(rt, line_num))) {
 printf("[BREAK at line %d]\n", line_num);
 vm_set_state(rt, VM_PAUSED);
 rt->resume_index = rt->current_index;
 return;
 }
 }
 skip_first_break = 0;

 /*
 * Trace output (TRON/TROFF).
 */
 if (rt->trace_on) {
 vdev_printf(rt->dev_con, "[%d]", line_num);
 }

 /* Initialize lexer on the line text */
 lexer_init(&lex, line->text);

 /*
 * Skip the line number.
 */
 if (lex.current.type == TOK_NUMBER) {
 lexer_next(&lex);
 }

 /* Reset next_index to -1 (no jump pending) */
 rt->next_index = -1;

 /*
 * Suppress error messages when ON ERROR GOTO
 * or WHEN EXCEPTION is active.
 */
 {
 int suppress = 0;
 if (rt->on_error_line > 0)
 suppress = 1;
 if (!suppress) {
 /* Check stack for FRAME_EXCEPTION */
 int ei;
 for (ei = rt->stack_top - 1;
 ei >= 0; ei--) {
 if (rt->stack[ei].type ==
 FRAME_EXCEPTION) {
 suppress = 1;
 break;
 }
 }
 }
 error_set_suppress(suppress);
 }

 /* Parse and execute the line */
 parser_execute_line(&lex, rt, line_num);

 /* Restore error output */
 error_set_suppress(0);

 /*
 * WHEN EXCEPTION handler (ECMA-116).
 *
 * Block-scoped: checked FIRST, before ON ERROR GOTO.
 * If an error occurred and there's a FRAME_EXCEPTION
 * on the stack, redirect to the USE handler.
 */
 if (error_occurred()) {
 int ei;
 for (ei = rt->stack_top - 1; ei >= 0; ei--) {
 if (rt->stack[ei].type == FRAME_EXCEPTION) {
 int use_idx = rt->stack[ei]
 .data.exception.use_index;

 /* Save error info for ERR/ERL */
 if (rt->last_err_line != line_num) {
 rt->last_err_code =
 (int)error_get();
 rt->last_err_line = line_num;
 }

 /* Record err_index for CONTINUE */
 rt->stack[ei].data.exception
 .err_index = rt->current_index;

 error_clear();
 /* Jump to first line of USE handler */
 rt->next_index = use_idx + 1;
 break;
 }
 }
 }

 /*
 * ON ERROR GOTO handler.
 * (Only reached if no WHEN EXCEPTION frame handled it.)
 */
 if (error_occurred() && rt->on_error_line > 0) {
 int target_line = rt->on_error_line;

 /* Save error info for ERL/ERR.
 * If CAUSE EXCEPTION already set last_err_code
 * and last_err_line for this line, preserve
 * the user's exception code.
 */
 if (rt->last_err_line != line_num) {
 rt->last_err_code = (int)error_get();
 rt->last_err_line = line_num;
 }

 rt->on_error_line = 0;
 error_clear();

 vm_jump(rt, target_line, line_num);

 if (rt->next_index < 0) {
 vm_set_state(rt, VM_ERROR);
 break;
 }
 }

 /* Check for unhandled error */
 if (error_occurred()) {
 vm_set_state(rt, VM_ERROR);
 break;
 }

 /*
 * Check if STOP paused execution.
 * If VM is PAUSED (set by STOP handler), save
 * resume position and return to REPL.
 */
 if (vm_get_state(rt) == VM_PAUSED) {
 /* Determine resume point */
 if (rt->next_index >= 0) {
 rt->resume_index = rt->next_index;
 } else {
 rt->resume_index = rt->current_index + 1;
 }
 return; /* Don't set STOPPED */
 }

 /* Determine next line */
 if (rt->next_index >= 0) {
 rt->current_index = rt->next_index;
 } else if (vm_get_state(rt) == VM_RUNNING) {
 rt->current_index++;
 }
 }

 vm_set_state(rt, VM_STOPPED);
}

/*
 * exec_run - Start program execution from the beginning.
 */
void exec_run(RuntimeState *rt)
{
 /* Reset execution state for fresh run */
 runtime_reset(rt);

 /* Collect DATA values before execution begins */
 runtime_collect_data(rt);

 /* Collect line labels for GOTO/GOSUB label resolution */
 runtime_collect_labels(rt);

 /*
 * Pre-scan for SUB/FUNCTION definitions.
 *
 * QBasic-style: resolve all SUB/FUNCTION bodies
 * before the first line executes. This makes
 * DECLARE EXTERNAL and forward references work
 * correctly. Execute each SUB/FUNCTION line which
 * registers the body and skips to END SUB/FUNCTION.
 */
 {
 int idx;
 ProgramStore *pgm = rt->program;
 for (idx = 0; idx < pgm->count; idx++) {
 Lexer cl;
 const char *text = pgm->lines[idx].text;
 int ln = pgm->lines[idx].line_number;
 lexer_init(&cl, text);
 if (cl.current.type == TOK_NUMBER)
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD &&
 (cl.current.value.keyword == KW_SUB ||
 cl.current.value.keyword == KW_FUNCTION)) {
 /* Execute this line to register the
 * SUB/FUNCTION definition */
 rt->current_index = idx;
 rt->next_index = -1;
 parser_execute_line(&cl, rt, ln);
 if (error_occurred()) {
 vm_set_state(rt, VM_ERROR);
 return;
 }
 /* SUB handler skips to END SUB; advance
 * past it */
 if (rt->next_index > idx)
 idx = rt->next_index - 1;
 }
 }
 /* Reset execution state for actual run */
 rt->current_index = 0;
 rt->next_index = -1;
 }

 exec_run_from(rt, 0);
}

/*
 * exec_cont - Continue execution from paused state.
 *
 * Resumes from the saved resume_index without resetting state.
 * Returns 0 on success, -1 if not paused.
 */
int exec_cont(RuntimeState *rt)
{
 if (vm_get_state(rt) != VM_PAUSED) {
 printf("Not paused.\n");
 return -1;
 }

 if (rt->resume_index < 0 ||
 rt->resume_index >= rt->program->count) {
 printf("Cannot continue - no resume point.\n");
 vm_set_state(rt, VM_STOPPED);
 return -1;
 }

 exec_run_from(rt, rt->resume_index);
 return 0;
}

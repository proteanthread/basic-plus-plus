/*
 * =====================================================================
 * BASIC++ Interpreter - exec.h
 * =====================================================================
 *
 * Executor interface - the runtime execution loop.
 *
 * PURPOSE:
 *   Provides the main program execution loop (exec_run) which
 *   iterates through stored program lines, tokenizes each line,
 *   and dispatches to the parser for execution.
 *
 * WHY THIS IS SEPARATE FROM parser.c:
 *   The parser handles single-line parsing and expression evaluation.
 *   The executor handles the program-level execution flow: line
 *   sequencing, GOTO target resolution, and the RUN loop. This
 *   separation keeps each module focused and testable.
 *
 * HOW IT WORKS:
 *   exec_run() resets the runtime state, then enters a loop:
 *     1. Get the current line from the program store.
 *     2. Initialize the lexer on the line's text.
 *     3. Skip the line number tokens.
 *     4. Call parser_execute_line() to parse and execute.
 *     5. Check if GOTO/GOSUB changed next_index.
 *     6. If so, jump to the new target; otherwise advance.
 *     7. If current_index exceeds program size, stop.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#ifndef BASICPP_EXEC_H
#define BASICPP_EXEC_H

#include "runtime.h"

/*
 * exec_run - Execute the stored program from the beginning.
 *
 * Resets all variables and the stack, then executes lines
 * sequentially from the first stored line. Execution stops on:
 *   - END or STOP statement
 *   - Running past the last line
 *   - Runtime error
 *
 * This is called by the RUN command.
 */
void exec_run(RuntimeState *rt);

/*
 * exec_cont - Continue execution from paused state.
 *
 * Resumes from the saved resume_index without resetting
 * runtime state. Returns 0 on success, -1 if not paused.
 */
int exec_cont(RuntimeState *rt);

#endif /* BASICPP_EXEC_H */

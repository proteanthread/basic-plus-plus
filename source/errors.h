/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: errors.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - errors.h
 // ---
 //
 // Error handling subsystem interface.
 //
 // PURPOSE:
 // Provides the classic Tiny BASIC error model:
 // WHAT? - syntax error (malformed statement or expression)
 // HOW? - runtime error (illegal operation, bad line number, etc.)
 // SORRY. - resource error (out of memory, stack overflow)
 //
 // Errors are non-fatal: they print a message, set an error flag,
 // and cause the current statement to abort. The REPL loop checks
 // the flag and returns to the prompt. No longjmp, no exceptions,
 // no stack unwinding.
 //
 // HOW IT WORKS:
 // 1. Any module calls error_raise(code, line_num) on error.
 // 2. error_raise() prints the message and sets the global flag.
 // 3. The caller checks error_occurred() and returns early.
 // 4. The REPL loop (or runtime loop) detects the error and
 // returns to the prompt.
 // 5. error_clear() resets the flag for the next input.
 //
 // HOW TO EXTEND:
 // To add new error codes (e.g., for file I/O errors in future
 // phases), add entries to the ErrorCode enum and update
 // error_message() in errors.c. The three-message model can
 // be extended while maintaining backward compatibility.
 //
 // ---

#ifndef BASICPP_ERRORS_H
#define BASICPP_ERRORS_H

// --- Error Codes ---
 // ERR_NONE: No error (default state).
 // ERR_WHAT: Syntax error - unexpected token, malformed statement.
 // ERR_HOW: Runtime error - GOTO to nonexistent line, division
 // by zero, stack underflow, etc.
 // ERR_SORRY: Resource error - out of memory, stack overflow,
 // too many program lines.
typedef enum ErrorCode {
 ERR_NONE = 0,
 ERR_WHAT,
 ERR_HOW,
 ERR_SORRY
} ErrorCode;

 // error_raise - Report an error.
 //
 // Prints the appropriate error message to stdout. If line_num > 0,
 // the line number is included in the output (e.g., "WHAT? AT LINE 10").
 // Sets the global error flag so callers can detect and abort.
 //
 // Parameters:
 // code - the error category (ERR_WHAT, ERR_HOW, ERR_SORRY)
 // line_num - the BASIC line number where the error occurred,
 // or 0 for immediate mode errors.
extern double g_current_executing_line;

void error_raise(ErrorCode code, double line_num);

 // error_clear - Reset the error state.
 //
 // Called by the REPL loop before processing each new input line.
 // Clears the error flag so previous errors don't affect new input.
void error_clear(void);

 // error_occurred - Check if an error is active.
 //
 // Returns non-zero if an error has been raised since the last
 // error_clear(). Used by parser, executor, and runtime loop to
 // detect errors and abort the current operation.
int error_occurred(void);

 // error_get - Return the current error code.
 //
 // Returns ERR_NONE if no error is active.
ErrorCode error_get(void);

 // error_message - Return the display string for an error code.
 //
 // Returns a pointer to a static string: "WHAT?", "HOW?", or
 // "SORRY." Returns "" for ERR_NONE.
const char *error_message(ErrorCode code);

 // error_set_suppress - Control error message output.
 //
 // When suppress is non-zero, error_raise() sets the error flag
 // but does NOT print the error message. Used by ON ERROR GOTO
 // to silently catch errors.
void error_set_suppress(int suppress);

 // error_get_suppress - Query error suppress state.
 //
 // Returns non-zero if error output is currently suppressed.
 // Used to save/restore suppress state across pre-scan phases.
int error_get_suppress(void);

 // error_set_beep - Control error beep.
 //
 // When on=1 (default), errors emit an audible beep (BEL).
 // When on=0, errors are silent. Controlled by BEEP ON/OFF.
void error_set_beep(int on);

 // error_get_beep - Query error beep state.
 //
 // Returns 1 if error beep is enabled, 0 if disabled.
int error_get_beep(void);

#endif // BASICPP_ERRORS_H

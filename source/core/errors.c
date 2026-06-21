/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: errors.c
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
// BASIC++ Interpreter - errors.c
// ---
//
// Implementation of the classic Tiny BASIC error handling model.
//
// PURPOSE:
//   Provides a simple, global error flag system for reporting
//   runtime and syntax errors. All error messages print to stdout
//   (matching classic BASIC convention) and set a flag that the
//   REPL loop checks to decide whether to print "READY".
//
// HOW IT WORKS:
//   1. A statement encounters an error (bad syntax, overflow, etc.)
//   2. It calls error_raise(ERR_xxx, line_num).
//   3. error_raise() sets the global error flag and prints the message.
//   4. The caller returns immediately (no further execution).
//   5. The REPL loop (main.c) calls error_occurred() to check.
//   6. If true, it prints READY and returns to the prompt.
//   7. error_clear() resets the flag for the next input.
//
//   With ON ERROR GOTO active:
//   - error_set_suppress(1) is called by exec.c.
//   - error_raise() sets the flag but does NOT print the message.
//   - exec.c detects the error and jumps to the ON ERROR target.
//   - The BASIC program handles the error via ERR/ERL variables.
//
// HOW TO EXTEND / CUSTOMIZE:
//   Adding a new error code:
//   1. Add ERR_YOURCODE to the ErrorCode enum in errors.h.
//   2. Add a case in error_message() below with the display text.
//   3. In error_registry.c, add a GW-BASIC-compatible numeric code
//      if needed for ERR variable compatibility.
//   4. Call error_raise(ERR_YOURCODE, line_num) where appropriate.
//
//   Changing error message text:
//   - Edit the error_message() switch below.
//   - Keep messages short (classic BASIC convention).
//   - The three original messages ("WHAT?", "HOW?", "SORRY.")
//     cover syntax errors, logic errors, and resource exhaustion.
//
//   Adding a new error severity level:
//   - Currently all errors are non-fatal (abort statement only).
//   - To add fatal errors, add a severity field to ErrorCode or
//     create a separate error_fatal() function that calls exit().
//
// FINE-TUNING:
//   - beep_on_error: Controls whether the BEL character (\a) is
//     emitted with error messages. Default: ON. Toggle via the
//     BEEP ON / BEEP OFF BASIC statements.
//
//   - suppress_output: Controls whether error messages are printed.
//     Used by ON ERROR GOTO to silently capture errors. Managed
//     by exec.c during program execution.
//
// TROUBLESHOOTING:
//   - "WHAT?" (ERR_WHAT):
//     Syntax error. The parser encountered unexpected input.
//     Check the line for typos, missing quotes, or incorrect
//     keyword spelling. In strict dialect mode, this may also
//     mean a keyword from a different dialect was used.
//
//   - "HOW?" (ERR_HOW):
//     Logic or range error. Common causes:
//       * GOTO to a nonexistent line number
//       * Division by zero
//       * Array index out of bounds
//       * Line number > LINE_NUMBER_MAX (32767)
//       * RETURN without GOSUB
//       * NEXT without matching FOR
//
//   - "SORRY." (ERR_SORRY):
//     Resource exhaustion. Common causes:
//       * Program store full (MAX_PROGRAM_LINES reached)
//       * Variable pool full (MAX_NAMED_VARS reached)
//       * Stack overflow (MAX_STACK_DEPTH reached)
//       * Memory pool exhausted (increase sizes in config.h)
//       * Array element pool full (MAX_ARRAY_ELEMENTS reached)
//
// PERFORMANCE:
//   - error_raise() and error_clear() are O(1).
//   - error_occurred() is O(1) -- a single comparison.
//   - Error handling adds zero overhead when no error occurs.
//
// MINIMALIZATION:
//   This file is CORE tier -- required for any BASIC++ build.
//   It is already minimal (~160 lines). For embedded builds,
//   you could remove the BEL beep and reduce message strings
//   to single characters to save ROM space.
//
// DEPENDENCIES:
//   - errors.h  (ErrorCode enum, function declarations)
//   - txn.h     (txn_on_error for ATOMIC block auto-rollback)
//
// C17 COMPLIANCE:
//   - Uses only C17 standard library (stdio.h for printf).
//   - No platform-specific code.
//   - Compiles cleanly on gcc -std=c17 -Wall -Wextra and
//     MSVC /std:c17 /W4.
//
// ---

#include <stdio.h>
#include "errors.h"
#include "txn.h"

// -----------------------------------------------------------------
// Module State -- Global Error Flag
// -----------------------------------------------------------------
// The error system uses a single global flag. This works because:
//   1. The interpreter is single-threaded.
//   2. Only one statement executes at a time.
//   3. Error recovery always means "return to prompt."
//
// If you ever make the interpreter multi-threaded, this state
// would need to be moved into a per-thread or per-context struct.
// -----------------------------------------------------------------

// current_error: The active error code, or ERR_NONE if no error.
// Set by error_raise(), cleared by error_clear().
static ErrorCode current_error = ERR_NONE;

// suppress_output: When non-zero, error_raise() sets the flag
// but does NOT print the message. Used by ON ERROR GOTO so that
// the BASIC program can handle the error itself via ERR/ERL.
static int suppress_output = 0;

// beep_on_error: When non-zero, error_raise() emits BEL (\a)
// along with the message. Default: ON. Controlled by the
// BEEP ON / BEEP OFF BASIC statements.
static int beep_on_error = 1;

// -----------------------------------------------------------------
// Error Messages
// -----------------------------------------------------------------

// error_message - Return the human-readable string for an error code.
//
// These strings match the original Li-Chen Wang Palo Alto Tiny BASIC
// error messages exactly. The brevity is intentional: classic BASIC
// interpreters used short messages to save memory on systems with
// as little as 2 KB of RAM.
//
//   ERR_WHAT  -> "WHAT?"   Syntax error (unexpected input)
//   ERR_HOW   -> "HOW?"    Logic/range error (impossible operation)
//   ERR_SORRY -> "SORRY."  Resource exhaustion (out of memory/slots)
//
// How to customize:
//   To add a new error code, add a case here. Keep messages short.
//   For longer explanations, add them to the HELP system (help.c)
//   or to the error registry (error_registry.c) for numeric codes.
//
const char *error_message(ErrorCode code)
{
    switch (code) {
    case ERR_WHAT:  return "WHAT?";
    case ERR_HOW:   return "HOW?";
    case ERR_SORRY: return "SORRY.";
    default:        return "";
    }
}

// -----------------------------------------------------------------
// Error Reporting API
// -----------------------------------------------------------------

// error_raise - Report an error and set the error flag.
//
// Prints the error message to stdout (not stderr -- matching
// original BASIC behavior where all output goes to the console).
// If line_num > 0, appends " AT LINE <n>" for context.
//
// Side effects:
//   - Sets current_error to the given code.
//   - Calls txn_on_error() to auto-rollback any active ATOMIC block.
//   - Prints the error message (unless suppress_output is set).
//   - Emits BEL character if beep_on_error is set.
//
// After calling this, all subsequent checks of error_occurred()
// return true until error_clear() is called.
//
// Parameters:
//   code     - error code (ERR_WHAT, ERR_HOW, or ERR_SORRY)
//   line_num - line number for context (0 = immediate mode)
//
void error_raise(ErrorCode code, int line_num)
{
    if (code == ERR_NONE) {
        return;
    }

    current_error = code;

    // Auto-rollback ATOMIC blocks on error.
    // If a transaction is in progress, this rolls it back
    // before the error message is printed.
    txn_on_error(line_num);

    // If ON ERROR GOTO is active, suppress the message.
    // The error flag is still set so exec.c can detect it
    // and redirect execution to the error handler.
    if (suppress_output) {
        return;
    }

    // Audible beep on error (BEL character).
    // Disabled by BEEP OFF statement.
    if (beep_on_error) {
        printf("\a");
    }

    // Print the error message with optional line number.
    if (line_num > 0) {
        printf("%s AT LINE %d\n", error_message(code), line_num);
    } else {
        printf("%s\n", error_message(code));
    }
}

// error_clear - Reset error state for new input.
//
// Called at the start of each REPL cycle (main.c) and before
// each statement during program execution (exec.c). After this
// call, error_occurred() returns 0.
//
void error_clear(void)
{
    current_error = ERR_NONE;
}

// error_occurred - Check if an error is currently active.
//
// Returns:
//   1 if an error has been raised since the last error_clear()
//   0 otherwise
//
int error_occurred(void)
{
    return (current_error != ERR_NONE) ? 1 : 0;
}

// error_get - Return the current error code.
//
// Returns ERR_NONE if no error is active, or the specific
// ErrorCode that was passed to the most recent error_raise().
//
// Used by:
//   - exec.c to determine the error code for ERR variable
//   - error_registry.c to map to GW-BASIC numeric codes
//
ErrorCode error_get(void)
{
    return current_error;
}

// -----------------------------------------------------------------
// Error Suppression (for ON ERROR GOTO)
// -----------------------------------------------------------------

// error_set_suppress - Control error message suppression.
//
// When suppress=1, error_raise() will set the error flag but
// NOT print any message. This is used by exec.c when ON ERROR GOTO
// is active: the BASIC program handles the error itself.
//
// When suppress=0 (default), errors print normally.
//
// How it interacts with ON ERROR GOTO:
//   1. Program executes ON ERROR GOTO 1000.
//   2. exec.c calls error_set_suppress(1).
//   3. An error occurs -> flag is set, no message printed.
//   4. exec.c detects the flag, sets ERR and ERL variables,
//      and jumps to line 1000.
//   5. The BASIC program examines ERR and handles the error.
//   6. RESUME clears the error and continues.
//
void error_set_suppress(int suppress)
{
    suppress_output = suppress;
}

// error_get_suppress - Query whether error suppression is active.
//
// Returns:
//   1 if error messages are being suppressed (ON ERROR GOTO active)
//   0 if errors print normally
//
int error_get_suppress(void)
{
    return suppress_output;
}

// -----------------------------------------------------------------
// Error Beep Control
// -----------------------------------------------------------------

// error_set_beep - Control the audible error beep.
//
// When on=1 (default), errors emit BEL character (\a).
// When on=0, errors are silent.
//
// Controlled by the BASIC statements:
//   BEEP ON   -> enable error beep
//   BEEP OFF  -> disable error beep
//
void error_set_beep(int on)
{
    beep_on_error = on ? 1 : 0;
}

// error_get_beep - Query whether error beep is enabled.
//
// Returns:
//   1 if error beep is on
//   0 if error beep is off
//
int error_get_beep(void)
{
    return beep_on_error;
}

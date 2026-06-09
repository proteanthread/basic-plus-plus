/*
 * =====================================================================
 * BASIC++ Interpreter - errors.c
 * =====================================================================
 *
 * Implementation of the classic Tiny BASIC error handling model.
 *
 * DESIGN RATIONALE:
 *   The error system is intentionally simple. It uses a single global
 *   error flag rather than exceptions, longjmp, or error return codes
 *   threaded through every function. This matches the original Tiny
 *   BASIC design where errors immediately print a message and abort
 *   the current statement.
 *
 *   The global flag approach works because:
 *   1. The interpreter is single-threaded.
 *   2. Only one statement executes at a time.
 *   3. Error recovery always means "return to prompt."
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include <stdio.h>
#include "errors.h"

/* =====================================================================
 * Module State
 * =====================================================================
 * current_error: The currently active error code, or ERR_NONE if
 *   no error is pending. This is set by error_raise() and cleared
 *   by error_clear().
 */
static ErrorCode current_error = ERR_NONE;

/*
 * suppress_output: When non-zero, error_raise() sets the error flag
 * but does NOT print the message. Used by ON ERROR GOTO.
 */
static int suppress_output = 0;

/*
 * beep_on_error: When non-zero, error_raise() emits BEL
 * character along with the error message. Default is ON.
 * Controlled by BEEP ON / BEEP OFF statements.
 */
static int beep_on_error = 1;

/*
 * error_message - Return the display string for an error code.
 *
 * These strings match the original Li-Chen Wang Palo Alto Tiny BASIC
 * error messages exactly. The simplicity is intentional: classic
 * BASIC interpreters used short messages to save memory.
 */
const char *error_message(ErrorCode code)
{
    switch (code) {
        case ERR_WHAT:  return "WHAT?";
        case ERR_HOW:   return "HOW?";
        case ERR_SORRY: return "SORRY.";
        default:        return "";
    }
}

/*
 * error_raise - Report an error and set the error flag.
 *
 * Prints the error message to stdout (not stderr - matching original
 * BASIC behavior where all output goes to the terminal/console).
 * If line_num > 0, appends the line number for context.
 *
 * After calling this, all subsequent checks of error_occurred() will
 * return true until error_clear() is called.
 */
void error_raise(ErrorCode code, int line_num)
{
    if (code == ERR_NONE) {
        return;
    }

    current_error = code;

    /* If ON ERROR GOTO is active, suppress the message */
    if (suppress_output) {
        return;
    }

    if (beep_on_error) {
        printf("\a");  /* BEL - audible error beep */
    }

    if (line_num > 0) {
        printf("%s AT LINE %d\n", error_message(code), line_num);
    } else {
        printf("%s\n", error_message(code));
    }
}

/*
 * error_clear - Reset error state for new input.
 */
void error_clear(void)
{
    current_error = ERR_NONE;
}

/*
 * error_occurred - Check if an error is active.
 *
 * Returns 1 if an error has been raised, 0 otherwise.
 */
int error_occurred(void)
{
    return (current_error != ERR_NONE) ? 1 : 0;
}

/*
 * error_get - Return the current error code.
 */
ErrorCode error_get(void)
{
    return current_error;
}

/*
 * error_set_suppress - Control error message suppression.
 *
 * Called by the execution loop when ON ERROR GOTO is active.
 */
void error_set_suppress(int suppress)
{
    suppress_output = suppress;
}

/*
 * error_set_beep - Control error beep.
 *
 * When on=1, errors emit BEL character (audible beep).
 * When on=0, errors are silent.
 */
void error_set_beep(int on)
{
    beep_on_error = on ? 1 : 0;
}

/*
 * error_get_beep - Query error beep state.
 */
int error_get_beep(void)
{
    return beep_on_error;
}

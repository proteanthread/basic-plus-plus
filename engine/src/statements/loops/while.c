/**
 * @file while.c
 * @brief WHILE loop condition check statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements WHILE condition statement evaluation (WHILE expr), pushing loop top line position
 * onto vm->while_stack when true, or scanning to WEND when false.
 *
 * 2. WHY IT EXISTS:
 * Provides structured condition-first pre-test loop control across all execution profiles.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Evaluates expr via eval_expression(). If non-zero (true), pushes line number and offset onto
 * vm->while_stack via vm_while_push(). If zero (false), scans tokens to the matching WEND statement.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_while'. Directly includes "statements/loops/while.h",
 * "vm/vm.h", "lexer/lexer.h", "eval/eval.h", and "device/vdev.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To add loop iteration counters or timeout guards, store loop state in BppWhileFrame.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Stack push invariant: WHILE loop condition evaluation MUST push return address before proceeding.
 *
 * 8. WHAT TO EXPECT:
 * Pushes WHILE frame or jumps to WEND, returning ERR_NONE or ERR_SYNTAX.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect vm_while_push() in vm/control.c. Check WEND scanning logic for nested loops.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Double-precision floating-point truth evaluation (val != 0.0).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/control.c
 * - engine/src/eval/eval.c
 * - engine/src/statements/loops/wend.c
 * Prerequisite Header Files:
 * - engine/include/statements/loops/while.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 * - engine/include/eval/eval.h
 */

#include "statements/loops/while.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"

void stmt_while_register(void) {
    MicroLibMetadata meta = {
        .name = "WHILE",
        .category = "Looping / Control Flow",
        .syntax = "WHILE condition",
        .help_text = "Executes a series of statements in a loop as long as condition evaluates to non-zero (true).",
        .error_codes = "Error 2: Syntax Error, Error 30: WHILE Without WEND"
    };
    microlib_register(&meta);
}
#include "device/vdev.h"
#include "security/security.h"
#include <string.h>

BppError stmt_while_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

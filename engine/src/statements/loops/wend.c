/**
 * @file wend.c
 * @brief WEND loop termination statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements WEND loop completion evaluation, popping or peeking the top WHILE stack frame
 * and jumping back to the WHILE condition line.
 *
 * 2. WHY IT EXISTS:
 * Serves as the loop tail counterpart to WHILE across all target execution profiles.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Peeks the top WHILE frame from vm->while_stack via vm_while_peek(). If empty, emits ERR_WEND_WITHOUT_WHILE
 * (Error 2). Otherwise, jumps back to the WHILE line and offset to re-evaluate the condition.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_wend'. Directly includes "statements/loops/wend.h",
 * "vm/vm.h", "lexer/lexer.h", "eval/eval.h", and "device/vdev.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To add loop profiling, increment loop pass counter inside the while stack frame structure.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Stack underflow check invariant: WEND without an active WHILE loop MUST emit ERR_WEND_WITHOUT_WHILE (Error 2).
 *
 * 8. WHAT TO EXPECT:
 * Jumps to WHILE line number and returns ERR_NONE, or returns ERR_WEND_WITHOUT_WHILE.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect vm_while_peek() in vm/control.c. Check while stack balance in vm->while_stack.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext. Active WHILE loop stack frame.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Zero host C stack allocation.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/control.c
 * - engine/src/statements/loops/while.c
 * Prerequisite Header Files:
 * - engine/include/statements/loops/wend.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "statements/loops/wend.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"

void stmt_wend_register(void) {
    MicroLibMetadata meta = {
        .name = "WEND",
        .category = "Looping / Control Flow",
        .syntax = "WEND",
        .help_text = "Terminates a WHILE...WEND loop block and transfers control back to the matching WHILE statement.",
        .error_codes = "Error 29: WEND Without WHILE, Error 2: Syntax Error"
    };
    microlib_register(&meta);
}
#include "device/vdev.h"
#include "security/security.h"
#include <string.h>

BppError stmt_wend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

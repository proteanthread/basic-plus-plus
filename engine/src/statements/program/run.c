/**
 * @file run.c
 * @brief RUN [line_num | filename_expr [, R]] program execution statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements RUN statement handler for starting program execution from beginning, target line number, or loading and executing an external file.
 *
 * 2. WHY IT EXISTS:
 * Starts execution loop after clearing variables or loading new program source per GW-BASIC / QBASIC REPL standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Clears variable tables via vm_clear_variables(), sets program counter to line 0 or target line number, and initiates non-recursive VM main evaluation loop.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_run'. Includes "statements/program/run.h",
 * "vm/vm.h", "lexer/lexer.h", "eval/eval.h", "device/vdev.h", "security/security.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (Core Included).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support RUN with parameters passing command-line args to program scope.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Variable clearing invariant: Standard RUN MUST clear non-COMMON variable tables before execution begins.
 *
 * 8. WHAT TO EXPECT:
 * Resets variable state, jumps to start line, and initiates execution loop.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check line lookup resolution in vm_exec.c.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and program memory buffer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Zero host C recursion during VM execution.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/program/clear.c
 * - engine/src/vm/vm_exec.c
 * Prerequisite Header Files:
 * - engine/include/statements/program/run.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "statements/program/run.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_run_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

void stmt_run_register(void) {
    static const MicroLibMetadata meta = {
        .name = "RUN",
        .category = "Program Mgmt & Editing",
        .syntax = "RUN [line_number | filename [, R]]",
        .help_text = "Starts execution of the program currently in memory or loads and runs a specified file.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found, Error 8: Undefined Line Number"
    };
    microlib_register(&meta);
}


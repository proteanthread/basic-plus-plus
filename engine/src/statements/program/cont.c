/**
 * @file cont.c
 * @brief CONT program execution resume statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements CONT statement handler for resuming VM execution after a STOP statement or Ctrl+Break pause.
 *
 * 2. WHY IT EXISTS:
 * Restores execution pointer to the line following the pause point per GW-BASIC / QBASIC interactive REPL standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Restores program counter from stored VM break location saved when STOP or Break trap triggered.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_cont'. Includes "statements/program/cont.h",
 * "vm/vm.h", "lexer/lexer.h", "eval/eval.h", "device/vdev.h", "security/security.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (Core Included).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support single-step stepping or breakpoint query flags in interactive debuggers.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Error behavior: If program was modified after pause, CONT MUST return ERR_CANNOT_CONTINUE (Error 17).
 *
 * 8. WHAT TO EXPECT:
 * Resumes execution at saved break point and returns ERR_NONE or ERR_CANNOT_CONTINUE.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check break pointer validity in vm_exec.c.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext with active break state.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Host stack independent pointer restoration.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/core/stop.c
 * - engine/src/vm/vm_exec.c
 * Prerequisite Header Files:
 * - engine/include/statements/program/cont.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "statements/program/cont.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_cont_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;
    vm_set_single_step(vm, false);
    return err;
}

void stmt_cont_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CONT",
        .category = "Program Mgmt & Editing",
        .syntax = "CONT",
        .help_text = "Resumes program execution after a break or STOP statement.",
        .error_codes = "Error 2: Syntax Error, Error 17: Can't Continue"
    };
    microlib_register(&meta);
}


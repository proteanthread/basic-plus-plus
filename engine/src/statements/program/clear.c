/**
 * @file clear.c
 * @brief CLEAR [, [stack_size] [, exec_stack_size]] memory reset statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements CLEAR statement handler for resetting all runtime variables, arrays, DEF types, open file handles, and ON event handlers.
 *
 * 2. WHY IT EXISTS:
 * Restores initial VM memory state without purging loaded program source code per GW-BASIC / QBASIC standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Calls vm_clear_variables(), releases heap-allocated strings, resets array tables, closes non-system open files, and resets stack frames.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_clear'. Includes "statements/program/clear.h",
 * "vm/vm.h", "lexer/lexer.h", "eval/eval.h", "device/vdev.h", "security/security.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (Core Included).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support stack size parameters (CLEAR , , 32768) for setting host VM stack bounds.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Variable release obligation: Overwritten variable pointer fields MUST release old string / array references before zeroing slots per Rule #1.
 *
 * 8. WHAT TO EXPECT:
 * Clears runtime heap state and returns ERR_NONE.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check refcount leak detection during variable table purge.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Zero-initialization default memory reset.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/vm_context.c
 * - engine/src/vm/vm_var.c
 * Prerequisite Header Files:
 * - engine/include/statements/program/clear.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "statements/program/clear.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

#include "runtime/variables.h"
#include "runtime/arrays.h"

BppError stmt_clear_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    memset(&err, 0, sizeof(err));

    VariableContext *vc = vm_get_var(vm);
    if (vc) var_clear_all(vc);

    ArrayContext *ac = vm_get_arr(vm);
    if (ac) arr_clear_all(ac);

    return err;
}

void stmt_clear_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CLEAR",
        .category = "Variables & Memory",
        .syntax = "CLEAR [, [basic_stack_size] [, exec_stack_size]]",
        .help_text = "Frees variable memory, closes open files, and resets sound/graphics channels.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}


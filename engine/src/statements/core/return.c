/**
 * @file return.c
 * @brief RETURN subroutine return statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements RETURN subroutine completion execution, popping return line address and token offset
 * from the GOSUB call stack and resuming execution past the original GOSUB statement.
 *
 * 2. WHY IT EXISTS:
 * Serves as the counterpart to GOSUB, enabling structured procedural subroutines.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Pops the top call stack frame via vm_gosub_pop(). If the stack is empty, emits ERR_RETURN_WITHOUT_GOSUB
 * (Error 3). Otherwise, performs a VM jump to the saved return line and character offset.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_return'. Directly includes "statements/core/return.h",
 * "vm/vm.h", and "runtime/micro_lib_metadata.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To support explicit target line returns (RETURN line_num), parse optional target line number after
 * popping and clearing the GOSUB frame.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Stack popping invariant: RETURN on an empty stack MUST emit Error 3 (RETURN without GOSUB).
 *
 * 8. WHAT TO EXPECT:
 * Resumes execution at calling location and returns ERR_NONE, or returns ERR_RETURN_WITHOUT_GOSUB.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check vm_gosub_pop() logic in vm/control.c. Ensure stack zero-initialization and balance.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext. Non-empty GOSUB call stack.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Zero host stack manipulation.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/control.c
 * - engine/src/statements/core/gosub.c
 * Prerequisite Header Files:
 * - engine/include/statements/core/return.h
 * - engine/include/vm/vm.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "statements/core/return.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include <string.h>

void stmt_return_register(void) {
    MicroLibMetadata meta = {
        .name = "RETURN",
        .category = "Control Flow",
        .syntax = "RETURN [line_num]",
        .help_text = "Returns control from a GOSUB subroutine to the statement following GOSUB, or specified line.",
        .error_codes = "Error 3: RETURN without GOSUB (call stack empty)"
    };
    microlib_register(&meta);
}

BppError stmt_return_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    BppLineNumber resume_line = 0.0;
    const char *resume_pos = NULL;

    if (!vm_gosub_pop(vm, &resume_line, &resume_pos)) {
        err.code = 3; /* RETURN without GOSUB */
        err.message = "RETURN without GOSUB";
        return err;
    }

    vm_jump(vm, resume_line, resume_pos);
    vm_clear_event_handlers(vm);
    return err;
}

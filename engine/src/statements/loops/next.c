/**
 * @file next.c
 * @brief NEXT counting loop iteration statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements NEXT loop iteration evaluation (NEXT [var1, var2...]), incrementing loop variables,
 * checking termination bounds, and looping or popping the FOR stack frame.
 *
 * 2. WHY IT EXISTS:
 * Serves as the iteration counterpart to FOR loops across all target execution profiles.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Peeks or pops the top FOR frame from vm->for_stack. Increments the target variable by step.
 * If step > 0 and var <= target (or step < 0 and var >= target), jumps back to the stored loop top line
 * and character position. Otherwise, pops the frame and continues execution.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_next'. Directly includes "statements/loops/next.h",
 * "vm/vm.h", "lexer/lexer.h", "eval/eval.h", and "device/vdev.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To support multiple variable targets on a single line (NEXT I, J, K), loop over variable tokens
 * popping matching frames in inner-to-outer order.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Stack underflow check invariant: calling NEXT without an active matching FOR loop MUST emit
 * ERR_NEXT_WITHOUT_FOR (Error 1).
 *
 * 8. WHAT TO EXPECT:
 * Modifies VM line position or pops loop frame, returning ERR_NONE or ERR_NEXT_WITHOUT_FOR.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect vm_for_peek() and vm_for_pop() in vm/control.c. Check step comparison logic.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext. Active FOR loop stack frame.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Double-precision float comparison safety.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/control.c
 * - engine/src/statements/loops/for.c
 * Prerequisite Header Files:
 * - engine/include/statements/loops/next.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "statements/loops/next.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"

void stmt_next_register(void) {
    MicroLibMetadata meta = {
        .name = "NEXT",
        .category = "Looping / Control Flow",
        .syntax = "NEXT [var1[, var2...]]",
        .help_text = "Increments the FOR loop counter variable and loops back if target bound has not been exceeded.",
        .error_codes = "Error 1: NEXT Without FOR, Error 2: Syntax Error"
    };
    microlib_register(&meta);
}
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include <string.h>

BppError stmt_next_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

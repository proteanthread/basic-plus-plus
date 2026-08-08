/**
 * @file rem.c
 * @brief REM and single-quote comment statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements REM and single-quote (') comment statement evaluation (no-op execution).
 *
 * 2. WHY IT EXISTS:
 * Handles inline code documentation and remark statements without altering VM state.
 *
 * 3. WHY IT WORKS THIS WAY:
 * The lexer skips the remainder of the line payload up to end-of-line (EOL). The statement handler
 * performs a zero-allocation no-op and returns ERR_NONE immediately.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_rem'. Directly includes "statements/core/rem.h",
 * "vm/vm.h", and "runtime/micro_lib_metadata.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To store pragmas or compiler annotations inside REM comments (e.g. REM #PRAGMA), parse comment text.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Zero side-effect invariant: comment execution must never mutate VM registers or variable tables.
 *
 * 8. WHAT TO EXPECT:
 * Immediate return of ERR_NONE.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify lexer line-end detection in lexer.c.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid LexerContext positioned at or after the REM/' keyword token.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Universal ASCII comment text handling.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/lexer/lexer.c
 * Prerequisite Header Files:
 * - engine/include/statements/core/rem.h
 * - engine/include/vm/vm.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "statements/core/rem.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include <string.h>

void stmt_rem_register(void) {
    MicroLibMetadata meta = {
        .name = "REM",
        .category = "Control Flow",
        .syntax = "REM [comment text] or ' [comment text]",
        .help_text = "Defines a remark/comment line ignored by the BASIC engine during execution.",
        .error_codes = "None (comments never generate errors)"
    };
    microlib_register(&meta);
}

BppError stmt_rem_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    (void)lex;
    BppError err;
    memset(&err, 0, sizeof(err));
    return err;
}

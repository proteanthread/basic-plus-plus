/**
 * @file if.h
 * @brief Public interface header for IF/THEN/ELSE conditional branching statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares the public handler function stmt_if_handler() for IF conditional execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to invoke IF statement handling.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_if'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add helper function signatures for multi-line block IF evaluation if extending the parser.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public signature stmt_if_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STMT_IF_H and required header include resolution.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Included self-sufficiently by VM statement registry.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-sufficient include guards.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/core/if.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STMT_IF_H
#define STMT_IF_H

#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handles execution of the IF/THEN/ELSE statement.
 * @param vm Active Virtual Machine context.
 * @param lex Active Lexer context positioned at token after IF.
 * @return BppError structure indicating success or error details.
 */
BppError stmt_if_handler(VMContext *vm, LexerContext *lex);
void stmt_if_register(void);

#ifdef __cplusplus
}
#endif

#endif /* STMT_IF_H */

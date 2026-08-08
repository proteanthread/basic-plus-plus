/**
 * @file def.h
 * @brief Public interface header for DEFINT, DEFSNG, DEFDBL, DEFSTR, and DEFUSR statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_def_handler(), stmt_defint_handler(), stmt_defsng_handler(), stmt_defdbl_handler(), stmt_defstr_handler(), stmt_defusr_handler(), stmt_common_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to handle default variable type binding statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_def_var'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add DEFLNG and DEFQUAD prototypes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_*_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_VARIABLES_DEF_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/variables/def.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_VARIABLES_DEF_H
#define STATEMENTS_VARIABLES_DEF_H
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_def_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defint_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defsng_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defdbl_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defstr_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defusr_handler(VMContext *vm, LexerContext *lex);

BppError stmt_common_handler(VMContext *vm, LexerContext *lex);

#endif

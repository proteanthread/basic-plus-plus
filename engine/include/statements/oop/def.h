/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file def.h
 * @brief Public interface header for DEFINT, DEFSNG, DEFDBL, DEFSTR statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_defint_handler(), stmt_defsng_handler(), stmt_defdbl_handler(), stmt_defstr_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes public API allowing VM statement dispatcher to process implicit variable type definition statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_def'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (Core Included).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add prototypes for custom type default setters if introducing novel data primitives.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures for DEF statement handlers.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_STMT_EXTENDED_STMT_DEF_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/oop/def.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_STMT_EXTENDED_STMT_DEF_H
#define STATEMENTS_STMT_EXTENDED_STMT_DEF_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_def_handler(VMContext *vm, LexerContext *lex);
void stmt_def_register(void);

#endif /* STATEMENTS_STMT_EXTENDED_STMT_DEF_H */

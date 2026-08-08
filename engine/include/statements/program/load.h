/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file load.h
 * @brief Public interface header for LOAD and MERGE program source loading statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_load_handler() and stmt_merge_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to process LOAD and MERGE statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_load'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (Core Included).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add loader query helper prototypes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures for LOAD and MERGE statement handlers.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_PROGRAM_LOAD_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/program/load.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_PROGRAM_LOAD_H
#define STATEMENTS_PROGRAM_LOAD_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_load_handler(VMContext *vm, LexerContext *lex);

BppError stmt_merge_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_PROGRAM_LOAD_H */

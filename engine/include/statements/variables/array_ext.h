/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file array_ext.h
 * @brief Public interface header for extended ARRAY statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions for ARRAY PUSH, POP, SHIFT, UNSHIFT, SORT, REVERSE, SEARCH, MAP, and FILTER statements.
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to handle dynamic array manipulation statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_array_ext'. Includes "lexer/lexer.h", "vm/vm.h", "types/errors.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add array slice and comparator function prototypes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_*_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_VARIABLES_ARRAY_EXT_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/variables/array_ext.c
 * Prerequisite Header Files:
 * - engine/include/lexer/lexer.h
 * - engine/include/vm/vm.h
 * - engine/include/types/errors.h
 */

#ifndef STATEMENTS_STMT_EXTENDED_STMT_ARRAYEXT_H
#define STATEMENTS_STMT_EXTENDED_STMT_ARRAYEXT_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_array_sort_handler(VMContext *vm, LexerContext *lex);
void stmt_arrayext_register(void);

#endif /* STATEMENTS_STMT_EXTENDED_STMT_ARRAYEXT_H */

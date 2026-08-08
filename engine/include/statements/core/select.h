/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file select.h
 * @brief Public interface header for SELECT CASE statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler function stmt_select_handler(), stmt_case_handler(), and registration.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to handle SELECT CASE blocks.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_select'. Includes "types/types.h", "lexer/lexer.h", "vm/vm.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add secondary case matching function signatures to this header if needed by custom evaluators.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_select_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_STMT_EXTENDED_STMT_SELECT_H.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guards.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/core/select.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/lexer/lexer.h
 * - engine/include/vm/vm.h
 */

#ifndef STATEMENTS_STMT_EXTENDED_STMT_SELECT_H
#define STATEMENTS_STMT_EXTENDED_STMT_SELECT_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_select_handler(VMContext *vm, LexerContext *lex);
void stmt_select_register(void);

#endif /* STATEMENTS_STMT_EXTENDED_STMT_SELECT_H */

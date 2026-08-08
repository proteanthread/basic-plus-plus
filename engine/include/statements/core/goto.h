/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file goto.h
 * @brief Public interface header for GOTO unconditional branch statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares the public handler function stmt_goto_handler() and metadata registration stmt_goto_register().
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to trigger GOTO jumps.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_goto'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add label search prototype extensions if expanding control flow resolution.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_goto_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Returns BppError enum value.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check header guard STATEMENTS_CORE_STMT_GOTO_H.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained header guards.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/core/goto.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_CORE_STMT_GOTO_H
#define STATEMENTS_CORE_STMT_GOTO_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_goto_handler(VMContext *vm, LexerContext *lex);
void stmt_goto_register(void);

#endif /* STATEMENTS_CORE_STMT_GOTO_H */

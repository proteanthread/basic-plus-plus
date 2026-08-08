/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file try.h
 * @brief Public interface header for TRY / CATCH exception handling statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_try_handler() and self-registration routine stmt_try_register().
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to handle structured exceptions.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_try'. Includes "lexer/lexer.h", "vm/vm.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add exception handler registration signatures if expanding modular error subsystems.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_try_handler(VMContext*, LexerContext*) and stmt_try_register(void).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_STMT_EXTENDED_STMT_TRY_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/event/try.c
 * Prerequisite Header Files:
 * - engine/include/lexer/lexer.h
 * - engine/include/vm/vm.h
 */

#ifndef STATEMENTS_STMT_EXTENDED_STMT_TRY_H
#define STATEMENTS_STMT_EXTENDED_STMT_TRY_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_try_handler(VMContext *vm, LexerContext *lex);
void stmt_try_register(void);

#endif /* STATEMENTS_STMT_EXTENDED_STMT_TRY_H */

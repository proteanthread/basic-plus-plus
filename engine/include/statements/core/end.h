/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file end.h
 * @brief Public interface header for END statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler function stmt_end_handler() and metadata registration stmt_end_register().
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to trigger END statement evaluation.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_end'. Includes "types/types.h", "lexer/lexer.h", "vm/vm.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add block terminator registration prototypes if adding new language constructs.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_end_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STMT_END_H and required header inclusions.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/core/end.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/lexer/lexer.h
 * - engine/include/vm/vm.h
 */

#ifndef STMT_END_H
#define STMT_END_H

#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_end_handler(VMContext *vm, LexerContext *lex);
void stmt_end_register(void);

#endif /* STMT_END_H */

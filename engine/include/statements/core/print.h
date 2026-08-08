/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file print.h
 * @brief Public interface header for PRINT and PRINT USING statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares the public handler function signature stmt_print_handler() for PRINT execution.
 *
 * 2. WHY IT EXISTS:
 * Provides the public API header contract allowing the VM dispatcher to invoke PRINT statement processing.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_print'. Self-sufficient header including "types/types.h", "vm/vm.h", and "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add secondary public formatting helper function signatures to this header if needed by external callers.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_print_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_CORE_PRINT_H and direct include resolution.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Included alone or by statement dispatcher.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-sufficient include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/core/print.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_CORE_PRINT_H
#define STATEMENTS_CORE_PRINT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_print_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_CORE_PRINT_H */

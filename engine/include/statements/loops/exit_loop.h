/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file exit_loop.h
 * @brief Public interface header for EXIT loop break statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler function stmt_exit_loop_handler() and stmt_exit_handler() for EXIT loop execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to trigger early loop exit.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_exit_loop'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add multi-level exit function signatures if expanding loop breaking keywords.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_exit_loop_handler(VMContext*, LexerContext*) and stmt_exit_handler.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_LOOPS_EXIT_LOOP_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/loops/exit_loop.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_LOOPS_EXIT_LOOP_H
#define STATEMENTS_LOOPS_EXIT_LOOP_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_exit_loop_handler(VMContext *vm, LexerContext *lex);

BppError stmt_exit_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_LOOPS_EXIT_LOOP_H */

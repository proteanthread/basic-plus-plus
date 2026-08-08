/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file mat_ops.h
 * @brief Public interface header for MAT linear algebra operation statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_mat_ops_handler() and stmt_mat_handler() for MAT statement execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to handle MAT matrix operation statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_mat_ops'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (SUPPORT_MAT).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add matrix calculation function prototypes if exposing matrix math utilities to C extensions.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_mat_ops_handler(VMContext*, LexerContext*) and stmt_mat_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_MATRICES_MAT_OPS_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/matrices/mat_ops.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_MATRICES_MAT_OPS_H
#define STATEMENTS_MATRICES_MAT_OPS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_mat_ops_handler(VMContext *vm, LexerContext *lex);

BppError stmt_mat_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_MATRICES_MAT_OPS_H */

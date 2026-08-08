/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file defseg.h
 * @brief Public interface header for DEF SEG segment base definition statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler function stmt_defseg_handler() for DEF SEG statement execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to execute DEF SEG segment definition statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_defseg' (libbasicpp ONLY). Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * EXCLUDED from libbasicpp_lite (bpp, bs) per Rule #1 (Segmented memory subsystem `vmem`). FULLY included in libbasicpp (baspp standard desktop).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add segment register helper prototypes if extending memory inspection routines.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_defseg_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_SYSTEM_DEFSEG_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/io/defseg.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_SYSTEM_DEFSEG_H
#define STATEMENTS_SYSTEM_DEFSEG_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_SYSTEM_DEFSEG_H */

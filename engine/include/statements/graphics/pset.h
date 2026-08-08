/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file pset.h
 * @brief Public interface header for PSET pixel plot statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler function stmt_pset_handler() for PSET statement execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to execute PSET pixel plot statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_pset' (libbasicpp ONLY). Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * EXCLUDED from libbasicpp_lite (bpp, bs) per Rule #1 (SDL2 graphics subsystem). FULLY included in libbasicpp (baspp standard desktop).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add pixel plot helper function prototypes if extending vector drawing API.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_pset_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_GRAPHICS_PSET_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/graphics/pset.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_GRAPHICS_PSET_H
#define STATEMENTS_GRAPHICS_PSET_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_pset_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_GRAPHICS_PSET_H */

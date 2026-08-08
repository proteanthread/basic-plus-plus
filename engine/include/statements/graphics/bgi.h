/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bgi.h
 * @brief Public interface header for Borland Graphics Interface (BGI) statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public statement handlers for BGI graphics primitives (INITGRAPH, CLOSEGRAPH, PUTPIXEL, LINETO, etc.).
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to execute Turbo BASIC BGI commands.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_bgi' (libbasicpp ONLY). Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * EXCLUDED from libbasicpp_lite (bpp, bs) per Rule #1 (BGI graphics subsystem). FULLY included in libbasicpp (baspp standard desktop).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add new BGI primitive handler prototypes when extending graphic capabilities.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Standard handler function signatures for BGI statement entry points.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_GRAPHICS_BGI_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/graphics/bgi.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_GRAPHICS_BGI_H
#define STATEMENTS_GRAPHICS_BGI_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_bgi_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_GRAPHICS_BGI_H */

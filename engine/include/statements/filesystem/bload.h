/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bload.h
 * @brief Public interface header for BLOAD / BSAVE binary I/O statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_bload_handler(), stmt_bsave_handler(), and stmt_brun_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to handle BLOAD and BSAVE statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_bload'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add binary offset validation prototypes if extending memory safety tools.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_bload_handler, stmt_bsave_handler, stmt_brun_handler.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_FILESYSTEM_BLOAD_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/filesystem/bload.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_FILESYSTEM_BLOAD_H
#define STATEMENTS_FILESYSTEM_BLOAD_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_bload_handler(VMContext *vm, LexerContext *lex);

BppError stmt_bsave_handler(VMContext *vm, LexerContext *lex);

BppError stmt_brun_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_FILESYSTEM_BLOAD_H */

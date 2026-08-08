/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file save.h
 * @brief Public interface header for SAVE program source saving statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler function stmt_save_handler() for SAVE statement execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to handle SAVE statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_save'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (Core Included).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add saver query helper prototypes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_save_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_PROGRAM_SAVE_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/program/save.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_PROGRAM_SAVE_H
#define STATEMENTS_PROGRAM_SAVE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_save_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_PROGRAM_SAVE_H */

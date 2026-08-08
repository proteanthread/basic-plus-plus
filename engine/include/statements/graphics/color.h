/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file color.h
 * @brief Public interface header for COLOR, BCOLOR, FCOLOR, CURSOR, LOCATE statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_color_handler(), stmt_bcolor_handler(), stmt_fcolor_handler(), stmt_cursor_handler(), stmt_locate_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes text attribute and screen positioning interfaces for console/graphics manipulation.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_color'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add text attribute query prototypes when extending virtual console features.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures for color/cursor handlers.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_GRAPHICS_COLOR_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/graphics/color.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_GRAPHICS_COLOR_H
#define STATEMENTS_GRAPHICS_COLOR_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_color_handler(VMContext *vm, LexerContext *lex);

BppError stmt_bcolor_handler(VMContext *vm, LexerContext *lex);

BppError stmt_fcolor_handler(VMContext *vm, LexerContext *lex);

BppError stmt_cursor_handler(VMContext *vm, LexerContext *lex);

BppError stmt_locate_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_GRAPHICS_COLOR_H */

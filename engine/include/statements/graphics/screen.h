/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file screen.h
 * @brief Public interface header for SCREEN, TITLE, SCREENMOVE, FULLSCREEN, RESIZE statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_screen_handler(), stmt_title_handler(), stmt_screenmove_handler(), stmt_fullscreen_handler(), stmt_resize_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes video mode configuration and window frame control interfaces to the VM statement dispatcher.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_screen' (libbasicpp ONLY). Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * EXCLUDED from libbasicpp_lite (bpp, bs) per Rule #1 (SDL2 graphics subsystem). FULLY included in libbasicpp (baspp standard desktop).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add window attribute prototype functions (e.g. window transparency) if expanding window controls.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures for video mode and window handlers.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_GRAPHICS_SCREEN_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/graphics/screen.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_GRAPHICS_SCREEN_H
#define STATEMENTS_GRAPHICS_SCREEN_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_screen_handler(VMContext *vm, LexerContext *lex);

BppError stmt_title_handler(VMContext *vm, LexerContext *lex);

BppError stmt_screenmove_handler(VMContext *vm, LexerContext *lex);

BppError stmt_fullscreen_handler(VMContext *vm, LexerContext *lex);

BppError stmt_resize_handler(VMContext *vm, LexerContext *lex);

BppError stmt_icon_handler(VMContext *vm, LexerContext *lex);

BppError stmt_freeimage_handler(VMContext *vm, LexerContext *lex);

BppError stmt_putimage_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_GRAPHICS_SCREEN_H */

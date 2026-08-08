/**
 * @file compat.h
 * @brief Public interface header for legacy system graphics statement compatibility handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions for legacy Apple II, Atari BASIC, and ZX Spectrum graphics statements.
 *
 * 2. WHY IT EXISTS:
 * Exposes retro computer graphics statement prototypes for VM dialect compatibility dispatchers.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_graphics_compat' (libbasicpp ONLY). Includes "types/errors.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * EXCLUDED from libbasicpp_lite (bpp, bs) per Rule #1 (SDL2 graphics subsystem). FULLY included in libbasicpp (baspp standard desktop).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add legacy graphics handler prototypes when adding retro computer system dialects.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures for legacy graphics handlers.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STMT_COMPAT_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/graphics/compat.c
 * Prerequisite Header Files:
 * - engine/include/types/errors.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */
#ifndef STMT_COMPAT_H
#define STMT_COMPAT_H
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_gr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hgr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hgr2_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_plot_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hlin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vlin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hplot_handler(VMContext *vm, LexerContext *lex);
BppError stmt_graphics_handler(VMContext *vm, LexerContext *lex);
BppError stmt_drawto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_border_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ink_handler(VMContext *vm, LexerContext *lex);
BppError stmt_paper_handler(VMContext *vm, LexerContext *lex);
#endif

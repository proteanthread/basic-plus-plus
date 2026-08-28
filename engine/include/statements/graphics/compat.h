// FILENAME: compat.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (compat.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the COMPAT statement in BASIC++.
//
// ---- Includes ----

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
void stmt_gfx_compat_register(void);

#endif

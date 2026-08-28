// FILENAME: stmt_plot.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_plot.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the PLOT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_STMT_PLOT_H
#define STATEMENTS_GRAPHICS_STMT_PLOT_H

#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_plot_register(void);
BppError stmt_plot_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_GRAPHICS_STMT_PLOT_H

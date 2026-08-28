// FILENAME: page.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (page.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the PAGE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_IO_PAGE_H
#define STATEMENTS_IO_PAGE_H

#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_page_register(void);
BppError stmt_page_handler(VMContext *vm, LexerContext *lex);
BppError stmt_nopage_handler(VMContext *vm, LexerContext *lex);
BppError stmt_nomargin_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_IO_PAGE_H

// FILENAME: stmt_gemini_browse.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_gemini_browse.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Declares GEMINI.BROWSE interactive TUI capsule browser.
//
// ---- Includes ----

#ifndef STATEMENTS_NETWORK_STMT_GEMINI_BROWSE_H
#define STATEMENTS_NETWORK_STMT_GEMINI_BROWSE_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_gemini_browse_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_NETWORK_STMT_GEMINI_BROWSE_H

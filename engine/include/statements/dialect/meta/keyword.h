// FILENAME: keyword.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, keyword.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the KEYWORD statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DIALECT_KEYWORD_H
#define STATEMENTS_DIALECT_KEYWORD_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

// @brief Main statement handler for KEYWORD.
BppError stmt_keyword_handler(VMContext *vm, LexerContext *lex);
void stmt_keyword_register(void);

#endif // STATEMENTS_DIALECT_KEYWORD_H

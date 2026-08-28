// FILENAME: viewport.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (viewport.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VIEWPORT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_BGI_VIEWPORT_H
#define STATEMENTS_BGI_VIEWPORT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_viewport_handler(VMContext *vm, LexerContext *lex);
void stmt_viewport_register(void);

#endif // STATEMENTS_BGI_VIEWPORT_H

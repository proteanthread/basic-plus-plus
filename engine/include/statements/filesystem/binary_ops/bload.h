// FILENAME: bload.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (bload.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the BLOAD statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_BLOAD_H
#define STATEMENTS_FILESYSTEM_BLOAD_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_bload_handler(VMContext *vm, LexerContext *lex);
void stmt_bload_register(void);

#endif // STATEMENTS_FILESYSTEM_BLOAD_H

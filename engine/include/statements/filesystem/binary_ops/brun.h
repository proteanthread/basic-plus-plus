// FILENAME: brun.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (brun.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the BRUN statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_BRUN_H
#define STATEMENTS_FILESYSTEM_BRUN_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_brun_handler(VMContext *vm, LexerContext *lex);
void stmt_brun_register(void);

#endif // STATEMENTS_FILESYSTEM_BRUN_H

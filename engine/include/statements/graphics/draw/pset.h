// FILENAME: pset.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (pset.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the PSET statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_PSET_H
#define STATEMENTS_GRAPHICS_PSET_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_pset_handler(VMContext *vm, LexerContext *lex);

void stmt_pset_register(void);

#endif // STATEMENTS_GRAPHICS_PSET_H

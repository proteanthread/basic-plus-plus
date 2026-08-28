// FILENAME: lset.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (lset.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the LSET statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_LSET_H
#define STATEMENTS_VARIABLES_LSET_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_lset_handler(VMContext *vm, LexerContext *lex);
void stmt_lset_register(void);

#endif // STATEMENTS_VARIABLES_LSET_H

// FILENAME: let.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (let.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the LET statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_LET_H
#define STATEMENTS_VARIABLES_LET_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_let_handler(VMContext *vm, LexerContext *lex);
void stmt_let_register(void);

#endif // STATEMENTS_VARIABLES_LET_H

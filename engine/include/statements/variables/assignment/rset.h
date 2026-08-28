// FILENAME: rset.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (rset.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the RSET statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_RSET_H
#define STATEMENTS_VARIABLES_RSET_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_rset_handler(VMContext *vm, LexerContext *lex);
void stmt_rset_register(void);

#endif // STATEMENTS_VARIABLES_RSET_H

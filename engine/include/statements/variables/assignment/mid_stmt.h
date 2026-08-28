// FILENAME: mid_stmt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mid_stmt.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the MID_STMT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_MID_STMT_H
#define STATEMENTS_VARIABLES_MID_STMT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_mid_stmt_handler(VMContext *vm, LexerContext *lex);

void stmt_mid_stmt_register(void);

#endif // STATEMENTS_VARIABLES_MID_STMT_H

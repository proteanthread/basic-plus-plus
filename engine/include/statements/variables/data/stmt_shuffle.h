// FILENAME: stmt_shuffle.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, stmt_shuffle.c)
// NEEDS: libkernel (types.h), libengine (lexer.h, vm.h)
// Provides runtime declaration for the SHUFFLE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_DATA_STMT_SHUFFLE_H
#define STATEMENTS_VARIABLES_DATA_STMT_SHUFFLE_H

#include "lexer/lexer.h"
#include "types/types.h"
#include "vm/vm.h"

BppError stmt_shuffle_handler(VMContext *vm, LexerContext *lex);
void stmt_shuffle_register(void);

#endif // STATEMENTS_VARIABLES_DATA_STMT_SHUFFLE_H

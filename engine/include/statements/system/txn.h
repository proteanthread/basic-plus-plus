// FILENAME: txn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the TXN statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_TXN_H
#define STATEMENTS_SYSTEM_TXN_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_txn_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_SYSTEM_TXN_H

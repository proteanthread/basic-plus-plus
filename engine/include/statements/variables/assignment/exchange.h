// FILENAME: exchange.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exchange.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the EXCHANGE statement in BASIC++.
//
// ---- Includes ----

#ifndef EXCHANGE_H
#define EXCHANGE_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_exchange_register(void);
BppError stmt_exchange_handler(VMContext *vm, LexerContext *lex);

#endif // EXCHANGE_H

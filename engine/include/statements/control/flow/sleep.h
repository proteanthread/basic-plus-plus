// FILENAME: sleep.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sleep.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the SLEEP statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CONTROL_SLEEP_H
#define STATEMENTS_CONTROL_SLEEP_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_sleep_handler(VMContext *vm, LexerContext *lex);
void stmt_sleep_register(void);

#endif // STATEMENTS_CONTROL_SLEEP_H

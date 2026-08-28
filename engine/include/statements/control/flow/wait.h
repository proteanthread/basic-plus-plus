// FILENAME: wait.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (wait.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the WAIT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CONTROL_WAIT_H
#define STATEMENTS_CONTROL_WAIT_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_wait_handler(VMContext *vm, LexerContext *lex);
void stmt_wait_register(void);

#endif // STATEMENTS_CONTROL_WAIT_H

// FILENAME: continue.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (continue.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the CONTINUE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CONTROL_CONTINUE_H
#define STATEMENTS_CONTROL_CONTINUE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_continue_handler(VMContext *vm, LexerContext *lex);
void stmt_continue_register(void);

#endif // STATEMENTS_CONTROL_CONTINUE_H

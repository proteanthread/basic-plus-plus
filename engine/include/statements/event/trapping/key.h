// FILENAME: key.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (key.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the KEY statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_EVENT_KEY_H
#define STATEMENTS_EVENT_KEY_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_key_register(void);
BppError stmt_key_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_EVENT_KEY_H

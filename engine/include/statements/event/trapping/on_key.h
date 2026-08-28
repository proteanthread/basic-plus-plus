// FILENAME: on_key.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (on_key.c, on_timer.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the ON_KEY statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_EVENT_ON_KEY_H
#define STATEMENTS_EVENT_ON_KEY_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_on_key_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_EVENT_ON_KEY_H

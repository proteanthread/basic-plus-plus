// FILENAME: lock.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (lock.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the LOCK statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_LOCK_H
#define STATEMENTS_FILESYSTEM_LOCK_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_lock_handler(VMContext *vm, LexerContext *lex);

BppError stmt_unlock_handler(VMContext *vm, LexerContext *lex);

void stmt_lock_register(void);

#endif // STATEMENTS_FILESYSTEM_LOCK_H

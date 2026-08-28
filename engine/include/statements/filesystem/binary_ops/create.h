// FILENAME: create.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (create.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the CREATE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_CREATE_H
#define STATEMENTS_FILESYSTEM_CREATE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_create_register(void);
BppError stmt_create_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_CREATE_H

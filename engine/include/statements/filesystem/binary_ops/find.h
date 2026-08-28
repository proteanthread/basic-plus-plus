// FILENAME: find.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (find.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the FIND statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_FIND_H
#define STATEMENTS_FILESYSTEM_FIND_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_find_handler(VMContext *vm, LexerContext *lex);
void stmt_find_register(void);

#endif // STATEMENTS_FILESYSTEM_FIND_H

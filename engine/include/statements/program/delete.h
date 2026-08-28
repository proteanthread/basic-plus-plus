// FILENAME: delete.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (delete.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DELETE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_PROGRAM_DELETE_H
#define STATEMENTS_PROGRAM_DELETE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_delete_handler(VMContext *vm, LexerContext *lex);
void stmt_delete_register(void);

#endif // STATEMENTS_PROGRAM_DELETE_H

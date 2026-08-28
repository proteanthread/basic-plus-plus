// FILENAME: assign.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (assign.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the ASSIGN statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_ASSIGN_H
#define STATEMENTS_FILESYSTEM_ASSIGN_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_assign_handler(VMContext *vm, LexerContext *lex);
BppError stmt_advance_handler(VMContext *vm, LexerContext *lex);
void stmt_assign_register(void);

#endif // STATEMENTS_FILESYSTEM_ASSIGN_H

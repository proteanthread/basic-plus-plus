// FILENAME: read.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (read.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the READ statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_READ_H
#define STATEMENTS_VARIABLES_READ_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_read_handler(VMContext *vm, LexerContext *lex);
void stmt_read_register(void);

#endif // STATEMENTS_VARIABLES_READ_H

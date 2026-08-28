// FILENAME: while.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (while.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the WHILE statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_WHILE_H
#define STMT_WHILE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BppError stmt_while_handler(VMContext *vm, LexerContext *lex);
void stmt_while_register(void);

#endif // STMT_WHILE_H

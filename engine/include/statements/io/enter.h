// FILENAME: enter.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (enter.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Provides runtime implementation for the ENTER statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_IO_ENTER_H
#define STATEMENTS_IO_ENTER_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_enter_register(void);
BppError stmt_enter_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_IO_ENTER_H

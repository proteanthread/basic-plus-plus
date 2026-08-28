// FILENAME: repeat.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (repeat.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the REPEAT statement in BASIC++.
//
// ---- Includes ----

#ifndef REPEAT_H
#define REPEAT_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_repeat_register(void);
BppError stmt_repeat_handler(VMContext *vm, LexerContext *lex);

#endif // REPEAT_H

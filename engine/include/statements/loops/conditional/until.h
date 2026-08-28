// FILENAME: until.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (until.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the UNTIL statement in BASIC++.
//
// ---- Includes ----

#ifndef UNTIL_H
#define UNTIL_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_until_register(void);
BppError stmt_until_handler(VMContext *vm, LexerContext *lex);

#endif // UNTIL_H

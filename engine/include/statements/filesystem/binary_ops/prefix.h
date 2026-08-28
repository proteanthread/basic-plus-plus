// FILENAME: prefix.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (prefix.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the PREFIX statement in BASIC++.
//
// ---- Includes ----

#ifndef PREFIX_H
#define PREFIX_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_prefix_register(void);
BppError stmt_prefix_handler(VMContext *vm, LexerContext *lex);

#endif // PREFIX_H

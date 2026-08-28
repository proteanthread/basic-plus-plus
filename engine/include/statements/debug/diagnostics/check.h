// FILENAME: check.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (check.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the CHECK statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DEBUG_CHECK_H
#define STATEMENTS_DEBUG_CHECK_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_check_handler(VMContext *vm, LexerContext *lex);
void stmt_check_register(void);

#endif // STATEMENTS_DEBUG_CHECK_H

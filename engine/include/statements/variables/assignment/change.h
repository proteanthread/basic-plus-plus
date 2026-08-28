// FILENAME: change.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (change.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the CHANGE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_CHANGE_H
#define STATEMENTS_VARIABLES_CHANGE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_change_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_VARIABLES_CHANGE_H

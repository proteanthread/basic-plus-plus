// FILENAME: gosub.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (gosub.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the GOSUB statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CORE_STMT_GOSUB_H
#define STATEMENTS_CORE_STMT_GOSUB_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_gosub_handler(VMContext *vm, LexerContext *lex);
void stmt_gosub_register(void);

#endif // STATEMENTS_CORE_STMT_GOSUB_H

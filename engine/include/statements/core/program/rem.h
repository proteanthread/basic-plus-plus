// FILENAME: rem.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (rem.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the REM statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_REM_H
#define STMT_REM_H

#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_rem_handler(VMContext *vm, LexerContext *lex);
void stmt_rem_register(void);

#endif // STMT_REM_H

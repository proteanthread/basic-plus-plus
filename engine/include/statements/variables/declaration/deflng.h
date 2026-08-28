// FILENAME: deflng.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (deflng.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DEFLNG statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_DEFLNG_H
#define STATEMENTS_VARIABLES_DEFLNG_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_deflng_handler(VMContext *vm, LexerContext *lex);
void stmt_deflng_register(void);

#endif // STATEMENTS_VARIABLES_DEFLNG_H

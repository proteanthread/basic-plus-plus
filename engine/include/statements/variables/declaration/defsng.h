// FILENAME: defsng.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (defsng.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DEFSNG statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_DEFSNG_H
#define STATEMENTS_VARIABLES_DEFSNG_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_defsng_handler(VMContext *vm, LexerContext *lex);
void stmt_defsng_register(void);

#endif // STATEMENTS_VARIABLES_DEFSNG_H

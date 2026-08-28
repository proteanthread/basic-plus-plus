// FILENAME: print.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the PRINT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CORE_PRINT_H
#define STATEMENTS_CORE_PRINT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_print_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_CORE_PRINT_H

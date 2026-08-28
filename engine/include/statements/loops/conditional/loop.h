// FILENAME: loop.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (loop.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the LOOP statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_LOOPS_LOOP_H
#define STATEMENTS_LOOPS_LOOP_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_loop_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_LOOPS_LOOP_H

// FILENAME: pen.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (pen.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the PEN statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_PEN_H
#define STATEMENTS_SYSTEM_PEN_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_pen_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_SYSTEM_PEN_H

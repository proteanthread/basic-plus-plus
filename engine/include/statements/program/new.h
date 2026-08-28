// FILENAME: new.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (new.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the NEW statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_PROGRAM_NEW_H
#define STATEMENTS_PROGRAM_NEW_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_new_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_PROGRAM_NEW_H

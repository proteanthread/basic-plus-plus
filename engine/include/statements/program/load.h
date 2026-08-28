// FILENAME: load.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (load.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the LOAD statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_PROGRAM_LOAD_H
#define STATEMENTS_PROGRAM_LOAD_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_load_handler(VMContext *vm, LexerContext *lex);

BppError stmt_merge_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_PROGRAM_LOAD_H

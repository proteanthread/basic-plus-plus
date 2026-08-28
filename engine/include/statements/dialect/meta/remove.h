// FILENAME: remove.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the REMOVE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DIALECT_REMOVE_H
#define STATEMENTS_DIALECT_REMOVE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_remove_handler(VMContext *vm, LexerContext *lex);

BppError stmt_remove_str_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_DIALECT_REMOVE_H

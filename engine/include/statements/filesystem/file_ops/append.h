// FILENAME: append.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (append.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the APPEND statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_APPEND_H
#define STATEMENTS_FILESYSTEM_APPEND_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_append_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_APPEND_H

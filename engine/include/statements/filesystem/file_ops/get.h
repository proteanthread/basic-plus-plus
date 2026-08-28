// FILENAME: get.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (get.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the GET statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_GET_H
#define STATEMENTS_FILESYSTEM_GET_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_get_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_GET_H

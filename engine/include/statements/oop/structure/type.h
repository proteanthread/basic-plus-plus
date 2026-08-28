// FILENAME: type.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (type.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the TYPE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_OOP_TYPE_H
#define STATEMENTS_OOP_TYPE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_type_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_type_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_OOP_TYPE_H

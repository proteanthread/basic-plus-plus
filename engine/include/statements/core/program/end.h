// FILENAME: end.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (end.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the END statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_END_H
#define STMT_END_H

#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_end_handler(VMContext *vm, LexerContext *lex);
void stmt_end_register(void);

#endif // STMT_END_H

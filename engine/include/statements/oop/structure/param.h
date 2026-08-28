// FILENAME: param.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (param.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the PARAM statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_OOP_PARAM_H
#define STATEMENTS_OOP_PARAM_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_param_register(void);
BppError stmt_param_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_OOP_PARAM_H

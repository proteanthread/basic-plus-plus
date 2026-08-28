// FILENAME: out.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (out.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the OUT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_OUT_H
#define STATEMENTS_SYSTEM_OUT_H

#include "lexer/lexer.h"
#include "vm/vm.h"
#include "types/errors.h"

void stmt_out_register(void);
BppError stmt_out_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_SYSTEM_OUT_H

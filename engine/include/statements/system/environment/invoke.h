// FILENAME: invoke.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (invoke.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the INVOKE statement in BASIC++.
//
// ---- Includes ----

#ifndef INVOKE_H
#define INVOKE_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_invoke_register(void);
BppError stmt_invoke_handler(VMContext *vm, LexerContext *lex);

#endif // INVOKE_H

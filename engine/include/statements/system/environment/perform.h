// FILENAME: perform.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (perform.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the PERFORM statement in BASIC++.
//
// ---- Includes ----

#ifndef PERFORM_H
#define PERFORM_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_perform_register(void);
BppError stmt_perform_handler(VMContext *vm, LexerContext *lex);

#endif // PERFORM_H

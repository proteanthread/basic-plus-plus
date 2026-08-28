// FILENAME: whenever.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (whenever.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the WHENEVER statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_EVENT_WHENEVER_H
#define STATEMENTS_EVENT_WHENEVER_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void     stmt_whenever_register(void);
BppError stmt_whenever_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_EVENT_WHENEVER_H

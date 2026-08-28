// FILENAME: system.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (system.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the SYSTEM statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_SYSTEM_H
#define STATEMENTS_SYSTEM_SYSTEM_H

#include "lexer/lexer.h"
#include "vm/vm.h"
#include "types/errors.h"

BppError stmt_system_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bye_handler(VMContext *vm, LexerContext *lex);
void stmt_system_register(void);

#endif // STATEMENTS_SYSTEM_SYSTEM_H

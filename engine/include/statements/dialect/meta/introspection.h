// FILENAME: introspection.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the INTROSPECTION statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DIALECT_INTROSPECTION_H
#define STATEMENTS_DIALECT_INTROSPECTION_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_hostname_handler(VMContext *vm, LexerContext *lex);
BppError stmt_username_handler(VMContext *vm, LexerContext *lex);
void stmt_introspection_register(void);

#endif // STATEMENTS_DIALECT_INTROSPECTION_H

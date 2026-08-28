// FILENAME: help.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the HELP statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DIALECT_HELP_H
#define STATEMENTS_DIALECT_HELP_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_help_handler(VMContext *vm, LexerContext *lex);
BppError stmt_catalog_handler(VMContext *vm, LexerContext *lex);
void stmt_help_register(void);

#endif // STATEMENTS_DIALECT_HELP_H

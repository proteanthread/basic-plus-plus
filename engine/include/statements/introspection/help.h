// FILENAME: help.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (help.c)
// NEEDS: libcore (types.h)
// Declares the HELP and CATALOG statement handler interface in BASIC++.

#ifndef HELP_H
#define HELP_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_help_handler(VMContext *vm, LexerContext *lex);
BppError stmt_catalog_handler(VMContext *vm, LexerContext *lex);
void stmt_help_register(void);

#endif // HELP_H

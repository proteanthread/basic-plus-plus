// FILENAME: keyword.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, keyword.c)
// NEEDS: libengine (lexer.h, vm.h)
// NEEDS: libkernel (types.h)
// Declares the KEYWORD statement handler interface in BASIC++.

#ifndef STATEMENTS_INTROSPECTION_KEYWORD_H
#define STATEMENTS_INTROSPECTION_KEYWORD_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_keyword_handler(VMContext *vm, LexerContext *lex);
void stmt_keyword_register(void);

#endif // STATEMENTS_INTROSPECTION_KEYWORD_H

// FILENAME: def.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (def.c, eval_expr_internal.h, exec_internal.h)
// NEEDED BY: libengine (sub_internal.h)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the DEF statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_STMT_EXTENDED_STMT_DEF_H
#define STATEMENTS_STMT_EXTENDED_STMT_DEF_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_def_handler(VMContext *vm, LexerContext *lex);
void stmt_def_register(void);

#endif // STATEMENTS_STMT_EXTENDED_STMT_DEF_H

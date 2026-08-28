// FILENAME: goto.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (goto.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the GOTO statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CORE_STMT_GOTO_H
#define STATEMENTS_CORE_STMT_GOTO_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_goto_handler(VMContext *vm, LexerContext *lex);
void stmt_goto_register(void);

#endif // STATEMENTS_CORE_STMT_GOTO_H

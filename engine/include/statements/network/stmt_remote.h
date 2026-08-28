// FILENAME: stmt_remote.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_remote.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides REMOTE statement handler prototype.
//
// ---- Includes ----

#ifndef STATEMENTS_NETWORK_STMT_REMOTE_H
#define STATEMENTS_NETWORK_STMT_REMOTE_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_remote_handler(VMContext *vm, LexerContext *lex);
void     stmt_remote_register(void);

#endif // STATEMENTS_NETWORK_STMT_REMOTE_H

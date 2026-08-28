// FILENAME: stmt_sock.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_sock.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Declares low-level BSD SOCK statements.
//
// ---- Includes ----

#ifndef STATEMENTS_NETWORK_STMT_SOCK_H
#define STATEMENTS_NETWORK_STMT_SOCK_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_sock_handler(VMContext *vm, LexerContext *lex);
void     stmt_sock_register(void);

#endif // STATEMENTS_NETWORK_STMT_SOCK_H

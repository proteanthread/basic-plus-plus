// FILENAME: stmt_peer.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_peer.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Declares the PEER statement handler for universal peer-to-peer communications.
//
// ---- Includes ----

#ifndef STATEMENTS_NETWORK_STMT_PEER_H
#define STATEMENTS_NETWORK_STMT_PEER_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_peer_handler(VMContext *vm, LexerContext *lex);
void     stmt_peer_register(void);

#endif // STATEMENTS_NETWORK_STMT_PEER_H

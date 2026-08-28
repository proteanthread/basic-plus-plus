// FILENAME: stmt_net_config.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_net_config.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Declares NET.CONFIG statement handler for IP and NIC configuration.
//
// ---- Includes ----

#ifndef STATEMENTS_NETWORK_STMT_NET_CONFIG_H
#define STATEMENTS_NETWORK_STMT_NET_CONFIG_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_net_config_handler(VMContext *vm, LexerContext *lex);
void     stmt_net_config_register(void);

#endif // STATEMENTS_NETWORK_STMT_NET_CONFIG_H

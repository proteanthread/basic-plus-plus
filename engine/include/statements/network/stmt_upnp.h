// FILENAME: stmt_upnp.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libserver (stmt_upnp.c)
// NEEDS: libengine (vm.h, lexer.h)
// NEEDS: libkernel (types.h)
// Declares UPnP port forwarding statements (UPNP.FORWARD, UPNP.UNFORWARD).

#ifndef STATEMENTS_NETWORK_STMT_UPNP_H
#define STATEMENTS_NETWORK_STMT_UPNP_H

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "types/types.h"

BppError stmt_upnp_handler(VMContext *vm, LexerContext *lex);
BppError stmt_upnp_forward_handler(VMContext *vm, LexerContext *lex);
BppError stmt_upnp_unforward_handler(VMContext *vm, LexerContext *lex);
void     stmt_upnp_register(void);

#endif // STATEMENTS_NETWORK_STMT_UPNP_H

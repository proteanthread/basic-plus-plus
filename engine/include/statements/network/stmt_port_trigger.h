// FILENAME: stmt_port_trigger.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_port_trigger.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Declares PORT statement handler for port knocking, triggering and trapping.
//
// ---- Includes ----

#ifndef STATEMENTS_NETWORK_STMT_PORT_TRIGGER_H
#define STATEMENTS_NETWORK_STMT_PORT_TRIGGER_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_port_trigger_handler(VMContext *vm, LexerContext *lex);
void     stmt_port_trigger_register(void);

#endif // STATEMENTS_NETWORK_STMT_PORT_TRIGGER_H

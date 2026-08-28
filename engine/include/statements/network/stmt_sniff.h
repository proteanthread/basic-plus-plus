// FILENAME: stmt_sniff.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_sniff.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Declares SNIFF statement handler for promiscuous packet capture.
//
// ---- Includes ----

#ifndef STATEMENTS_NETWORK_STMT_SNIFF_H
#define STATEMENTS_NETWORK_STMT_SNIFF_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_sniff_handler(VMContext *vm, LexerContext *lex);
void     stmt_sniff_register(void);

#endif // STATEMENTS_NETWORK_STMT_SNIFF_H

// FILENAME: introspection.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (introspection.c)
// NEEDS: libcore (types.h)
// Declares the HOSTNAME and USERNAME statement handler interface in BASIC++.

#ifndef INTROSPECTION_H
#define INTROSPECTION_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_hostname_handler(VMContext *vm, LexerContext *lex);
BppError stmt_username_handler(VMContext *vm, LexerContext *lex);
BppError stmt_comspec_handler(VMContext *vm, LexerContext *lex);
BppError stmt_logname_handler(VMContext *vm, LexerContext *lex);
BppError stmt_homepath_handler(VMContext *vm, LexerContext *lex);
BppError stmt_homedrive_handler(VMContext *vm, LexerContext *lex);
BppError stmt_userpath_handler(VMContext *vm, LexerContext *lex);
BppError stmt_computername_handler(VMContext *vm, LexerContext *lex);
BppError stmt_totalmem_handler(VMContext *vm, LexerContext *lex);
BppError stmt_availmem_handler(VMContext *vm, LexerContext *lex);
BppError stmt_uptime_handler(VMContext *vm, LexerContext *lex);
BppError stmt_epoch_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unixtime_handler(VMContext *vm, LexerContext *lex);
BppError stmt_stardate_handler(VMContext *vm, LexerContext *lex);
void stmt_introspection_register(void);
void stmt_introspection_register_all(VMContext *vm);

#endif // INTROSPECTION_H

// FILENAME: stmt_nil_bead.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_net_config.c, stmt_nil_bead.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides statement handler prototype for NET.UNPACK and NIL.UNPACK.
//
// ---- Includes ----

#ifndef STATEMENTS_NETWORK_STMT_NIL_BEAD_H
#define STATEMENTS_NETWORK_STMT_NIL_BEAD_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_nil_unpack_handler(VMContext *vm, LexerContext *lex);
void     stmt_nil_unpack_register(void);

#endif // STATEMENTS_NETWORK_STMT_NIL_BEAD_H

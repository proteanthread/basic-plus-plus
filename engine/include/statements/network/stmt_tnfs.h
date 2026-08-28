// FILENAME: stmt_tnfs.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_tnfs.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Declares TNFS statements for mounting and managing remote network shares.
//
// ---- Includes ----

#ifndef STATEMENTS_NETWORK_STMT_TNFS_H
#define STATEMENTS_NETWORK_STMT_TNFS_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_tnfs_handler(VMContext *vm, LexerContext *lex);
void stmt_tnfs_register(void);

#endif // STATEMENTS_NETWORK_STMT_TNFS_H

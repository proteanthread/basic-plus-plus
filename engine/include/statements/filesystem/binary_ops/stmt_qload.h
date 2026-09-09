// FILENAME: stmt_qload.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c, exec_dispatch.c)
// NEEDS: libengine (vm.h, lexer.h)
// Declares QLOAD statement interface in BASIC++.

#ifndef STMT_QLOAD_H
#define STMT_QLOAD_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_qload_handler(VMContext *vm, LexerContext *lex);
void stmt_qload_register(void);

#endif // STMT_QLOAD_H

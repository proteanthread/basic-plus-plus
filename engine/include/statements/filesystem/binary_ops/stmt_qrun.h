// FILENAME: stmt_qrun.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c, exec_dispatch.c)
// NEEDS: libengine (vm.h, lexer.h)
// Declares QRUN statement interface in BASIC++.

#ifndef STATEMENTS_FILESYSTEM_BINARY_OPS_STMT_QRUN_H
#define STATEMENTS_FILESYSTEM_BINARY_OPS_STMT_QRUN_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_qrun_handler(VMContext *vm, LexerContext *lex);
void stmt_qrun_register(void);

#endif // STATEMENTS_FILESYSTEM_BINARY_OPS_STMT_QRUN_H

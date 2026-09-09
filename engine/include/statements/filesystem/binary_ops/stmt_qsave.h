// FILENAME: stmt_qsave.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c, exec_dispatch.c)
// NEEDS: libengine (vm.h, lexer.h)
// Declares QSAVE statement interface in BASIC++.

#ifndef STMT_QSAVE_H
#define STMT_QSAVE_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_qsave_handler(VMContext *vm, LexerContext *lex);
void stmt_qsave_register(void);

#endif // STMT_QSAVE_H

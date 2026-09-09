// FILENAME: stmt_revert.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, common_reg_stmts.c)
// NEEDS: libengine (lexer.h, vm.h)
// Provides interface definitions for the REVERT statement in BASIC++.

#ifndef STATEMENTS_PROGRAM_STMT_REVERT_H
#define STATEMENTS_PROGRAM_STMT_REVERT_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_revert_handler(VMContext *vm, LexerContext *lex);
BValue   func_revert_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void     stmt_revert_register(void);

#endif // STATEMENTS_PROGRAM_STMT_REVERT_H

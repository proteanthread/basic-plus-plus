// FILENAME: stmt_stack.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c, common_reg_stmts.c
// NEEDS: types/types.h, vm/vm.h, lexer/lexer.h
// Implements runtime statements for HP-based STACK operations (STACK POP, STACK EXPORT, STACK DUMP, STACK CLEAR).

#ifndef ENGINE_STATEMENTS_SYSTEM_STMT_STACK_H
#define ENGINE_STATEMENTS_SYSTEM_STMT_STACK_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Handler for STACK statements
BppError stmt_stack_handler(VMContext *vm, LexerContext *lex);

// Registration
void stmt_stack_register(void);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_STATEMENTS_SYSTEM_STMT_STACK_H

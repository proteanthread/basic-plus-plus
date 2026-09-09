// FILENAME: stmt_send.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// Declarations for Point-to-Point Messaging Statements (SEND, MSGSEND).
//
// ---- Includes ----

#ifndef STMT_SEND_H
#define STMT_SEND_H

#include "types/types.h"
#include "lexer/lexer.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_send_register(void);
BppError stmt_send_handler(VMContext *vm, LexerContext *lex);
BppError stmt_msgsend_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STMT_SEND_H

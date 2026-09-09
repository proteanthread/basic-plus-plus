// FILENAME: stmt_unsubscribe.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// Declarations for Topic Unsubscription Statement (UNSUBSCRIBE).
//
// ---- Includes ----

#ifndef STMT_UNSUBSCRIBE_H
#define STMT_UNSUBSCRIBE_H

#include "types/types.h"
#include "lexer/lexer.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_unsubscribe_register(void);
BppError stmt_unsubscribe_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STMT_UNSUBSCRIBE_H

// FILENAME: stmt_signal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// Declarations for Signal Emission and Trapping Statements (RAISE SIGNAL, RAISESIGNAL).
//
// ---- Includes ----

#ifndef STMT_SIGNAL_H
#define STMT_SIGNAL_H

#include "types/types.h"
#include "lexer/lexer.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_signal_register(void);
BppError stmt_raise_handler(VMContext *vm, LexerContext *lex);
BppError stmt_raisesignal_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STMT_SIGNAL_H

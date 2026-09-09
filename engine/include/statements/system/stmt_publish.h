// FILENAME: stmt_publish.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// Declarations for Publish / Subscribe Broadcast Statements (PUBLISH, PUBSUB).
//
// ---- Includes ----

#ifndef STMT_PUBLISH_H
#define STMT_PUBLISH_H

#include "types/types.h"
#include "lexer/lexer.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_publish_register(void);
BppError stmt_publish_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pubsub_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STMT_PUBLISH_H

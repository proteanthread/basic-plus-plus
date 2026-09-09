// FILENAME: stmt_user_ipc.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h)
// Header definitions for Inter-User Messaging statements (WALL, MESG, WRITE, TALK, LOGGER).
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_STMT_USER_IPC_H
#define STATEMENTS_SYSTEM_STMT_USER_IPC_H

#include "types/types.h"
#include "types/errors.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void     stmt_user_ipc_register(void);
BppError stmt_wall_handler(VMContext *ctx, LexerContext *lx);
BppError stmt_mesg_handler(VMContext *ctx, LexerContext *lx);
BppError stmt_write_user_handler(VMContext *ctx, LexerContext *lx);
BppError stmt_talk_handler(VMContext *ctx, LexerContext *lx);
BppError stmt_logger_handler(VMContext *ctx, LexerContext *lx);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_SYSTEM_STMT_USER_IPC_H

// FILENAME: func_receive.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_funcs.c
// Declarations for Point-to-Point Messaging Functions (RECEIVE$, MSGRECV$).
//
// ---- Includes ----

#ifndef FUNC_RECEIVE_H
#define FUNC_RECEIVE_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_receive_register(void);
BValue func_receive_eval(VMContext *vm, const char *name, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // FUNC_RECEIVE_H

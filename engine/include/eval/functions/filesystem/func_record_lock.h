// FILENAME: func_record_lock.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c, func_record_lock.c)
// NEEDS: libkernel (types.h)
// Provides runtime function prototypes for READU, WRITEU, RELEASE, and LOCKED.
//
// ---- Includes ----

#ifndef FUNC_RECORD_LOCK_H
#define FUNC_RECORD_LOCK_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_record_lock_register(void);
BValue func_readu_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_writeu_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_release_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_locked_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // FUNC_RECORD_LOCK_H

// FILENAME: func_isam_fn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c, func_isam_fn.c)
// NEEDS: libkernel (types.h)
// Provides runtime function prototypes for KEY$, KEYCOUNT, and ISAM.
//
// ---- Includes ----

#ifndef FUNC_ISAM_FN_H
#define FUNC_ISAM_FN_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_isam_fn_register(void);
BValue func_key_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_keycount_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_isam_check_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // FUNC_ISAM_FN_H

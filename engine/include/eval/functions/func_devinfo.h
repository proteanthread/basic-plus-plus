// FILENAME: func_devinfo.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, sys_fn.c
// NEEDS: libkernel (types.h, errors.h)
// Header for DEVINFO$ and DEVCAPS introspection functions.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_FUNC_DEVINFO_H
#define EVAL_FUNCTIONS_FUNC_DEVINFO_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void   func_devinfo_register(void);
BValue func_devinfo_eval(VMContext *vm, const char *name, int arg_count, BValue *args, BppError *err);
BValue func_devcaps_eval(VMContext *vm, const char *name, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // EVAL_FUNCTIONS_FUNC_DEVINFO_H

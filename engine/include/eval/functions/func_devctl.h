// FILENAME: func_devctl.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, sys_fn.c
// NEEDS: libkernel (types.h, errors.h)
// Header definitions for DEVCTL, DEVCTL$, and MESG built-in functions.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_FUNC_DEVCTL_H
#define EVAL_FUNCTIONS_FUNC_DEVCTL_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

BValue func_devctl_eval(VMContext *ctx, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_devctl_str_eval(VMContext *ctx, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_mesg_eval(VMContext *ctx, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // EVAL_FUNCTIONS_FUNC_DEVCTL_H

// FILENAME: func_overlay.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, func_overlay.c)
// NEEDS: libengine (eval_internal.h), libkernel (types.h)
// Header for the OVERLAY$ string overwriting function in BASIC++.

#ifndef FUNC_OVERLAY_H
#define FUNC_OVERLAY_H

#include "types/types.h"
#include "eval/eval_internal.h"

void func_overlay_register(void);
BValue func_overlay_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_OVERLAY_H

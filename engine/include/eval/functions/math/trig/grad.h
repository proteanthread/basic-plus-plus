// FILENAME: grad.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, grad.c)
// NEEDS: libcore (types.h)
// Declares the GRAD function converting angle from degrees to grads.
//
// ---- Includes ----

#ifndef BPP_GRAD_H
#define BPP_GRAD_H

#include "types/errors.h"
#include "vm/vm.h"
#include "eval/eval.h"

void func_grad_register(void);
BValue func_grad_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // BPP_GRAD_H

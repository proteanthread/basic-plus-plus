// FILENAME: func_tim.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c, dispatch_check.c)
// NEEDS: libkernel (bvalue.h, error.h, vm_types.h)
// Provides public declarations for HP 2000/3000 TIM(n) built-in function in BASIC++.

#ifndef FUNC_TIM_H
#define FUNC_TIM_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

void func_tim_register(void);
BValue func_tim_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_TIM_H

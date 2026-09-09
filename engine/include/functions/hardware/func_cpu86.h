// FILENAME: func_cpu86.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: types/types.h, vm/vm.h
// Provides definitions for the freestanding CPU / CPU86 function in BASIC++.

#ifndef ENGINE_FUNCTIONS_HARDWARE_FUNC_CPU86_H
#define ENGINE_FUNCTIONS_HARDWARE_FUNC_CPU86_H

#include "types/types.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function evaluator for CPU(op$, ...) / CPU86(op$, ...)
BValue func_cpu86_eval(BValue *args, int arg_count, void *rt);

// Registration
void func_cpu86_register(void);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_FUNCTIONS_HARDWARE_FUNC_CPU86_H

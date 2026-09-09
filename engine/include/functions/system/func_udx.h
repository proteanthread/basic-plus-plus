// FILENAME: func_udx.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: types/types.h, vm/vm.h
// Provides definitions for freestanding UDX, FIFO, LIFO functions in BASIC++.

#ifndef ENGINE_FUNCTIONS_SYSTEM_FUNC_UDX_H
#define ENGINE_FUNCTIONS_SYSTEM_FUNC_UDX_H

#include "types/types.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function evaluators
BValue func_udx_eval(BValue *args, int arg_count, void *rt);
BValue func_fifo_eval(BValue *args, int arg_count, void *rt);
BValue func_lifo_eval(BValue *args, int arg_count, void *rt);

// Registration
void func_udx_register(void);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_FUNCTIONS_SYSTEM_FUNC_UDX_H

// FILENAME: func_bios.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: types/types.h, vm/vm.h
// Provides definitions for the freestanding BIOS function in BASIC++.

#ifndef ENGINE_FUNCTIONS_HARDWARE_FUNC_BIOS_H
#define ENGINE_FUNCTIONS_HARDWARE_FUNC_BIOS_H

#include "types/types.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function evaluator for BIOS(int_num, ...) / BIOS(op$, ...)
BValue func_bios_eval(BValue *args, int arg_count, void *rt);

// Registration
void func_bios_register(void);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_FUNCTIONS_HARDWARE_FUNC_BIOS_H

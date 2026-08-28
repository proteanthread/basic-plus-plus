// FILENAME: moddir.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, moddir.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the MODDIR built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_SYSTEM_MODDIR_H
#define EVAL_FUNCTIONS_SYSTEM_MODDIR_H

#include "types/types.h"
#include "vm/vm.h"

void func_moddir_register(void);
BValue func_moddir_eval(VMContext *vm, const char *name, int argc, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_SYSTEM_MODDIR_H

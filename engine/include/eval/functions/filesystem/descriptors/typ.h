// FILENAME: typ.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, typ.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the TYP built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_FILESYSTEM_TYP_H
#define EVAL_FUNCTIONS_FILESYSTEM_TYP_H

#include "types/types.h"
#include "vm/vm.h"

void func_typ_register(void);
BValue func_typ_eval(VMContext *vm, const char *name, int argc, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_FILESYSTEM_TYP_H

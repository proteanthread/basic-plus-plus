// FILENAME: tek.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, tek.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the TEK built-in function in BASIC++.
//
// ---- Includes ----

#ifndef FUNC_TEK_H
#define FUNC_TEK_H

#include "types/types.h"
#include "vm/vm.h"

void   func_tek_register(void);
BValue func_tek_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_vec_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_TEK_H

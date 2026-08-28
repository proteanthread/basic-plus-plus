// FILENAME: ath.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ath.c, string_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the ATH built-in function in BASIC++.
//
// ---- Includes ----

#ifndef FUNC_ATH_H
#define FUNC_ATH_H

#include "types/types.h"
#include "vm/vm.h"

void   func_ath_register(void);
BValue func_hta_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_ath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_ATH_H

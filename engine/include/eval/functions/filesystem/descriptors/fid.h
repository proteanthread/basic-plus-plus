// FILENAME: fid.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (fid.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the FID built-in function in BASIC++.
//
// ---- Includes ----

#ifndef FUNC_FID_H
#define FUNC_FID_H

#include "types/types.h"
#include "vm/vm.h"

void   func_fid_register(void);
BValue func_fid_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_fin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_FID_H

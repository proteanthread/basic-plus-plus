// FILENAME: mbf.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, mbf.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the MBF built-in function in BASIC++.
//
// ---- Includes ----

#ifndef MBF_H
#define MBF_H

#include "types/types.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

BValue func_cvsmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_cvdmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_mksmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_mkdmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

void func_mbf_register(void);

#ifdef __cplusplus
}
#endif

#endif // MBF_H

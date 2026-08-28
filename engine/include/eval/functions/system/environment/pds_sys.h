// FILENAME: pds_sys.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, pds_sys.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Provides runtime implementation for the PDS_SYS built-in function in BASIC++.
//
// ---- Includes ----

#ifndef PDS_SYS_H
#define PDS_SYS_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

BValue func_dir_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_curdir_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_setmem_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_sseg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

void func_pds_sys_register(void);

#ifdef __cplusplus
}
#endif

#endif // PDS_SYS_H

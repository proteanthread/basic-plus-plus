// FILENAME: vbdos_filebox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, vbdos_filebox.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VBDOS_FILEBOX built-in function in BASIC++.
//
// ---- Includes ----

#ifndef VBDOS_FILEBOX_H
#define VBDOS_FILEBOX_H

#include "types/types.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

BValue func_fileopenbox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_filesavebox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

void func_vbdos_filebox_register(void);

#ifdef __cplusplus
}
#endif

#endif // VBDOS_FILEBOX_H

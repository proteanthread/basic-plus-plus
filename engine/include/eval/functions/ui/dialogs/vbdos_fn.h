// FILENAME: vbdos_fn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, vbdos_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VBDOS_FN built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_UI_VBDOS_FN_H
#define EVAL_FUNCTIONS_UI_VBDOS_FN_H

#include "types/types.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void   func_vbdos_register(void);
BValue func_msgbox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_inputbox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_doevents_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // EVAL_FUNCTIONS_UI_VBDOS_FN_H

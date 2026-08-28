// FILENAME: command_fn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libcore (iot_main.c)
// NEEDED BY: libengine (command_fn.c, conversion_fn.c, eval_expr_internal.h)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the COMMAND_FN built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_SYSTEM_COMMAND_FN_H
#define EVAL_FUNCTIONS_SYSTEM_COMMAND_FN_H

#include "types/types.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_command_register(void);
void runtime_set_command_line(const char *cmd);
const char *runtime_get_command_line(void);

BValue func_command_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // EVAL_FUNCTIONS_SYSTEM_COMMAND_FN_H

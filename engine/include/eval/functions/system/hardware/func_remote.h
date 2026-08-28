// FILENAME: func_remote.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_remote.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides REMOTE.EVAL$() and IOT.RPC$() prototypes.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_REMOTE_H
#define EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_REMOTE_H

#include "vm/vm.h"
#include "types/types.h"

BValue func_remote_eval(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_iot_rpc(VMContext *vm, int argc, BValue *argv, BppError *err);

#endif // EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_REMOTE_H

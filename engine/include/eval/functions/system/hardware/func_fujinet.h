// FILENAME: func_fujinet.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_fujinet.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Declares FUJI.STATUS$(), FUJI.SSID$(), FUJI.IP$(), and FUJI.JSON.GET$() functions.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_HARDWARE_FUNC_FUJINET_H
#define EVAL_FUNCTIONS_HARDWARE_FUNC_FUJINET_H

#include "vm/vm.h"
#include "types/types.h"

BValue func_fuji_status(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_fuji_ssid(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_fuji_ip(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_fuji_json_get(VMContext *vm, int argc, BValue *argv, BppError *err);

#endif // EVAL_FUNCTIONS_HARDWARE_FUNC_FUJINET_H

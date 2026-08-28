// FILENAME: func_remote.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (nil_transport.h, nil_transport.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (func_remote.h, string.c)
// Evaluates REMOTE.EVAL$() and IOT.RPC$() functions.
//
// ---- Includes ----

#include "eval/functions/system/hardware/func_remote.h"
#include "runtime/nil_transport.h"
#include "runtime/strings.h"

#include <stdlib.h>
#include <string.h>

BValue func_remote_eval(VMContext *vm, int argc, BValue *argv, BppError *err) {
    if (argc < 2) {
        err->code = 13;
        err->message = "REMOTE.EVAL$ expects target$ and expr$";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    const char *target = (argv[0].type == VAL_STRING && argv[0].as.string) ? str_data(argv[0].as.string) : "";
    const char *expr = (argv[1].type == VAL_STRING && argv[1].as.string) ? str_data(argv[1].as.string) : "";

    nil_transport_send(vm, target, (const uint8_t *)expr, strlen(expr));

    // Simulated / RPC return response
    const char *resp = "42";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), resp, strlen(resp))};
}

BValue func_iot_rpc(VMContext *vm, int argc, BValue *argv, BppError *err) {
    if (argc < 2) {
        err->code = 13;
        err->message = "IOT.RPC$ expects target$ and func_call$";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    const char *target = (argv[0].type == VAL_STRING && argv[0].as.string) ? str_data(argv[0].as.string) : "";
    const char *call_str = (argv[1].type == VAL_STRING && argv[1].as.string) ? str_data(argv[1].as.string) : "";

    nil_transport_send(vm, target, (const uint8_t *)call_str, strlen(call_str));

    const char *resp = "RPC_SUCCESS";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), resp, strlen(resp))};
}

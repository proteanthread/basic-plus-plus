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
#include "runtime/language_descriptor.h"

#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_remote_eval_desc = {
    .name = "REMOTE.EVAL$", .category = "Hardware & Network", .syntax = "REMOTE.EVAL$(target$, expr$)",
    .description = "Sends an expression string to a remote node for evaluation and returns the result string.",
    .error_summary = "Error 13 (Type mismatch) on missing or invalid arguments", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_iot_rpc_desc = {
    .name = "IOT.RPC$", .category = "Hardware & Network", .syntax = "IOT.RPC$(target$, func_call$)",
    .description = "Executes a remote procedure call on an IoT target node and returns the response string.",
    .error_summary = "Error 13 (Type mismatch) on missing or invalid arguments", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};

void func_remote_register(void) {
    lang_desc_register(&g_remote_eval_desc);
    lang_desc_register(&g_iot_rpc_desc);
}

BValue func_remote_eval(VMContext *vm, int argc, BValue *argv, BppError *err) {
    if (argc < 2) {
        err->code = 13;
        err->message = "REMOTE.EVAL$ expects target$ and expr$";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    const char *target = (argv[0].type == VAL_STRING && argv[0].as.string) ? str_data(argv[0].as.string) : "";
    const char *expr = (argv[1].type == VAL_STRING && argv[1].as.string) ? str_data(argv[1].as.string) : "";

    nil_transport_send(vm, target, (const uint8_t *)expr, runtime_strlen(expr));

    // Simulated / RPC return response
    const char *resp = "42";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), resp, runtime_strlen(resp))};
}

BValue func_iot_rpc(VMContext *vm, int argc, BValue *argv, BppError *err) {
    if (argc < 2) {
        err->code = 13;
        err->message = "IOT.RPC$ expects target$ and func_call$";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    const char *target = (argv[0].type == VAL_STRING && argv[0].as.string) ? str_data(argv[0].as.string) : "";
    const char *call_str = (argv[1].type == VAL_STRING && argv[1].as.string) ? str_data(argv[1].as.string) : "";

    nil_transport_send(vm, target, (const uint8_t *)call_str, runtime_strlen(call_str));

    const char *resp = "RPC_SUCCESS";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), resp, runtime_strlen(resp))};
}

// FILENAME: func_receive.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_funcs.c
// NEEDS: libkernel (types.h, errors.h, msg_broker.h, dev_user.h), libcore (strings.h, language_descriptor.h)
// Implementation for Point-to-Point Messaging Functions (RECEIVE$, MSGRECV$).
//
// ---- Includes ----

#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "eval/functions/system/ipc/func_receive.h"
#include "device/msg_broker.h"
#include "device/dev_user.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_receive_desc = {
    .name = "RECEIVE$",
    .category = "Communications & IPC",
    .syntax = "RECEIVE$(source$ [, timeout_ms])",
    .description = "Pulls and returns the next pending message from the specified endpoint, channel, or user mailbox queue.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_msgrecv_desc = {
    .name = "MSGRECV$",
    .category = "Communications & IPC",
    .syntax = "MSGRECV$(source$ [, timeout_ms])",
    .description = "Alias for RECEIVE$. Pulls next message from a named endpoint or user queue.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_receive_register(void) {
    lang_desc_register(&g_receive_desc);
    lang_desc_register(&g_msgrecv_desc);
}

BValue func_receive_eval(VMContext *vm, const char *name, int arg_count, BValue *args, BppError *err) {
    (void)name;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (!vm || !err || arg_count < 1 || !args) {
        if (err) {
            err->code = ERR_ILLEGAL_FUNCTION_CALL;
            err->message = "RECEIVE$ requires at least 1 argument";
        }
        return res;
    }

    if (args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *source_s = str_data(args[0].as.string);
    int timeout_ms = 0;
    if (arg_count >= 2) {
        if (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) {
            timeout_ms = (int)args[1].as.number;
        }
    }

    char buf[MSG_PAYLOAD_MAX_LEN];
    buf[0] = '\0';
    bool ok = false;

    // Route based on source prefix
    if (runtime_strncasecmp(source_s, "USER:", 5) == 0 || runtime_strncasecmp(source_s, "USR:", 4) == 0) {
        const char *uname = (source_s[3] == ':') ? source_s + 4 : source_s + 5;
        char sender[64];
        ok = dev_user_fetch_message(uname, sender, sizeof(sender), buf, sizeof(buf));
    } else if (runtime_strncasecmp(source_s, "BUS:", 4) == 0) {
        const char *tname = source_s + 4;
        ok = msg_broker_poll_topic(tname, buf, sizeof(buf));
    } else {
        const char *ep_name = source_s;
        if (runtime_strncasecmp(ep_name, "IPC:", 4) == 0) ep_name += 4;
        ok = msg_broker_recv_ipc(ep_name, buf, sizeof(buf), timeout_ms);
    }

    if (ok && buf[0]) {
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }

    return res;
}

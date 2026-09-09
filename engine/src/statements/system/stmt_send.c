// FILENAME: stmt_send.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, msg_broker.h, dev_user.h), libcore (eval.h, strings.h, language_descriptor.h)
// Implementation for Point-to-Point Messaging Statements (SEND, MSGSEND).
//
// ---- Includes ----

#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "statements/system/stmt_send.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "device/msg_broker.h"
#include "device/dev_user.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_send_desc = {
    .name = "SEND",
    .category = "Communications & IPC",
    .syntax = "SEND target$, payload$",
    .description = "Sends a point-to-point message to the specified endpoint, channel, or user mailbox queue.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_msgsend_desc = {
    .name = "MSGSEND",
    .category = "Communications & IPC",
    .syntax = "MSGSEND target$, payload$",
    .description = "Alias for SEND. Dispatches a message to a named endpoint or user queue.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_send_register(void) {
    lang_desc_register(&g_send_desc);
    lang_desc_register(&g_msgsend_desc);
}

BppError stmt_send_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in SEND";
        return err;
    }

    // 1. Evaluate target expression (endpoint name / user name / channel)
    BValue target_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (target_val.type != VAL_STRING) {
        if (target_val.type == VAL_STRING && target_val.as.string) {
            str_release(vm_get_str(vm), target_val.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    // 2. Consume comma
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    } else {
        if (target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
        err.code = ERR_SYNTAX;
        err.message = "Expected comma between target and payload in SEND";
        return err;
    }

    // 3. Evaluate payload expression
    BValue payload_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        if (target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
        return err;
    }
    if (payload_val.type != VAL_STRING) {
        if (target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
        if (payload_val.type == VAL_STRING && payload_val.as.string) {
            str_release(vm_get_str(vm), payload_val.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *target_s = target_val.as.string ? str_data(target_val.as.string) : "";
    const char *payload_s = payload_val.as.string ? str_data(payload_val.as.string) : "";
    size_t payload_len = payload_val.as.string ? str_len(payload_val.as.string) : 0;

    // Route based on target prefix: USER: -> user mailbox, BUS: -> topic broker, default -> IPC endpoint queue
    if (runtime_strncasecmp(target_s, "USER:", 5) == 0 || runtime_strncasecmp(target_s, "USR:", 4) == 0) {
        const char *uname = (target_s[3] == ':') ? target_s + 4 : target_s + 5;
        dev_user_post_message(uname, dev_user_current_username(), payload_s);
    } else if (runtime_strncasecmp(target_s, "BUS:", 4) == 0) {
        const char *tname = target_s + 4;
        msg_broker_publish(tname, payload_s, payload_len);
    } else {
        const char *ep_name = target_s;
        if (runtime_strncasecmp(ep_name, "IPC:", 4) == 0) ep_name += 4;
        msg_broker_send_ipc(ep_name, payload_s, payload_len);
    }

    if (payload_val.as.string) str_release(vm_get_str(vm), payload_val.as.string);
    if (target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
    return err;
}

BppError stmt_msgsend_handler(VMContext *vm, LexerContext *lex) {
    return stmt_send_handler(vm, lex);
}

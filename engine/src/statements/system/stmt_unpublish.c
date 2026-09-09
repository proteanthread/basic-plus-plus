// FILENAME: stmt_unpublish.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, msg_broker.h), libcore (eval.h, strings.h, language_descriptor.h)
// Implementation for Topic Unpublish Statement (UNPUBLISH).
//
// ---- Includes ----

#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "statements/system/stmt_unpublish.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "device/msg_broker.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_unpublish_desc = {
    .name = "UNPUBLISH",
    .category = "Communications & PubSub",
    .syntax = "UNPUBLISH topic$ [, payload$] | UNPUBLISH ALL",
    .description = "Unpublishes a topic, withdraws a queued message payload, or resets all active pubsub topics.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_unpublish_register(void) {
    lang_desc_register(&g_unpublish_desc);
}

BppError stmt_unpublish_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in UNPUBLISH";
        return err;
    }

    BppToken tok = lex_peek(lex);
    // Check for "ALL" or "UNPUBLISH ALL"
    if (tok.length == 3 && runtime_strncasecmp(tok.start, "ALL", 3) == 0) {
        lex_next(lex);
        msg_broker_unpublish_all();
        return err;
    }

    // 1. Evaluate topic expression
    BValue topic_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (topic_val.type != VAL_STRING) {
        if (topic_val.type == VAL_STRING && topic_val.as.string) {
            str_release(vm_get_str(vm), topic_val.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *topic_s = topic_val.as.string ? str_data(topic_val.as.string) : "";
    if (runtime_strncasecmp(topic_s, "BUS:", 4) == 0) topic_s += 4;

    // 2. Check for optional comma and payload expression
    BppToken next_tok = lex_peek(lex);
    if (next_tok.type == TOK_COMMA) {
        lex_next(lex); // Consume comma
        BValue payload_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            if (topic_val.as.string) str_release(vm_get_str(vm), topic_val.as.string);
            return err;
        }
        if (payload_val.type != VAL_STRING) {
            if (topic_val.as.string) str_release(vm_get_str(vm), topic_val.as.string);
            if (payload_val.type == VAL_STRING && payload_val.as.string) {
                str_release(vm_get_str(vm), payload_val.as.string);
            }
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }

        const char *payload_s = payload_val.as.string ? str_data(payload_val.as.string) : "";
        msg_broker_unpublish(topic_s, payload_s);

        if (payload_val.as.string) str_release(vm_get_str(vm), payload_val.as.string);
    } else {
        msg_broker_unpublish(topic_s, NULL);
    }

    if (topic_val.as.string) str_release(vm_get_str(vm), topic_val.as.string);
    return err;
}

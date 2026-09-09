// FILENAME: stmt_publish.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, msg_broker.h), libcore (eval.h, strings.h, language_descriptor.h)
// Implementation for Publish / Subscribe Broadcast Statements (PUBLISH, PUBSUB).
//
// ---- Includes ----

#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "statements/system/stmt_publish.h"
#include "statements/system/stmt_subscribe.h"
#include "statements/system/stmt_unsubscribe.h"
#include "statements/system/stmt_unpublish.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "device/msg_broker.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_publish_desc = {
    .name = "PUBLISH",
    .category = "Communications & PubSub",
    .syntax = "PUBLISH topic$, payload$",
    .description = "Publishes a message payload to all active subscribers and message queues on the specified topic.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_pubsub_desc = {
    .name = "PUBSUB",
    .category = "Communications & PubSub",
    .syntax = "PUBSUB topic$, payload$ | PUBSUB.SUB topic$ | PUBSUB.UNSUB topic$",
    .description = "Universal publish/subscribe statement supporting topic broadcast, subscription, and unsubscription.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_publish_register(void) {
    lang_desc_register(&g_publish_desc);
    lang_desc_register(&g_pubsub_desc);
}

BppError stmt_publish_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in PUBLISH";
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

    // 2. Consume comma
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    } else {
        if (topic_val.as.string) str_release(vm_get_str(vm), topic_val.as.string);
        err.code = ERR_SYNTAX;
        err.message = "Expected comma between topic and payload in PUBLISH";
        return err;
    }

    // 3. Evaluate payload expression
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

    const char *topic_s = topic_val.as.string ? str_data(topic_val.as.string) : "";
    const char *payload_s = payload_val.as.string ? str_data(payload_val.as.string) : "";
    size_t payload_len = payload_val.as.string ? str_len(payload_val.as.string) : 0;

    // Strip leading "BUS:" if present
    if (runtime_strncasecmp(topic_s, "BUS:", 4) == 0) topic_s += 4;

    msg_broker_publish(topic_s, payload_s, payload_len);

    if (payload_val.as.string) str_release(vm_get_str(vm), payload_val.as.string);
    if (topic_val.as.string) str_release(vm_get_str(vm), topic_val.as.string);
    return err;
}

static inline bool is_subcommand(BppToken tok, BppKeywordId kw, const char *name) {
    if (kw != KW_NONE && tok.type == TOK_KEYWORD && tok.as.keyword == kw) return true;
    if (tok.type == TOK_KEYWORD && tok.start && name) {
        size_t len = runtime_strlen(name);
        if (tok.length == len && runtime_strncasecmp(tok.start, name, len) == 0) return true;
    }
    if (tok.type == TOK_IDENT && tok.start && name) {
        size_t len = runtime_strlen(name);
        if (tok.length == len && runtime_strncasecmp(tok.start, name, len) == 0) return true;
        if (tok.start[0] == '.' && tok.length == len + 1 && runtime_strncasecmp(tok.start + 1, name, len) == 0) return true;
    }
    return false;
}

BppError stmt_pubsub_handler(VMContext *vm, LexerContext *lex) {
    if (!vm || !lex) {
        BppError err;
        runtime_memset(&err, 0, sizeof(err));
        err.code = 5; err.message = "Null context in PUBSUB";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (is_subcommand(tok, KW_SUBSCRIBE, "SUB") || is_subcommand(tok, KW_SUBSCRIBE, "SUBSCRIBE")) {
        lex_next(lex);
        return stmt_subscribe_handler(vm, lex);
    } else if (is_subcommand(tok, KW_UNSUBSCRIBE, "UNSUB") || is_subcommand(tok, KW_UNSUBSCRIBE, "UNSUBSCRIBE")) {
        lex_next(lex);
        return stmt_unsubscribe_handler(vm, lex);
    } else if (is_subcommand(tok, KW_UNPUBLISH, "UNPUB") || is_subcommand(tok, KW_UNPUBLISH, "UNPUBLISH")) {
        lex_next(lex);
        return stmt_unpublish_handler(vm, lex);
    } else if (is_subcommand(tok, KW_PUBLISH, "PUB") || is_subcommand(tok, KW_PUBLISH, "PUBLISH")) {
        lex_next(lex);
        return stmt_publish_handler(vm, lex);
    }

    // Default: PUBSUB topic$, payload$
    return stmt_publish_handler(vm, lex);
}

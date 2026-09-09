// FILENAME: stmt_signal.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, msg_broker.h), libcore (eval.h, strings.h, language_descriptor.h)
// Implementation for Signal Emission and Trapping Statements (RAISE SIGNAL, RAISESIGNAL).
//
// ---- Includes ----

#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "statements/system/stmt_signal.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "device/msg_broker.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_raise_desc = {
    .name = "RAISE",
    .category = "Communications & Signaling",
    .syntax = "RAISE [SIGNAL] topic$, payload$",
    .description = "Emits a synchronous or asynchronous signal event to active ON SIGNAL trap handlers and topic subscribers.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_raisesignal_desc = {
    .name = "RAISESIGNAL",
    .category = "Communications & Signaling",
    .syntax = "RAISESIGNAL topic$, payload$",
    .description = "Emits a signal event to active ON SIGNAL trap handlers and subscribers.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_signal_register(void) {
    lang_desc_register(&g_raise_desc);
    lang_desc_register(&g_raisesignal_desc);
}

BppError stmt_raise_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in RAISE";
        return err;
    }

    BppToken tok = lex_peek(lex);
    // Optional keyword/ident "SIGNAL"
    if (tok_is_keyword(tok, KW_SIGNAL, "SIGNAL")) {
        lex_next(lex); // consume SIGNAL
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
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    } else {
        if (topic_val.as.string) str_release(vm_get_str(vm), topic_val.as.string);
        err.code = ERR_SYNTAX;
        err.message = "Expected comma between topic and payload in RAISE SIGNAL";
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

    // Publish to topic message broker
    msg_broker_publish(topic_s, payload_s, payload_len);

    // Check if an ON SIGNAL trap is registered for this topic
    bool is_gosub = true;
    int trap_line = msg_broker_get_signal_trap(topic_s, &is_gosub);
    if (trap_line > 0) {
        if (is_gosub) {
            BppLineNumber current = vm_get_current_line(vm);
            const char *resume_pos = lex_get_pos(lex);
            if (!vm_gosub_push(vm, current, resume_pos)) {
                err.code = 14;
                err.message = "GOSUB stack overflow in signal trap";
            }
        }
        if (err.code == 0) {
            vm_jump(vm, (BppLineNumber)trap_line, NULL);
        }
    }

    if (payload_val.as.string) str_release(vm_get_str(vm), payload_val.as.string);
    if (topic_val.as.string) str_release(vm_get_str(vm), topic_val.as.string);
    return err;
}

BppError stmt_raisesignal_handler(VMContext *vm, LexerContext *lex) {
    return stmt_raise_handler(vm, lex);
}

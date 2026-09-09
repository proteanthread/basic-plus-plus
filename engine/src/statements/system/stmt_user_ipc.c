// FILENAME: stmt_user_ipc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h, dev_user.h), libcore (eval.h, strings.h, language_descriptor.h)
// Implementation for Inter-User Messaging statements (WALL, MESG, WRITE, TALK, LOGGER).
//
// ---- Includes ----

#include "statements/system/stmt_user_ipc.h"
#include "device/vdev.h"
#include "device/dev_user.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/math/math.h"

static const LangDesc g_wall_desc = {
    .name = "WALL",
    .category = "System & Communications",
    .syntax = "WALL message$",
    .description = "Broadcasts a system-wide banner message to all active user sessions and terminals.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_mesg_desc = {
    .name = "MESG",
    .category = "System & Communications",
    .syntax = "MESG ON | OFF | Y | N",
    .description = "Controls whether the current session user accepts incoming direct messages.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_logger_desc = {
    .name = "LOGGER",
    .category = "System & Communications",
    .syntax = "LOGGER message$ [, level]",
    .description = "Writes a timestamped system event runtime_log entry to USER0: / syslog sink.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_user_ipc_register(void) {
    lang_desc_register(&g_wall_desc);

    lang_desc_register(&g_mesg_desc);

    lang_desc_register(&g_logger_desc);
}

BppError stmt_wall_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in WALL";
        return err;
    }

    BValue msg_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (msg_val.type != VAL_STRING) {
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *msg_str = msg_val.as.string ? str_data(msg_val.as.string) : "";
    dev_user_broadcast(dev_user_current_username(), msg_str);

    if (msg_val.as.string) str_release(vm_get_str(vm), msg_val.as.string);
    return err;
}

BppError stmt_mesg_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in MESG";
        return err;
    }

    BppToken tok = lex_peek(lex);
    bool allow = true;

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ON) {
        lex_next(lex);
        allow = true;
    } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_OFF) {
        lex_next(lex);
        allow = false;
    } else if (tok.type == TOK_IDENT && (tok.length == 2 && runtime_strncasecmp(tok.start, "ON", 2) == 0)) {
        lex_next(lex);
        allow = true;
    } else if (tok.type == TOK_IDENT && (tok.length == 3 && runtime_strncasecmp(tok.start, "OFF", 3) == 0)) {
        lex_next(lex);
        allow = false;
    } else if (tok.type == TOK_IDENT && (tok.length == 1 && (tok.start[0] == 'Y' || tok.start[0] == 'y'))) {
        lex_next(lex);
        allow = true;
    } else if (tok.type == TOK_IDENT && (tok.length == 1 && (tok.start[0] == 'N' || tok.start[0] == 'n'))) {
        lex_next(lex);
        allow = false;
    } else {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING) {
            const char *s = val.as.string ? str_data(val.as.string) : "";
            if (runtime_strcasecmp(s, "y") == 0 || runtime_strcasecmp(s, "yes") == 0 ||
                runtime_strcasecmp(s, "on") == 0 || runtime_strcasecmp(s, "1") == 0) {
                allow = true;
            } else {
                allow = false;
            }
            if (val.as.string) str_release(vm_get_str(vm), val.as.string);
        } else if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
            allow = (val.as.number != 0.0);
        } else {
            allow = true;
        }
    }

    dev_user_set_mesg(dev_user_current_username(), allow);
    return err;
}

BppError stmt_write_user_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in WRITE";
        return err;
    }

    BValue user_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (user_val.type != VAL_STRING) {
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue msg_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        if (user_val.as.string) str_release(vm_get_str(vm), user_val.as.string);
        return err;
    }
    if (msg_val.type != VAL_STRING) {
        if (user_val.as.string) str_release(vm_get_str(vm), user_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *target_u = user_val.as.string ? str_data(user_val.as.string) : "";
    const char *msg_s = msg_val.as.string ? str_data(msg_val.as.string) : "";
    dev_user_post_message(target_u, dev_user_current_username(), msg_s);

    if (user_val.as.string) str_release(vm_get_str(vm), user_val.as.string);
    if (msg_val.as.string) str_release(vm_get_str(vm), msg_val.as.string);
    return err;
}

BppError stmt_talk_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in TALK";
        return err;
    }

    BValue user_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (user_val.type != VAL_STRING) {
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *target_u = user_val.as.string ? str_data(user_val.as.string) : "";
    char alert[128];
    runtime_snprintf(alert, sizeof(alert), "[TALK connection initiated by %s]", dev_user_current_username());
    dev_user_post_message(target_u, dev_user_current_username(), alert);

    if (user_val.as.string) str_release(vm_get_str(vm), user_val.as.string);
    return err;
}

BppError stmt_logger_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in LOGGER";
        return err;
    }

    BValue msg_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (msg_val.type != VAL_STRING) {
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    int level = 0;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue lvl_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            if (msg_val.as.string) str_release(vm_get_str(vm), msg_val.as.string);
            return err;
        }
        if (lvl_val.type == VAL_NUMBER || lvl_val.type == VAL_INTEGER) {
            level = (int)lvl_val.as.number;
        }
    }

    const char *msg_s = msg_val.as.string ? str_data(msg_val.as.string) : "";
    dev_user_log_event(msg_s, level);

    if (msg_val.as.string) str_release(vm_get_str(vm), msg_val.as.string);
    return err;
}

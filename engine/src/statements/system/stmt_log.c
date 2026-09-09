// FILENAME: stmt_log.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, vdev.h), libcore (eval.h, strings.h, language_descriptor.h, logger.h)
// Implementation for Structured and Flat Logging Statements (LOG.INFO, LOG.WARN, LOG.ERROR, LOG.DEBUG, LOG.TRACE, LOGINFO, etc.).
//
// ---- Includes ----

#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "statements/system/stmt_log.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "debug/logger.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"
#include "runtime/math/math.h"

static const LangDesc g_log_desc = {
    .name = "LOG",
    .category = "System & Logging",
    .syntax = "LOG.INFO msg$ [, tag$] | LOG LEVEL level$ | LOG DUMP [level$] | LOG CLEAR",
    .description = "Emits structured runtime_log messages across multiple severity levels or manages the circular runtime_log ring buffer.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_loginfo_desc = {
    .name = "LOGINFO",
    .category = "System & Logging",
    .syntax = "LOGINFO message$ [, tag$]",
    .description = "Emits an INFO-severity runtime_log entry to the active logging sinks and circular ring buffer.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_logwarn_desc = {
    .name = "LOGWARN",
    .category = "System & Logging",
    .syntax = "LOGWARN message$ [, tag$]",
    .description = "Emits a WARN-severity runtime_log entry to the active logging sinks and circular ring buffer.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_logerror_desc = {
    .name = "LOGERROR",
    .category = "System & Logging",
    .syntax = "LOGERROR message$ [, tag$]",
    .description = "Emits an ERROR-severity runtime_log entry to the active logging sinks and circular ring buffer.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_logdebug_desc = {
    .name = "LOGDEBUG",
    .category = "System & Logging",
    .syntax = "LOGDEBUG message$ [, tag$]",
    .description = "Emits a DEBUG-severity runtime_log entry to the active logging sinks and circular ring buffer.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_logtrace_desc = {
    .name = "LOGTRACE",
    .category = "System & Logging",
    .syntax = "LOGTRACE message$ [, tag$]",
    .description = "Emits a TRACE-severity runtime_log entry to the active logging sinks and circular ring buffer.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_log_register(void) {
    lang_desc_register(&g_log_desc);
    lang_desc_register(&g_loginfo_desc);
    lang_desc_register(&g_logwarn_desc);
    lang_desc_register(&g_logerror_desc);
    lang_desc_register(&g_logdebug_desc);
    lang_desc_register(&g_logtrace_desc);
}

// Helper to check for subcommands across keywords, identifiers, and dot-prefixed identifiers
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

// Helper to parse message and optional tag expressions
static BppError parse_and_emit_log(VMContext *vm, LexerContext *lex, BppLogLevel level, const char *default_tag) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in LOG";
        return err;
    }

    BValue msg_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (msg_val.type != VAL_STRING) {
        if (msg_val.type == VAL_STRING && msg_val.as.string) {
            str_release(vm_get_str(vm), msg_val.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *msg_s = msg_val.as.string ? str_data(msg_val.as.string) : "";
    const char *tag_s = default_tag ? default_tag : "SYS";
    BValue tag_val;
    runtime_memset(&tag_val, 0, sizeof(tag_val));
    bool has_tag = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); // consume ','
        tag_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            if (msg_val.as.string) str_release(vm_get_str(vm), msg_val.as.string);
            return err;
        }
        if (tag_val.type == VAL_STRING && tag_val.as.string) {
            tag_s = str_data(tag_val.as.string);
            has_tag = true;
        }
    }

    log_emit(level, tag_s, "%s", msg_s);

    if (has_tag && tag_val.as.string) str_release(vm_get_str(vm), tag_val.as.string);
    if (msg_val.as.string) str_release(vm_get_str(vm), msg_val.as.string);
    return err;
}

static void dump_print_line(const char *line) {
    if (line) {
        vdev_puts(NULL, line);
        vdev_putc(NULL, '\n');
    }
}

// Master handler for LOG statement
BppError stmt_log_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in LOG";
        return err;
    }

    BppToken tok = lex_peek(lex);

    // Check for dot notation: LOG.INFO, LOG.WARN, LOG.ERROR, LOG.DEBUG, LOG.TRACE, LOG.LEVEL, LOG.DUMP, LOG.CLEAR
    if (tok.type == TOK_PERIOD || (tok.type == TOK_IDENT && tok.start && tok.start[0] == '.')) {
        if (tok.type == TOK_PERIOD) {
            lex_next(lex); // consume '.'
            tok = lex_peek(lex);
        }

        if (is_subcommand(tok, KW_INFO, "INFO")) {
            lex_next(lex);
            return parse_and_emit_log(vm, lex, BPP_LOG_INFO, "INFO");
        } else if (is_subcommand(tok, KW_NONE, "WARN")) {
            lex_next(lex);
            return parse_and_emit_log(vm, lex, BPP_LOG_WARN, "WARN");
        } else if (is_subcommand(tok, KW_ERROR, "ERROR")) {
            lex_next(lex);
            return parse_and_emit_log(vm, lex, BPP_LOG_ERROR, "ERROR");
        } else if (is_subcommand(tok, KW_NONE, "DEBUG")) {
            lex_next(lex);
            return parse_and_emit_log(vm, lex, BPP_LOG_DEBUG, "DEBUG");
        } else if (is_subcommand(tok, KW_NONE, "TRACE")) {
            lex_next(lex);
            return parse_and_emit_log(vm, lex, BPP_LOG_TRACE, "TRACE");
        } else if (is_subcommand(tok, KW_LEVEL, "LEVEL")) {
            lex_next(lex);
            BValue lvl_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (lvl_val.type == VAL_STRING && lvl_val.as.string) {
                logger_set_level(logger_level_from_str(str_data(lvl_val.as.string)));
                str_release(vm_get_str(vm), lvl_val.as.string);
            } else if (lvl_val.type == VAL_NUMBER || lvl_val.type == VAL_INTEGER) {
                logger_set_level((BppLogLevel)(int)lvl_val.as.number);
            }
            return err;
        } else if (is_subcommand(tok, KW_NONE, "DUMP")) {
            lex_next(lex);
            BppLogLevel min_lvl = BPP_LOG_TRACE;
            BppToken next_tok = lex_peek(lex);
            if (!tok_is_eol_or_eof(next_tok) && next_tok.type != TOK_BACKSLASH) {
                BValue lvl_val = eval_expression(vm, lex, &err);
                if (err.code == 0) {
                    if (lvl_val.type == VAL_STRING && lvl_val.as.string) {
                        min_lvl = logger_level_from_str(str_data(lvl_val.as.string));
                        str_release(vm_get_str(vm), lvl_val.as.string);
                    } else if (lvl_val.type == VAL_NUMBER || lvl_val.type == VAL_INTEGER) {
                        min_lvl = (BppLogLevel)(int)lvl_val.as.number;
                    }
                }
            }
            logger_ring_dump(min_lvl, dump_print_line);
            return err;
        } else if (is_subcommand(tok, KW_NONE, "CLEAR")) {
            lex_next(lex);
            logger_ring_clear();
            return err;
        }
    }

    // Space-separated commands: LOG LEVEL, LOG DUMP, LOG CLEAR
    if (is_subcommand(tok, KW_LEVEL, "LEVEL")) {
        lex_next(lex);
        BValue lvl_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (lvl_val.type == VAL_STRING && lvl_val.as.string) {
            logger_set_level(logger_level_from_str(str_data(lvl_val.as.string)));
            str_release(vm_get_str(vm), lvl_val.as.string);
        } else if (lvl_val.type == VAL_NUMBER || lvl_val.type == VAL_INTEGER) {
            logger_set_level((BppLogLevel)(int)lvl_val.as.number);
        }
        return err;
    } else if (is_subcommand(tok, KW_NONE, "DUMP")) {
        lex_next(lex);
        BppLogLevel min_lvl = BPP_LOG_TRACE;
        BppToken next_tok = lex_peek(lex);
        if (!tok_is_eol_or_eof(next_tok) && next_tok.type != TOK_BACKSLASH) {
            BValue lvl_val = eval_expression(vm, lex, &err);
            if (err.code == 0) {
                if (lvl_val.type == VAL_STRING && lvl_val.as.string) {
                    min_lvl = logger_level_from_str(str_data(lvl_val.as.string));
                    str_release(vm_get_str(vm), lvl_val.as.string);
                } else if (lvl_val.type == VAL_NUMBER || lvl_val.type == VAL_INTEGER) {
                    min_lvl = (BppLogLevel)(int)lvl_val.as.number;
                }
            }
        }
        logger_ring_dump(min_lvl, dump_print_line);
        return err;
    } else if (is_subcommand(tok, KW_NONE, "CLEAR")) {
        lex_next(lex);
        logger_ring_clear();
        return err;
    }

    // Default: LOG message$ [, tag$] -> emits as INFO
    return parse_and_emit_log(vm, lex, BPP_LOG_INFO, "INFO");
}

BppError stmt_loginfo_handler(VMContext *vm, LexerContext *lex) {
    return parse_and_emit_log(vm, lex, BPP_LOG_INFO, "INFO");
}

BppError stmt_logwarn_handler(VMContext *vm, LexerContext *lex) {
    return parse_and_emit_log(vm, lex, BPP_LOG_WARN, "WARN");
}

BppError stmt_logerror_handler(VMContext *vm, LexerContext *lex) {
    return parse_and_emit_log(vm, lex, BPP_LOG_ERROR, "ERROR");
}

BppError stmt_logdebug_handler(VMContext *vm, LexerContext *lex) {
    return parse_and_emit_log(vm, lex, BPP_LOG_DEBUG, "DEBUG");
}

BppError stmt_logtrace_handler(VMContext *vm, LexerContext *lex) {
    return parse_and_emit_log(vm, lex, BPP_LOG_TRACE, "TRACE");
}

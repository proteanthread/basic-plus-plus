// FILENAME: stmt_pragma.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_stmt.c, exec_dispatch.c
// NEEDS: statements/system/stmt_pragma.h, eval/rpn_mirror.h, core/metadata.h
// NEEDS: core/dialect.h, platform/platform.h, vm/vm.h, reg/reg_hw.h
// Implements dedicated beginning-of-line !! pragma execution and legacy :: directive deprecation.

#include "statements/system/stmt_pragma.h"
#include "eval/rpn_mirror.h"
#include "runtime/metadata.h"
#include "core/dialect.h"
#include "platform/platform.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "vm/vm.h"
#include "reg/reg_hw.h"

// Executes a dedicated beginning-of-line !! pragma
BppError execute_pragma(VMContext *vm, LexerContext *lex, BppToken pragma_tok) {
    BppError err = {0};
    char pragma_name[64];
    int len = (int)(pragma_tok.length < sizeof(pragma_name) - 1 ? pragma_tok.length : sizeof(pragma_name) - 1);
    runtime_memcpy(pragma_name, pragma_tok.as.string, len);
    pragma_name[len] = '\0';

    if (runtime_strcasecmp(pragma_name, "OPTION") == 0) {
        BppToken opt_tok = lex_next(lex);
        if (opt_tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected option name in !!OPTION";
            return err;
        }
        char opt_name[64];
        int opt_len = (int)(opt_tok.length < sizeof(opt_name) - 1 ? opt_tok.length : sizeof(opt_name) - 1);
        runtime_memcpy(opt_name, opt_tok.start, opt_len);
        opt_name[opt_len] = '\0';
        BppMetadataRegistry *reg = vm_get_metadata(vm);
        if (runtime_strcasecmp(opt_name, "STRICT") == 0 || runtime_strcasecmp(opt_name, "EXPLICIT") == 0) {
            if (reg) reg->option_strict = true;
        } else {
            err.code = 2; err.message = "Unsupported option in !!OPTION";
            return err;
        }
    } else if (runtime_strcasecmp(pragma_name, "EVAL_MIRROR") == 0) {
        BppToken state_tok = lex_next(lex);
        if (state_tok.type == TOK_IDENT || state_tok.type == TOK_KEYWORD) {
            if (runtime_strncasecmp(state_tok.start, "ON", 2) == 0) {
                rpn_set_eval_mirror(true);
            } else if (runtime_strncasecmp(state_tok.start, "OFF", 3) == 0) {
                rpn_set_eval_mirror(false);
            } else {
                err.code = 2; err.message = "Expected ON or OFF in !!EVAL_MIRROR";
                return err;
            }
        } else {
            err.code = 2; err.message = "Expected ON or OFF in !!EVAL_MIRROR";
            return err;
        }
    } else if (runtime_strcasecmp(pragma_name, "TELEMETRY") == 0) {
        BppToken state_tok = lex_next(lex);
        if (state_tok.type == TOK_IDENT || state_tok.type == TOK_KEYWORD) {
            if (runtime_strncasecmp(state_tok.start, "ON", 2) == 0) {
                // Telemetry tracing active
            } else if (runtime_strncasecmp(state_tok.start, "OFF", 3) == 0) {
                // Telemetry tracing disabled
            } else {
                err.code = 2; err.message = "Expected ON or OFF in !!TELEMETRY";
                return err;
            }
        } else {
            err.code = 2; err.message = "Expected ON or OFF in !!TELEMETRY";
            return err;
        }
    } else if (runtime_strcasecmp(pragma_name, "INCLUDE") == 0) {
        BppToken val_tok = lex_next(lex);
        if (val_tok.type != TOK_STRING) {
            err.code = 2; err.message = "Expected string argument for !!INCLUDE";
            return err;
        }
        char inc_path[512] = {0};
        size_t in_len = (val_tok.length < sizeof(inc_path) - 1) ? val_tok.length : sizeof(inc_path) - 1;
        if (val_tok.start && in_len > 0) {
            runtime_memcpy(inc_path, val_tok.start, in_len);
            inc_path[in_len] = '\0';
        }
        size_t plen = runtime_strlen(inc_path);
        if (plen >= 2 && inc_path[0] == '"' && inc_path[plen - 1] == '"') {
            runtime_memmove(inc_path, inc_path + 1, plen - 2);
            inc_path[plen - 2] = '\0';
            plen -= 2;
        }
        if (plen >= 2 && ((inc_path[0] == '\'' && inc_path[plen - 1] == '\'') ||
                          (inc_path[0] == '<' && inc_path[plen - 1] == '>'))) {
            runtime_memmove(inc_path, inc_path + 1, plen - 2);
            inc_path[plen - 2] = '\0';
            plen -= 2;
        }

        char alt_path[512];
        void *f = platform_file_open(inc_path, "r");
        if (!f) {
            runtime_snprintf(alt_path, sizeof(alt_path), "engine/include/%s", inc_path);
            f = platform_file_open(alt_path, "r");
        }
        if (!f) {
            runtime_snprintf(alt_path, sizeof(alt_path), "include/%s", inc_path);
            f = platform_file_open(alt_path, "r");
        }
        if (!f) {
            err.code = 53;
            err.message = "File not found in !!INCLUDE pragma";
            return err;
        }

        char line_buf[1024];
        while (platform_file_gets(line_buf, sizeof(line_buf), f)) {
            size_t llen = runtime_strlen(line_buf);
            while (llen > 0 && (line_buf[llen - 1] == '\r' || line_buf[llen - 1] == '\n')) {
                line_buf[--llen] = '\0';
            }
            if (llen == 0) continue;
            BppError line_err = vm_execute_line(vm, line_buf);
            if (line_err.code != 0) {
                platform_file_close(f);
                return line_err;
            }
        }
        platform_file_close(f);
    } else {
        err.code = 2;
        err.message = "Unsupported compiler pragma";
    }
    return err;
}

// Executes or validates a legacy :: compiler directive with deprecation diagnostics
BppError execute_directive(VMContext *vm, LexerContext *lex, BppToken dir_tok) {
    BppError err = {0};
    char dir_name[64];
    int len = (int)(dir_tok.length < sizeof(dir_name) - 1 ? dir_tok.length : sizeof(dir_name) - 1);
    runtime_memcpy(dir_name, dir_tok.as.string, len);
    dir_name[len] = '\0';

    BppDialect *d = vm_get_active_dialect(vm);
    bool is_compat = (d != NULL && d->name != NULL &&
                      platform_strcasecmp(d->name, "STANDARD") != 0 &&
                      platform_strcasecmp(d->name, "DEFAULT") != 0);

    if (!is_compat) {
        err.code = 2;
        static char diag_msg[128];
        runtime_snprintf(diag_msg, sizeof(diag_msg),
                         "Directive ::%s is deprecated and prohibited in standard mode; use !!%s instead",
                         dir_name, dir_name);
        err.message = diag_msg;
        return err;
    }

    // In compatibility mode, execute as pragma
    return execute_pragma(vm, lex, dir_tok);
}

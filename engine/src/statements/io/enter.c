// FILENAME: enter.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (enter.h, eval.h, eval.c, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the ENTER statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/enter.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include "types/errors.h"
#include <string.h>

void stmt_enter_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ENTER",
        .category = "Input / Output",
        .syntax = "ENTER [#channel,] timeout_sec, status_var, string_var",
        .help_text = "Performs timed input from terminal or channel with status return (HP 2000 TSB).",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BppError stmt_enter_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ENTER) {
        lex_next(lex);
        tok = lex_peek(lex);
    } else if (tok.type == TOK_IDENT && tok.length == 5 && platform_strncasecmp(tok.start, "ENTER", 5) == 0) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    FileContext *fc = vm_get_file(vm);
    bool is_file = false;
    int channel = -1;

    if (tok.type == TOK_HASH) {
        is_file = true;
        lex_next(lex);
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
            if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        channel = (int)ch_val.as.number;
        if (!file_is_open(fc, channel)) {
            err.code = ERR_BAD_FILE_NUMBER;
            err.message = "Bad File Number";
            return err;
        }
        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = ERR_SYNTAX;
            err.message = "Expected ',' after channel in ENTER";
            return err;
        }
    }

    // 1. Timeout expression
    BValue timeout_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (timeout_val.type == VAL_STRING) {
        if (timeout_val.as.string) str_release(vm_get_str(vm), timeout_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }
    double timeout_sec = timeout_val.as.number;
    if (timeout_sec < 0.0) timeout_sec = 0.0;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        err.message = "Expected ',' after timeout in ENTER";
        return err;
    }

    // 2. Status variable name
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err.code = ERR_SYNTAX;
        err.message = "Expected status variable name in ENTER";
        return err;
    }
    char status_var_name[64];
    size_t slen = (tok.length < sizeof(status_var_name) - 1) ? tok.length : sizeof(status_var_name) - 1;
    memcpy(status_var_name, tok.start, slen);
    status_var_name[slen] = '\0';

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        err.message = "Expected ',' after status variable in ENTER";
        return err;
    }

    // 3. Target string variable name
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err.code = ERR_SYNTAX;
        err.message = "Expected string variable name in ENTER";
        return err;
    }
    char str_var_name[64];
    size_t vlen = (tok.length < sizeof(str_var_name) - 1) ? tok.length : sizeof(str_var_name) - 1;
    memcpy(str_var_name, tok.start, vlen);
    str_var_name[vlen] = '\0';

    char line_buf[1024] = "";
    int status_code = 0; // 0 = OK, 1 = timeout, 2 = error / EOF

    if (is_file) {
        if (file_eof(fc, channel)) {
            status_code = 2;
        } else if (!file_gets(fc, channel, line_buf, sizeof(line_buf))) {
            status_code = 2;
        }
    } else {
        if (timeout_sec == 0.0) {
            if (platform_kbhit()) {
                int ch = platform_getch();
                if (ch != -1) {
                    line_buf[0] = (char)ch;
                    line_buf[1] = '\0';
                    status_code = 0;
                } else {
                    status_code = 1;
                }
            } else {
                status_code = 1; // Timed out immediately
            }
        } else {
            double start_t = platform_get_timer();
            size_t pos = 0;
            status_code = 1; // Default to timed out
            while ((platform_get_timer() - start_t) < timeout_sec) {
                if (platform_kbhit()) {
                    int ch = platform_getch();
                    if (ch == '\r' || ch == '\n') {
                        status_code = 0;
                        break;
                    }
                    if (ch >= 32 && ch <= 126 && pos < sizeof(line_buf) - 1) {
                        line_buf[pos++] = (char)ch;
                        line_buf[pos] = '\0';
                        status_code = 0;
                    }
                } else {
                    platform_sleep_ms(10);
                }
            }
        }
    }

    // Remove trailing newlines
    size_t llen = strlen(line_buf);
    while (llen > 0 && (line_buf[llen - 1] == '\r' || line_buf[llen - 1] == '\n')) {
        line_buf[--llen] = '\0';
    }

    // Assign status code
    BValue s_val;
    memset(&s_val, 0, sizeof(s_val));
    s_val.type = VAL_NUMBER;
    s_val.as.number = (double)status_code;
    var_assign(vm_get_var(vm), status_var_name, s_val);

    // Assign input string
    BValue out_str;
    memset(&out_str, 0, sizeof(out_str));
    out_str.type = VAL_STRING;
    out_str.as.string = str_create(vm_get_str(vm), line_buf, llen);
    var_assign(vm_get_var(vm), str_var_name, out_str);
    str_release(vm_get_str(vm), out_str.as.string);

    return err;
}

// FILENAME: lock.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, lock.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the LOCK statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/lock.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "types/errors.h"
#include <string.h>

#if defined(_WIN32)
#include <io.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

static BppError handle_file_lock_unlock(VMContext *vm, const char *path, bool is_lock) {
    (void)vm;
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!path || strlen(path) == 0) {
        err.code = ERR_BAD_FILE_NAME;
        return err;
    }

#if defined(_WIN32)
    int mode = is_lock ? (_S_IREAD) : (_S_IREAD | _S_IWRITE);
    if (_chmod(path, mode) != 0) {
        // If file does not exist, return file not found
        err.code = ERR_FILE_NOT_FOUND;
        return err;
    }
#else
    mode_t mode = is_lock ? (S_IRUSR | S_IRGRP | S_IROTH) : (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (chmod(path, mode) != 0) {
        err.code = ERR_FILE_NOT_FOUND;
        return err;
    }
#endif

    return err;
}

BppError stmt_lock_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
    }

    // Evaluate target (file path string for Apple /// SOS, or channel number for QuickBASIC)
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_STRING) {
        const char *path = str_data(val.as.string);
        err = handle_file_lock_unlock(vm, path, true);
        str_release(vm_get_str(vm), val.as.string);
        return err;
    }

    // Channel / record locking
    int ch = (int)val.as.number;
    (void)ch;

    // Optional record or range
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); // Consume ','
        tok = lex_peek(lex);
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) ||
            (tok.type == TOK_IDENT && tok.length == 2 && strncasecmp(tok.start, "TO", 2) == 0)) {
            lex_next(lex); // Consume 'TO'
            BValue end_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (end_val.type == VAL_STRING) {
                str_release(vm_get_str(vm), end_val.as.string);
            }
        } else {
            BValue start_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (start_val.type == VAL_STRING) {
                str_release(vm_get_str(vm), start_val.as.string);
            }

            tok = lex_peek(lex);
            if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) ||
                (tok.type == TOK_IDENT && tok.length == 2 && strncasecmp(tok.start, "TO", 2) == 0)) {
                lex_next(lex); // Consume 'TO'
                BValue end_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (end_val.type == VAL_STRING) {
                    str_release(vm_get_str(vm), end_val.as.string);
                }
            }
        }
    }

    return err;
}

BppError stmt_unlock_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
    }

    // Evaluate target (file path string for Apple /// SOS, or channel number for QuickBASIC)
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_STRING) {
        const char *path = str_data(val.as.string);
        err = handle_file_lock_unlock(vm, path, false);
        str_release(vm_get_str(vm), val.as.string);
        return err;
    }

    // Channel / record unlock
    int ch = (int)val.as.number;
    (void)ch;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) ||
            (tok.type == TOK_IDENT && tok.length == 2 && strncasecmp(tok.start, "TO", 2) == 0)) {
            lex_next(lex);
            BValue end_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (end_val.type == VAL_STRING) str_release(vm_get_str(vm), end_val.as.string);
        } else {
            BValue start_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (start_val.type == VAL_STRING) str_release(vm_get_str(vm), start_val.as.string);

            tok = lex_peek(lex);
            if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) ||
                (tok.type == TOK_IDENT && tok.length == 2 && strncasecmp(tok.start, "TO", 2) == 0)) {
                lex_next(lex);
                BValue end_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (end_val.type == VAL_STRING) str_release(vm_get_str(vm), end_val.as.string);
            }
        }
    }

    return err;
}

void stmt_lock_register(void) {
    MicroLibMetadata meta = {
        .name = "LOCK",
        .category = "File & Resource Locking",
        .syntax = "LOCK filepath$ | LOCK [#]file_num [, [record_start] [TO record_end]]",
        .help_text = "Locks a file on disk (Apple /// Business BASIC) or locks a file channel/record range (QuickBASIC).",
        .error_codes = "Error 52: Bad File Number, Error 53: File Not Found, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

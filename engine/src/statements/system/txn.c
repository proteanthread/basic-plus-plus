// FILENAME: txn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, stmt.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the TXN statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "runtime/file.h"
#include "eval/eval.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

BppError stmt_commit_handler(struct VMContext *vm, struct LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    err = file_txn_commit(vm_get_file(vm));
    return err;
}

BppError stmt_rollback_handler(struct VMContext *vm, struct LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    err = file_txn_rollback(vm_get_file(vm));
    return err;
}

BppError stmt_atomic_handler(struct VMContext *vm, struct LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    file_txn_begin(vm_get_file(vm), 2, false);
    return err;
}

BppError stmt_txn_handler(struct VMContext *vm, struct LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_COMMIT) {
            return stmt_commit_handler(vm, lex);
        }
        if (tok.as.keyword == KW_ROLLBACK) {
            return stmt_rollback_handler(vm, lex);
        }
    }

    char word[64] = "";
    if (tok.length > 0 && tok.length < sizeof(word)) {
        memcpy(word, tok.start, tok.length);
        word[tok.length] = '\0';
    }

    if (platform_strcasecmp(word, "BEGIN") == 0) {
        bool use_file = false;
        BppToken next = lex_peek(lex);
        char next_word[64] = "";
        if (next.length > 0 && next.length < sizeof(next_word)) {
            memcpy(next_word, next.start, next.length);
            next_word[next.length] = '\0';
        }
        if (platform_strcasecmp(next_word, "FILE") == 0) {
            lex_next(lex);
            use_file = true;
        }
        file_txn_begin(vm_get_file(vm), 1, use_file);
    } else if (platform_strcasecmp(word, "COMMIT") == 0) {
        err = file_txn_commit(vm_get_file(vm));
    } else if (platform_strcasecmp(word, "ROLLBACK") == 0) {
        err = file_txn_rollback(vm_get_file(vm));
    } else if (platform_strcasecmp(word, "STATUS") == 0) {
        int status = file_txn_status(vm_get_file(vm));
        int entries = file_txn_entry_count(vm_get_file(vm));
        if (status == 0) {
            vdev_printf(vm_get_vdev(vm), "TXN: INACTIVE\n");
        } else if (status == 1) {
            vdev_printf(vm_get_vdev(vm), "TXN: ACTIVE (%d entries)\n", entries);
        } else if (status == 2) {
            vdev_printf(vm_get_vdev(vm), "TXN: ATOMIC (%d entries)\n", entries);
        }
    } else {
        err.code = 2;
        err.message = "Expected BEGIN, COMMIT, ROLLBACK, or STATUS after TXN";
    }

    return err;
}

void stmt_txn_register(void) {
    static const MicroLibMetadata meta = {
        .name = "TXN",
        .category = "File I/O",
        .syntax = "TXN BEGIN [FILE] | TXN COMMIT | TXN ROLLBACK | TXN STATUS | ATOMIC",
        .help_text = "Provides ACID file transaction management and atomic block rollback capability.",
        .error_codes = "Error 2: Syntax Error, Error 54: Bad File Mode, Error 57: Device I/O Error"
    };
    microlib_register(&meta);
}


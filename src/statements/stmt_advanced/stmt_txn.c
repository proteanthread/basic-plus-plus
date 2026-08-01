/**
 * @file stmt_txn.c
 * @brief Transaction control statement handlers.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements statement handlers for transaction controls:
 *   - TXN BEGIN [FILE]
 *   - TXN COMMIT
 *   - TXN ROLLBACK
 *   - TXN STATUS
 *   - ATOMIC
 *   - COMMIT
 *   - ROLLBACK
 * - Why it exists: Allows grouping of file operations for atomic updates, protecting
 *   database integrity and supporting transactional rollbacks.
 * - Why it works this way: Handlers interface with FileContext APIs to begin, commit,
 *   or rollback recorded journal changes.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Output formatting of TXN STATUS.
 * - What cannot be changed: Core integration with FileContext rollback logs.
 * - What to expect: Changes are recorded in memory or in a temp file during active transaction.
 * - What to do if something breaks: Verify file pointer state during rollback.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Channels are open and seekable for rolling back.
 * - Portability concerns: Portable C17 code, OS-independent file updates.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add savepoints, nested sub-transactions.
 * - How to write external extensions: External virtual devices can query transaction status.
 */

#include "bpp_stmt.h"
#include "bpp_file.h"
#include "bpp_eval.h"
#include "bpp_platform.h"
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

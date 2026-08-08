/**
 * @file txn.c
 * @brief TXN BEGIN, TXN COMMIT, TXN ROLLBACK, ATOMIC, COMMIT, and ROLLBACK transaction statement handlers for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements ACID file transaction control statements:
 * - TXN BEGIN [FILE #channel]: Starts a transactional journaling block on open file channels.
 * - TXN COMMIT / COMMIT: Flushes journaled transaction changes atomically to disk.
 * - TXN ROLLBACK / ROLLBACK: Discards journaled changes and restores original file state.
 * - TXN STATUS: Displays active transaction status and journal state.
 * - ATOMIC statement: Executes a single atomic block statement wrapper.
 *
 * 2. WHY IT EXISTS:
 * Ensures data integrity for file and database operations by providing rollback protection against partial writes or crashes.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Interacts with FileContext transaction journal log buffers to record modified disk blocks during active transactions.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_txn'. Includes "statements/system/txn.h",
 * "runtime/file.h", "eval/eval.h", "platform/platform.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support nested sub-transactions (SAVEPOINT point_name$) and multi-file two-phase commit protocols.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Journal atomicity invariant: ROLLBACK MUST restore original file offset pointers and original sector data without partial state corruption.
 *
 * 8. WHAT TO EXPECT:
 * Manages file transaction state and returns ERR_NONE or ERR_FILE_IO_ERROR on journal failure.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify file channel seek offsets and journal file handle allocation in engine/src/runtime/file.c.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and active FileContext handles.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Portable OS-independent file journaling.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/file.c
 * - engine/src/eval/eval.c
 * Prerequisite Header Files:
 * - engine/include/statements/system/txn.h
 * - engine/include/runtime/file.h
 * - engine/include/vm/vm.h
 */

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


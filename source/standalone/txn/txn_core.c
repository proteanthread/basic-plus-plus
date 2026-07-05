/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: txn_core.c
 * Subsystem: Transactional Journal File History Tracker
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Manages file edits journaling and handles rollback.
 *
 * 2. WHAT TO EXPECT:
 *    Saves original contents, performs undos.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Temporary backup file paths.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Rollback transaction loops.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If file fails to restore, verify locks.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE TRANSACTION JOURNAL CORE
 * File: txn_core.c
 * ===================================================================== */

#include <stdio.h>
#include <string.h>
#include "txn_core.h"

void txn_core_init(TxnJournal *j)
{
    if (j) {
        j->state = TXN_NONE;
        j->count = 0;
        j->auto_rollback = 0;
    }
}

int txn_core_begin(TxnJournal *j, int mode)
{
    if (!j) return -1;
    if (j->state != TXN_NONE) {
        return -1;
    }
    j->state = mode;
    j->count = 0;
    j->auto_rollback = (mode == TXN_ATOMIC) ? 1 : 0;
    return 0;
}

int txn_core_commit(TxnJournal *j)
{
    if (!j) return -1;
    if (j->state == TXN_NONE) {
        return -1;
    }
    j->state = TXN_NONE;
    j->count = 0;
    j->auto_rollback = 0;
    return 0;
}

int txn_core_rollback(TxnJournal *j, const TxnCoreCallbacks *cbs, void *user_data, int line_num)
{
    int i;
    int errors = 0;

    if (!j || !cbs) return -1;
    if (j->state == TXN_NONE) {
        return -1;
    }

    for (i = j->count - 1; i >= 0; i--) {
        TxnEntry *e = &j->entries[i];
        void *fp = cbs->get_fp(user_data, e->chan);

        if (fp == NULL) {
            errors++;
            continue;
        }

        if (e->was_eof) {
            if (cbs->fseek_set(user_data, fp, e->pos) != 0) {
                errors++;
            }
            if (cbs->truncate(user_data, fp, e->pos) != 0) {
                errors++;
            }
        } else {
            if (cbs->fseek_set(user_data, fp, e->pos) != 0) {
                errors++;
                continue;
            }
            if (cbs->fwrite(user_data, fp, e->data, e->len) != e->len) {
                errors++;
            }
            cbs->fflush(user_data, fp);
        }
    }

    j->state = TXN_NONE;
    j->count = 0;
    j->auto_rollback = 0;

    (void)line_num;
    return (errors > 0) ? -1 : 0;
}

int txn_core_journal_write(TxnJournal *j, const TxnCoreCallbacks *cbs, void *user_data, int chan, long pos, int len, int line_num)
{
    TxnEntry *e;
    void *fp;
    long file_size;
    int actual;

    if (!j || !cbs) return -1;

    if (j->state == TXN_NONE) {
        return 0;
    }

    if (j->count >= MAX_TXN_ENTRIES) {
        printf("TXN journal full (%d entries).\n", MAX_TXN_ENTRIES);
        cbs->error_raise_sorry(user_data, line_num);
        return -1;
    }

    if (len > MAX_TXN_DATA) {
        printf("TXN entry too large (%d bytes, max %d).\n", len, MAX_TXN_DATA);
        cbs->error_raise_sorry(user_data, line_num);
        return -1;
    }

    fp = cbs->get_fp(user_data, chan);
    e = &j->entries[j->count];
    e->chan = chan;
    e->pos = pos;
    e->len = len;
    e->was_eof = 0;

    if (fp != NULL) {
        long saved_pos;
        saved_pos = cbs->ftell(user_data, fp);
        cbs->fseek_end(user_data, fp);
        file_size = cbs->ftell(user_data, fp);

        if (pos >= file_size) {
            e->was_eof = 1;
            e->len = 0;
            memset(e->data, 0, (size_t)len);
        } else {
            int save_len = len;
            if (pos + len > file_size) {
                save_len = (int)(file_size - pos);
                e->was_eof = 1;
            }
            cbs->fseek_set(user_data, fp, pos);
            actual = cbs->fread(user_data, fp, e->data, save_len);
            e->len = actual;
        }

        cbs->fseek_set(user_data, fp, saved_pos);
    } else {
        e->was_eof = 1;
        e->len = 0;
    }

    j->count++;
    return 0;
}

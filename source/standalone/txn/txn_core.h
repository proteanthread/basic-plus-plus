/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: txn_core.h
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
 * File: txn_core.h
 * =====================================================================
 * Provides portable journal state structures and callback-driven
 * rollback/write-logging algorithms for atomic file execution.
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_TXN_CORE_H
#define BASICPP_STANDALONE_TXN_CORE_H

#define MAX_TXN_ENTRIES 64
#define MAX_TXN_DATA    1024

#define TXN_NONE     0
#define TXN_ACTIVE   1
#define TXN_ATOMIC   2

typedef struct TxnEntry {
    int chan;
    long pos;
    int len;
    char data[MAX_TXN_DATA];
    int was_eof;
} TxnEntry;

typedef struct TxnJournal {
    int state;
    int count;
    TxnEntry entries[MAX_TXN_ENTRIES];
    int auto_rollback;
} TxnJournal;

typedef struct TxnCoreCallbacks {
    void *(*get_fp)(void *user_data, int chan);
    long (*ftell)(void *user_data, void *fp);
    int (*fseek_set)(void *user_data, void *fp, long offset);
    int (*fseek_end)(void *user_data, void *fp);
    int (*fread)(void *user_data, void *fp, void *buf, int len);
    int (*fwrite)(void *user_data, void *fp, const void *buf, int len);
    void (*fflush)(void *user_data, void *fp);
    int (*truncate)(void *user_data, void *fp, long length);
    void (*error_raise_sorry)(void *user_data, int line_num);
} TxnCoreCallbacks;

void txn_core_init(TxnJournal *j);
int txn_core_begin(TxnJournal *j, int mode);
int txn_core_commit(TxnJournal *j);
int txn_core_rollback(TxnJournal *j, const TxnCoreCallbacks *cbs, void *user_data, int line_num);
int txn_core_journal_write(TxnJournal *j, const TxnCoreCallbacks *cbs, void *user_data, int chan, long pos, int len, int line_num);

#endif // BASICPP_STANDALONE_TXN_CORE_H

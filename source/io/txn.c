/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: txn.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Input/Output operations, screen print layout, file streams (sequential/binary/random), block I/O formatting, and record buffers.
 *
 * 2. WHAT TO EXPECT:
 *    Delegates file operations through the Virtual Device (VDev) mapping layer. Relies on standard ANSI C streams.
 *
 * 3. WHAT CAN BE CHANGED:
 *    File channel limit rules, I/O default buffers, record layout details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Binary packing format definitions (MKI$, MKS$, MKD$), file channel index lookups.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check file channel descriptor leaks (ensure CLOSE #ch is called). Verify binary file alignments and record record size boundaries.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - txn.c
 // ---
 //
 // Transaction / ATOMIC Block Implementation.
 //
 // The journal records original data before each write so it
 // can be restored on rollback. Writes are journaled by the
 // SIO/BIO handlers when a transaction is active.
 //
 // Rollback replays entries in reverse order:
 // 1. Seek to the saved position
 // 2. Write back the original data
 // 3. If was_eof, truncate the file (best-effort)
 //
 // ATOMIC blocks differ from explicit TXN in that they
 // automatically rollback on any error (via txn_on_error).
 //
//
// HOW TO EXTEND:
//   See the preamble comments in related files for
//   customization and extension instructions.
//
// TROUBLESHOOTING:
//   Check error_occurred() after operations that can fail.
//   Use error_raise(ERR_xxx, line_num) for error reporting.
 // ---

#ifndef _WIN32
  #if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 200112L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200112L
  #endif
#endif

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <io.h> // _fileno, _chsize
#elif defined(__unix__) || defined(__APPLE__)
#include <unistd.h> // fileno, ftruncate
#endif
#include "txn.h"
#include "fileio.h"
#include "errors.h"

// The single global journal
static TxnJournal journal;

 // txn_init - Clear the journal and reset state.
void txn_init(void)
{
    journal.state = TXN_NONE;
    journal.count = 0;
    journal.auto_rollback = 0;
}

 // txn_begin - Start a new transaction.
int txn_begin(int mode)
{
    if (journal.state != TXN_NONE) {
        return -1; // already active
    }
    journal.state = mode;
    journal.count = 0;
    journal.auto_rollback = (mode == TXN_ATOMIC) ? 1 : 0;
    return 0;
}

 // txn_commit - Discard the journal (writes become permanent).
int txn_commit(void)
{
    if (journal.state == TXN_NONE) {
        return -1; // nothing to commit
    }
    journal.state = TXN_NONE;
    journal.count = 0;
    journal.auto_rollback = 0;
    return 0;
}

 // txn_rollback - Replay journal in reverse to undo writes.
 //
 // For each entry:
 // 1. Get the file pointer for the channel
 // 2. Seek to the saved position
 // 3. Write back the original data
 // 4. If was_eof, attempt truncation (platform-specific)
 //
 // Errors during rollback are reported but don't stop the
 // rollback process -- we try to undo as much as possible.
int txn_rollback(int line_num)
{
    int i;
    int errors = 0;

    if (journal.state == TXN_NONE) {
        return -1;
    }

    // Replay in reverse order
    for (i = journal.count - 1; i >= 0; i--) {
        TxnEntry *e = &journal.entries[i];
        FILE *fp = fileio_get_fp(e->chan);

        if (fp == NULL) {
            // Channel closed or invalid -- can't undo
            errors++;
            continue;
        }

        if (e->was_eof) {
             // Data was written beyond the original EOF.
             // Best-effort: seek to where EOF was and truncate.
             // This is platform-specific; on systems without
             // ftruncate we just leave the extra bytes.
            if (fseek(fp, e->pos, SEEK_SET) != 0) {
                errors++;
            }
#ifdef _WIN32
            {
                int fd = _fileno(fp);
                _chsize(fd, e->pos);
            }
#elif defined(__unix__) || defined(__APPLE__)
            {
                int fd = fileno(fp);
                if (ftruncate(fd, e->pos) != 0)
                    errors++;
            }
#endif
            // On unsupported platforms, truncation is skipped
        } else {
            // Restore original data
            if (fseek(fp, e->pos, SEEK_SET) != 0) {
                errors++;
                continue;
            }
            if ((int)fwrite(e->data, 1, (size_t)e->len, fp)
                    != e->len) {
                errors++;
            }
            fflush(fp);
        }
    }

    // Clear journal
    journal.state = TXN_NONE;
    journal.count = 0;
    journal.auto_rollback = 0;

    (void)line_num;
    return (errors > 0) ? -1 : 0;
}

 // txn_journal_write - Record original data before a write.
 //
 // If no transaction is active, this is a no-op (returns 0).
 // Called by SIO/BIO write handlers before performing the write.
int txn_journal_write(int chan, long pos, int len, int line_num)
{
    TxnEntry *e;
    FILE *fp;
    long file_size;
    int actual;

    // No-op if no transaction active
    if (journal.state == TXN_NONE) {
        return 0;
    }

    // Check journal capacity
    if (journal.count >= MAX_TXN_ENTRIES) {
        printf("TXN journal full (%d entries).\n",
               MAX_TXN_ENTRIES);
        error_raise(ERR_SORRY, line_num);
        return -1;
    }

    // Clamp length to max data size
    if (len > MAX_TXN_DATA) {
        printf("TXN entry too large (%d bytes, max %d).\n",
               len, MAX_TXN_DATA);
        error_raise(ERR_SORRY, line_num);
        return -1;
    }

    fp = fileio_get_fp(chan);
    e = &journal.entries[journal.count];
    e->chan = chan;
    e->pos = pos;
    e->len = len;
    e->was_eof = 0;

    if (fp != NULL) {
        long saved_pos;
        // Determine file size to check if write extends past EOF
        saved_pos = ftell(fp);
        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);

        if (pos >= file_size) {
            // Writing beyond EOF -- no data to save
            e->was_eof = 1;
            e->len = 0;
            memset(e->data, 0, (size_t)len);
        } else {
            // Read the original data at this position
            int save_len = len;
            if (pos + len > file_size) {
                // Partial overlap with existing data
                save_len = (int)(file_size - pos);
                e->was_eof = 1;
            }
            fseek(fp, pos, SEEK_SET);
            actual = (int)fread(e->data, 1, (size_t)save_len, fp);
            e->len = actual;
        }

        // Restore file position
        fseek(fp, saved_pos, SEEK_SET);
    } else {
        // VDev or closed channel -- mark as no-restore
        e->was_eof = 1;
        e->len = 0;
    }

    journal.count++;
    return 0;
}

 // txn_is_active - Return current transaction state.
int txn_is_active(void)
{
    return journal.state;
}

 // txn_entry_count - Return number of journal entries.
int txn_entry_count(void)
{
    return journal.count;
}

 // txn_on_error - Auto-rollback for ATOMIC blocks.
 //
 // Called by the error handler. If an ATOMIC block is active,
 // rolls back all journaled writes automatically.
void txn_on_error(int line_num)
{
    if (journal.state == TXN_ATOMIC && journal.auto_rollback) {
        txn_rollback(line_num);
    }
}

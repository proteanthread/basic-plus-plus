/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: txn.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Utility or helper code for BASIC++ interpreter.
 *
 * 2. WHAT TO EXPECT:
 *    Executes with low overhead, relying on fixed compile-time limits and memory pools.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Internal helper functions, optimization passes, or local naming adjustments.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Public API structures, parameter contracts, or global type definitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check memory pool margins, look for segmentation faults, and trace parameter values via a debugger.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - txn.h
 // ---
 //
 // Transaction / ATOMIC Block System.
 //
 // PURPOSE:
 // Provides atomicity guarantees for groups of I/O operations.
 // When a transaction is active, all writes are journaled so they
 // can be rolled back if an error occurs or ROLLBACK is called.
 //
 // USAGE:
 //   ATOMIC
 //     BIOWRITE(1, 0, "header")
 //     BIOWRITE(1, 8, "data")
 //   END ATOMIC
 //
 //   TXN BEGIN
 //   BIOWRITE(1, 0, "header")
 //   IF ERR THEN TXN ROLLBACK : GOTO cleanup
 //   TXN COMMIT
 //
 // DESIGN:
 // A static journal of up to MAX_TXN_ENTRIES undo records.
 // Each record stores the channel, position, and original data
 // that was overwritten. On COMMIT, the journal is cleared.
 // On ROLLBACK, entries are replayed in reverse order to
 // restore original file contents.
 //
 // ATOMIC blocks automatically COMMIT on END ATOMIC. If an
 // error occurs during an ATOMIC block, the runtime error
 // handler calls txn_rollback() before propagating the error.
 //
 // Transactions do NOT nest. Starting a new TXN while one is
 // active raises an error. ATOMIC blocks use the same journal.
 //
 // ---

#ifndef BASICPP_TXN_H
#define BASICPP_TXN_H

// Maximum undo entries per transaction
#define MAX_TXN_ENTRIES 64

// Maximum bytes saved per undo entry
#define MAX_TXN_DATA    1024

// Transaction state
#define TXN_NONE     0 // no active transaction
#define TXN_ACTIVE   1 // transaction in progress
#define TXN_ATOMIC   2 // atomic block in progress

 // TxnEntry - Single undo journal entry.
 //
 // Records what data existed at a file position BEFORE a write
 // overwrote it. Used to roll back on error or explicit ROLLBACK.
 //
 // chan:     channel number (1-based)
 // pos:     byte position in file (0-based)
 // len:     number of bytes saved
 // data:    original content at that position
 // was_eof: 1 if the position was beyond EOF (truncation needed)
typedef struct TxnEntry {
    int chan;
    long pos;
    int len;
    char data[MAX_TXN_DATA];
    int was_eof;
} TxnEntry;

 // TxnJournal - The transaction journal.
 //
 // Holds all undo entries for the current transaction.
 // Only one transaction can be active at a time.
typedef struct TxnJournal {
    int state; // TXN_NONE, TXN_ACTIVE, TXN_ATOMIC
    int count; // number of journal entries
    TxnEntry entries[MAX_TXN_ENTRIES];
    int auto_rollback; // 1 = rollback on error (ATOMIC)
} TxnJournal;

 // txn_init - Initialize the transaction system.
 //
 // Clears the journal. Call once at boot.
void txn_init(void);

 // txn_begin - Start a new transaction.
 //
 // mode: TXN_ACTIVE (explicit TXN BEGIN) or
 //       TXN_ATOMIC (ATOMIC block, auto-rollback on error)
 //
 // Returns 0 on success, -1 if a transaction is already active.
int txn_begin(int mode);

 // txn_commit - Commit the current transaction.
 //
 // Discards all journal entries. The writes are permanent.
 // Returns 0 on success, -1 if no transaction is active.
int txn_commit(void);

 // txn_rollback - Roll back the current transaction.
 //
 // Replays journal entries in reverse order, restoring
 // original file contents. Then clears the journal.
 //
 // line_num: BASIC line for error reporting during rollback.
 // Returns 0 on success, -1 on error.
int txn_rollback(int line_num);

 // txn_journal_write - Record a write for potential rollback.
 //
 // Called BEFORE performing a write to save the original data.
 // If no transaction is active, this is a no-op.
 //
 // chan:     channel number
 // pos:     byte position (0-based) where write will occur
 // len:     number of bytes that will be overwritten
 // line_num: BASIC line for error reporting
 //
 // Returns 0 on success (or no-op), -1 on error (journal full).
int txn_journal_write(int chan, long pos, int len, int line_num);

 // txn_is_active - Check if a transaction is active.
 //
 // Returns TXN_NONE, TXN_ACTIVE, or TXN_ATOMIC.
int txn_is_active(void);

 // txn_entry_count - Return number of journal entries.
int txn_entry_count(void);

 // txn_on_error - Called by the error handler when an error
 //                occurs during an ATOMIC block.
 //
 // If auto_rollback is enabled (ATOMIC mode), performs a
 // rollback automatically.
void txn_on_error(int line_num);

#endif // BASICPP_TXN_H

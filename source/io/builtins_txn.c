/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: builtins_txn.c
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
 // BASIC++ Interpreter - builtins_txn.c
 // ---
 //
 // Transaction query function handlers for the built-in function
 // registry.
 //
 // FUNCTIONS:
 //   TXNSTATUS()  - Return current transaction state
 //                  0 = no transaction
 //                  1 = TXN active (explicit)
 //                  2 = ATOMIC block active
 //
//
// HOW TO EXTEND:
//   To add a new built-in function:
//   1. Write a handler: BValue my_func(BValue *args, int argc, void *ctx)
//   2. Register it in the init function with funcreg_add().
//   3. Specify min/max argument counts and return type.
//
// TROUBLESHOOTING:
//   - Wrong arg count: check min_args/max_args in registration.
//   - Type mismatch: use bval_to_float/bval_to_int for conversion.
 // ---

#include "builtins.h"
#include "txn.h"
#include "value.h"

 // TXNSTATUS() - Return current transaction state.
 //
 // Returns:
 //   0 = TXN_NONE    (no active transaction)
 //   1 = TXN_ACTIVE  (explicit TXN BEGIN)
 //   2 = TXN_ATOMIC  (ATOMIC block)
 //
 // No arguments required.
 // Category: FCAT_IO | Safety: FSAFE_PURE
BValue builtin_txnstatus(BValue *args, int argc, void *rt)
{
    (void)args; (void)argc; (void)rt;
    return bval_int((long)txn_is_active());
}

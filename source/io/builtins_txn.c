/*
 * ---
 * BASIC++ Interpreter - builtins_txn.c
 * ---
 *
 * Transaction query function handlers for the built-in function
 * registry.
 *
 * FUNCTIONS:
 *   TXNSTATUS()  - Return current transaction state
 *                  0 = no transaction
 *                  1 = TXN active (explicit)
 *                  2 = ATOMIC block active
 *
 * ---
 */

#include "builtins.h"
#include "txn.h"
#include "value.h"

/*
 * TXNSTATUS() - Return current transaction state.
 *
 * Returns:
 *   0 = TXN_NONE    (no active transaction)
 *   1 = TXN_ACTIVE  (explicit TXN BEGIN)
 *   2 = TXN_ATOMIC  (ATOMIC block)
 *
 * No arguments required.
 * Category: FCAT_IO | Safety: FSAFE_PURE
 */
BValue builtin_txnstatus(BValue *args, int argc, void *rt)
{
    (void)args; (void)argc; (void)rt;
    return bval_int((long)txn_is_active());
}

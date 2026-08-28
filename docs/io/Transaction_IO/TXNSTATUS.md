# TXNSTATUS Function Reference

The `TXNSTATUS` built-in function returns the lifecycle status code of an active atomic disk/database transaction (`libserver` / `kv.c`).

## Syntax

```basic
txn_state% = TXNSTATUS(transaction_id%)
```

## Parameters

- **`transaction_id%`** — Active transaction handle ID ($1 \le \text{id} \le 16$).

## Return Value & Transaction State Codes

| Code | State Name | Meaning |
|------|------------|---------|
| **`0`** | `INACTIVE` | Transaction handle is unallocated or closed. |
| **`1`** | `ACTIVE` | Transaction is in-flight; mutations buffered in memory. |
| **`2`** | `COMMITTED`| Transaction successfully committed and written to disk. |
| **`3`** | `ABORTED` | Transaction rolled back; no changes persisted. |

---

## Code Examples

### Example 1: Verifying Transaction State
```basic
10 Txn% = 1
20 State% = TXNSTATUS(Txn%)
30 IF State% = 1 THEN PRINT "Transaction in progress..."
```

---

## Engine Implementation (`server/kv.c` & `eval_builtins.c`)

In `engine/src/server/kv.c`:
`TXNSTATUS` queries the state enum in `KVTransactionContext`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 5 | Illegal Function Call (`ERR_ILLEGAL_FUNCTION_CALL`) | Transaction ID out of range |

---

## Cross-References

- **`Persistent_IO/PSTORE.md`** — Persistent storage store.
- **`Persistent_IO/PRETRIEVE.md`** — Persistent storage retrieve.
- **`game_engines/tinydb.md`** — Document key-value database.

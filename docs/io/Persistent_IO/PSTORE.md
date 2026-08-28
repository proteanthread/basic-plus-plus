# PSTORE Statement Reference

The `PSTORE` statement stores a numeric or string value associated with a key in the BASIC++ persistent key-value store (`libserver` / `kv.c`).

## Syntax

```basic
PSTORE key_string$, value_expression
```

## Parameters

- **`key_string$`** — A string expression specifying the destination storage key.
- **`value_expression`** — A numeric or string expression to store under `key_string$`.

---

## Code Examples

### Example 1: Saving Player Configuration and High Score
```basic
10 PSTORE "game.highscore", 99500
20 PSTORE "game.playername", "Alice"
30 PRINT "Settings saved persistently."
```

---

## Engine Implementation (`statements/io/stmt_pstore.c` & `server/kv.c`)

In `engine/src/statements/io/stmt_pstore.c`:
`PSTORE` evaluates the key and value, then inserts or updates the entry in the persistent hash map via `kv_set()`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Key is non-string |

---

## Cross-References

- **`PRETRIEVE.md`** — Numeric persistent retrieval.
- **`PRETRIEVE_STR.md`** — String persistent retrieval.
- **`tinydb.md`** — In-memory document storage.

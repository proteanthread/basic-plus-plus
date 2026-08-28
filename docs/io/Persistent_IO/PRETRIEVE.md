# PRETRIEVE Function Reference

The `PRETRIEVE` built-in numeric function retrieves a persistent numeric value associated with a string key from the BASIC++ persistent key-value store (`libserver` / `kv.c`).

## Syntax

```basic
value# = PRETRIEVE(key_string$)
```

## Parameters

- **`key_string$`** — A string expression specifying the lookup key.

## Return Value

- Returns a **double-precision number** (`VAL_NUMBER`) stored under `key_string$`.
- If `key_string$` is not found, returns `0`.

---

## Code Examples

### Example 1: Loading Persistent High Scores
```basic
10 HighScore# = PRETRIEVE("game.highscore")
20 PRINT "Current Saved High Score: "; HighScore#
```

---

## Engine Implementation (`server/kv.c` & `eval_builtins.c`)

In `engine/src/server/kv.c`:
`PRETRIEVE` executes a hash table lookup in the global persistent store.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument is non-string |

---

## Cross-References

- **`PRETRIEVE_STR.md`** — String persistent retrieval (`PRETRIEVE$`).
- **`PSTORE.md`** — Persistent storage write.
- **`Persistence/Key_Value_Storage/PRETRIEVE$.md`** — Top-level KV store.

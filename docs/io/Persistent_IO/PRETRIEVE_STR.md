# PRETRIEVE$ Function Reference

The `PRETRIEVE$` built-in string function retrieves a persistent string value associated with a string key from the BASIC++ persistent key-value store (`libserver` / `kv.c`).

## Syntax

```basic
value_str$ = PRETRIEVE$(key_string$)
```

## Parameters

- **`key_string$`** — A string expression specifying the lookup key.

## Return Value

- Returns a **string** (`VAL_STRING`) stored under `key_string$`.
- If `key_string$` is not found, returns `""` (empty string).

---

## Code Examples

### Example 1: Loading Saved Player Name
```basic
10 PlayerName$ = PRETRIEVE$("player.name")
20 IF PlayerName$ = "" THEN PlayerName$ = "Guest"
30 PRINT "Welcome back, "; PlayerName$; "!"
```

---

## Engine Implementation (`server/kv.c` & `eval_builtins.c`)

In `engine/src/server/kv.c`:
`PRETRIEVE$` retrieves string payload bytes from the persistent hash bucket and constructs a reference-counted string via `str_create()`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument is non-string |

---

## Cross-References

- **`PRETRIEVE.md`** — Numeric persistent retrieval.
- **`PSTORE.md`** — Persistent storage write.
- **`Persistence/Key_Value_Storage/PRETRIEVE$.md`** — Top-level KV store reference.

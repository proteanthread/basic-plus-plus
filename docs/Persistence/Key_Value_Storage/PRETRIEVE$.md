# PRETRIEVE$

## Syntax

```basic
result$ = PRETRIEVE$(key$)
```

## Parameters

- **`key$`** — A string expression specifying the unique key identifier in the persistent key-value storage database.

## Return Value

- Returns a **string** (`VAL_STRING`) corresponding to the value previously associated with `key$`.
- If the specified key does not exist in the database or has expired/been deleted, `PRETRIEVE$` returns an empty string (`""`).

## Description

`PRETRIEVE$` (Persistent Retrieve String) is a BASIC++ built-in string function designed to read persisted string data from the local application key-value store. It pairs directly with the `PSTORE` statement, enabling BASIC++ programs, utilities, and games to retain configurations, high scores, user preferences, session tokens, or serialized data between runs without manually managing low-level file I/O operations or record offsets.

In standard BASIC naming conventions, the trailing `$` denotes that `PRETRIEVE$` returns a string value. Its companion function `PRETRIEVE(key$)` retrieves numeric (floating-point or integer) values.

### Key Characteristics:
1. **Automatic Serialization**: Reads stored key-value pairs from an isolated, host-managed storage file without requiring `OPEN`, `FIELD`, `GET`, `PUT`, or `CLOSE`.
2. **Safe Defaults**: Non-existent keys safely yield an empty string (`""`) rather than generating a fatal runtime exception.
3. **Session Persistence**: Values persisted via `PSTORE` remain in storage even after the BASIC++ process terminates, making `PRETRIEVE$` ideal for application state recovery.

## Code Examples

### Example 1: Loading User Profile / Preferences
```basic
10 REM Check if user name is stored in persistent key-value database
20 UserName$ = PRETRIEVE$("USER_NAME")
30 IF UserName$ = "" THEN
40     PRINT "First time setup: What is your name?"
50     INPUT "> ", UserName$
60     PSTORE "USER_NAME", UserName$
70     PRINT "Saved profile for "; UserName$
80 ELSE
90     PRINT "Welcome back, "; UserName$; "!"
100 END IF
```

### Example 2: Loading Game State and Settings
```basic
100 REM Load configuration values
110 Theme$ = PRETRIEVE$("CONFIG_THEME")
120 IF Theme$ = "" THEN Theme$ = "DARK"
130 AudioMode$ = PRETRIEVE$("AUDIO_DRIVER")
140 IF AudioMode$ = "" THEN AudioMode$ = "SDL2"
150 PRINT "Active Theme: "; Theme$
160 PRINT "Audio Driver: "; AudioMode$
```

### Example 3: Serialized Array Restoration
```basic
10 DIM HighScores$(5)
20 FOR I = 1 TO 5
30     KeyName$ = "HIGHSCORE_SLOT_" + STR$(I)
40     Entry$ = PRETRIEVE$(KeyName$)
50     IF Entry$ = "" THEN Entry$ = "EMPTY - 0000"
60     HighScores$(I) = Entry$
70     PRINT "Rank "; I; ": "; HighScores$(I)
80 NEXT I
```

## Behavioral Edge Cases and Constraints

- **Empty Key**: Passing an empty string (`""`) as `key$` is a valid expression evaluation but will return `""` unless an empty key was explicitly saved.
- **Type Safety**: Passing a numeric expression instead of a string for `key$` triggers Error 13 (`ERR_TYPE_MISMATCH`).
- **Memory Lifecycle**: String values returned by `PRETRIEVE$` enter the reference-counted string pool (`BppStringPool`) via `str_create()`. The caller (e.g. assignment handler or expression stack) assumes ownership of the string and manages its reference count.
- **Key Casing**: Key lookups may be case-sensitive or case-insensitive depending on dialect configuration; standard BASIC++ key-value storage performs exact byte-wise string key matching.

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | `key$` argument evaluates to a numeric type instead of string |
| 14 | Out of String Space (`ERR_OUT_OF_STRING_SPACE`) | VM string pool cannot allocate space for the retrieved string |
| 70 | Permission Denied (`ERR_PERMISSION_DENIED`) | Security sandbox restricts persistent disk storage access |

## Cross-References

- **`PSTORE`** — Stores a persistent key-value pair to disk storage.
- **`PRETRIEVE`** — Numeric variant; retrieves a floating-point number for a given string key.
- **`ENVIRON$`** — Reads environment variables from the host operating system.
- **`_STATESAVE`** / **`_STATELOAD`** — Serializes or restores full VM execution state.
- **`COMMON`** — Shares in-memory variables across `CHAIN` program boundaries.

---

## Proposed Expansion or Changes

In legacy BASIC++ v5.0.5, persistent key-value storage was implemented through dedicated modules:
- Lexer token: `KW_PRETRIEVES` (`source/parser/lexer.c`)
- AST function node: `FUNC_PRETRIEVES` (`source/parser/ast.c`)
- Runtime builtin: `builtin_pretrieve_str()` (`source/persist/builtins_persist.c`)
- Key-Value Engine: `db_get_string()` (`source/persist/kv_store.c`)

### Required v6.5.2 Implementation Blueprint:
1. **Lexer Registration**: Add `KW_PRETRIEVE_STR` (`PRETRIEVE$`) and `KW_PRETRIEVE` (`PRETRIEVE`) to `engine/include/lexer/lexer.h` and token table in `engine/src/lexer/lexer.c`.
2. **Evaluator Dispatch**: Wire `PRETRIEVE$` into the built-in function dispatch in `engine/src/eval/eval_builtins.c` or `engine/src/eval/dispatch.c`.
3. **Storage Subsystem (`libserver` / `libkernel`)**: Implement a lightweight embedded key-value store module (`engine/src/runtime/kv_store.c`) backed by a per-application database file (e.g. `.bpp_storage.db` or JSON/binary KV format).
4. **Function Signature**:
   ```c
   BValue func_pretrieve_str_exec(VMContext *vm, const char *key, BppError *err);
   ```
5. **Help & Introspection**: Register `PRETRIEVE$` in `engine/src/statements/dialect/help_data.h` under the `"Persistence"` category with syntax `"result$ = PRETRIEVE$(key$)"`.

# USERNAME Statement & Function Reference

The `USERNAME` statement and `USERNAME$` built-in function provide access to the operating system username of the current user account running the interpreter.

## Syntax

### Statement Syntax
```basic
USERNAME
```

### Function Syntax
```basic
user$ = USERNAME$
```

## Parameters

*(Neither form takes arguments.)*

## Return Value (`USERNAME$`)

- Returns a **string** (`VAL_STRING`) containing the username of the active operating system user account.
- If the username cannot be resolved from the host environment, returns `"user"`.

## Description

The `USERNAME` statement immediately prints the active username to the virtual console (`VCon`).

The `USERNAME$` function returns the username as an evaluated string value for use in personalized greeting messages, file path constructions, save directory creation, or audit logs.

```basic
10 REM Personalize interactive greeting
20 U$ = USERNAME$
30 PRINT "Welcome to BASIC++, "; U$; "!"
```

---

## Code Examples

### Example 1: Console Statement Invocation
```basic
> USERNAME
rtdos
Ok
```

### Example 2: User-Specific Save File Path
```basic
100 REM Construct User Preferences File Path
110 PrefFile$ = "C:/Users/" + USERNAME$ + "/AppData/Local/mygame.cfg"
120 PRINT "Saving user settings to: "; PrefFile$
```

---

## Engine Implementation (`system.c` & `eval_builtins.c`)

### Statement Handler
In `engine/src/statements/system/system.c`:
`stmt_username_handler` fetches the platform username and prints it to `vm_get_vdev(vm)`.

### Built-in Function Evaluator
In `engine/src/eval/eval_builtins.c` (lines 832–842):
```c
else if (strcmp(uname, "USERNAME$") == 0) {
    if (arg_count != 0) {
        err->code = 13; err->message = "USERNAME$ expects no arguments"; return res;
    }
    const char *env_val = getenv("USER");
    if (!env_val) env_val = getenv("USERNAME");
    if (!env_val) env_val = "user";
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), env_val, strlen(env_val));
    return res;
}
```

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 13 | Type Mismatch / Argument Error (`ERR_TYPE_MISMATCH`) | Passing arguments to `USERNAME$` or `USERNAME` |

---

## Cross-References

- **`HOSTNAME`** / **`HOSTNAME$`** — Queries current host network name.
- **`ENVIRON$`** — Reads arbitrary environment variables from host OS.
- **`INFO`** — Displays complete system metrics.

---

## Proposed Expansion or Changes

1. **Sandboxed Anonymization Mode**: In secure sandbox levels (Level 4+), return an anonymous hash (e.g. `"anon_user"`) to prevent leaking OS user account names.

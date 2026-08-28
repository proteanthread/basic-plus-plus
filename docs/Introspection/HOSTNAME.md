# HOSTNAME Statement & Function Reference

The `HOSTNAME` statement and `HOSTNAME$` built-in function provide access to the network hostname of the host machine.

## Syntax

### Statement Syntax
```basic
HOSTNAME
```

### Function Syntax
```basic
host$ = HOSTNAME$
```

## Parameters

*(Neither form takes arguments.)*

## Return Value (`HOSTNAME$`)

- Returns a **string** (`VAL_STRING`) containing the network hostname of the host system.
- If the environment hostname cannot be resolved, returns `"localhost"`.

## Description

The `HOSTNAME` statement immediately prints the host system's network hostname to the active virtual console (`VCon`).

The `HOSTNAME$` function returns the hostname as an evaluated string value that can be assigned to variables, concatenated with strings, or sent over network telemetry channels (`N1:`).

```basic
10 REM Inspect host network identity
20 HOSTNAME : REM Prints directly to console
30 H$ = HOSTNAME$
40 PRINT "Connected to server node: "; H$
```

---

## Code Examples

### Example 1: Console Statement Invocation
```basic
> HOSTNAME
workstation-win11
Ok
```

### Example 2: Telemetry Packet Formatting
```basic
100 REM Format telemetry packet with host name
110 NodeID$ = HOSTNAME$
120 Timestamp$ = TIME$
130 Packet$ = "NODE=" + NodeID$ + ";TIME=" + Timestamp$ + ";STATUS=ONLINE"
140 PRINT Packet$
```

---

## Engine Implementation (`system.c` & `eval_builtins.c`)

### Statement Handler
In `engine/src/statements/system/system.c`:
`stmt_hostname_handler` queries the platform hostname and emits it directly to `vm_get_vdev(vm)`.

### Built-in Function Evaluator
In `engine/src/eval/eval_builtins.c` (lines 822–831):
```c
else if (strcmp(uname, "HOSTNAME$") == 0) {
    if (arg_count != 0) {
        err->code = 13; err->message = "HOSTNAME$ expects no arguments"; return res;
    }
    const char *env_val = getenv("HOSTNAME");
    if (!env_val) env_val = getenv("COMPUTERNAME");
    if (!env_val) env_val = "localhost";
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), env_val, strlen(env_val));
    return res;
}
```

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 13 | Type Mismatch / Argument Error (`ERR_TYPE_MISMATCH`) | Passing arguments to `HOSTNAME$` or `HOSTNAME` |

---

## Cross-References

- **`USERNAME`** / **`USERNAME$`** — Queries current logged-in user name.
- **`ENVIRON$`** — Queries arbitrary environment variables from host OS.
- **`Virtual_Network.md`** — Virtual network socket communication.

---

## Proposed Expansion or Changes

1. **`HOSTNAME$(mode%)` Parameter**: Add optional mode parameter (e.g. `0` = Short Name, `1` = Fully Qualified Domain Name `FQDN`, `2` = Primary IP Address).

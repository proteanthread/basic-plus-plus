# NERROR Function Reference

The `NERROR` built-in function returns the operating system socket error code (such as `WSAECONNREFUSED` or `ECONNRESET`) encountered by an active network I/O channel.

## Syntax

```basic
err_code% = NERROR(channel%)
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).

## Return Value

- Returns an **integer** (`VAL_NUMBER`) representing the raw OS socket error code (or `0` if no error occurred).

---

## Code Examples

### Example 1: Trapping Socket Connection Refusal
```basic
10 OPEN "TCP:127.0.0.1:9999" AS #1
20 IF NOT NCONNECTED(1) THEN
30     Err% = NERROR(1)
40     PRINT "Connection failed with socket error code: "; Err%
50 END IF
60 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c`)

In `engine/src/server/vnet.c`:
`NERROR` retrieves `WSAGetLastError()` on Windows or `errno` on POSIX from `VNetChannelContext`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`NSTATUS.md`** — Protocol status code.
- **`NCONNECTED.md`** — Connection state flag.

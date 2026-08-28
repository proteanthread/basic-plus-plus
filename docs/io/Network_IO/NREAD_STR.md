# NREAD$ Function Reference

The `NREAD$` built-in string function reads a stream of bytes from an active network socket I/O channel into a string expression.

## Syntax

```basic
data_chunk$ = NREAD$(channel%, max_bytes%)
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).
- **`max_bytes%`** — Maximum number of bytes to read ($1 \le \text{max} \le 65535$).

## Return Value

- Returns a **string** (`VAL_STRING`) of length $\le \text{max\_bytes}\%$.
- If no bytes are currently available, returns `""` (empty string) without blocking if channel is non-blocking.

---

## Code Examples

### Example 1: Reading Network Data Stream
```basic
10 OPEN "TCP:192.168.1.100:5000" AS #1
20 DO
30     IF NBYTESWAITING(1) > 0 THEN
40         Msg$ = NREAD$(1, 1024)
50         PRINT "Received: "; Msg$
60     END IF
70     _DELAY 0.05
80 LOOP UNTIL NEOF(1)
90 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c`)

In `engine/src/server/vnet.c`:
`NREAD$` invokes native `recv()` and packages the received bytes into a reference-counted string via `str_create_len()`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument error |

---

## Cross-References

- **`NWRITE.md`** — Socket write.
- **`NBYTESWAITING.md`** — Unread bytes query.
- **`NEOF.md`** — End-of-stream flag.

# NBYTESWAITING Function Reference

The `NBYTESWAITING` built-in function returns the number of unread bytes currently buffered and waiting in a network socket I/O channel (`libserver` / `vnet.c`).

## Syntax

```basic
bytes_available% = NBYTESWAITING(channel%)
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).

## Return Value

- Returns an **integer** (`VAL_NUMBER`) representing the number of bytes available to read immediately without blocking.

---

## Code Examples

### Example 1: Non-Blocking Network Polling
```basic
10 OPEN "TCP:127.0.0.1:8080" AS #1
20 DO
30     Avail% = NBYTESWAITING(1)
40     IF Avail% > 0 THEN
50         Data$ = NREAD$(1, Avail%)
60         PRINT "Received: "; Data$
70     END IF
80     _DELAY 0.05
90 LOOP UNTIL NEOF(1)
100 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c` & `device/vdev.c`)

In `engine/src/server/vnet.c`:
Invokes `ioctlsocket(FIONREAD)` on Windows or `ioctl(FIONREAD)` on POSIX.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open or not a socket |

---

## Cross-References

- **`NREAD_STR.md`** — Reads network bytes.
- **`NCONNECTED.md`** — Connection status flag.
- **`NEOF.md`** — End-of-stream flag.

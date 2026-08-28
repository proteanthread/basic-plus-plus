# NEOF Function Reference

The `NEOF` built-in function returns a boolean flag indicating whether the remote endpoint of a network socket I/O channel has closed the connection (End-of-File / Stream Termination).

## Syntax

```basic
is_eof% = NEOF(channel%)
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).

## Return Value

- Returns **`-1`** (true) if the remote peer has terminated the stream or closed the socket.
- Returns **`0`** (false) while the stream remains open and data can still be received.

---

## Code Examples

### Example 1: Reading an Entire HTTP Stream
```basic
10 OPEN "HTTP://api.example.com/data.txt" AS #1
20 WHILE NOT NEOF(1)
30     Line$ = NREAD$(1, 256)
40     PRINT Line$;
50 WEND
60 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c`)

In `engine/src/server/vnet.c`:
`NEOF` tests whether the socket recv buffer returned 0 bytes (graceful shutdown) or if `recv()` detected connection termination.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`NCONNECTED.md`** — Connection status flag.
- **`NREAD_STR.md`** — Network read.
- **`File_IO/EOF.md`** — Standard file EOF.

# NWRITE Statement Reference

The `NWRITE` statement transmits a raw binary or text payload from a string expression over an active network socket I/O channel.

## Syntax

```basic
NWRITE channel%, payload_string$
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).
- **`payload_string$`** — A string expression containing the data bytes to send.

---

## Code Examples

### Example 1: Sending an HTTP GET Request
```basic
10 OPEN "TCP:example.com:80" AS #1
20 Req$ = "GET /index.html HTTP/1.1" + CHR$(13) + CHR$(10) + _
          "Host: example.com" + CHR$(13) + CHR$(10) + _
          "Connection: close" + CHR$(13) + CHR$(10) + CHR$(13) + CHR$(10)
30 NWRITE 1, Req$
40 WHILE NOT NEOF(1)
50     PRINT NREAD$(1, 512);
60 WEND
70 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c`)

In `engine/src/server/vnet.c`:
`NWRITE` calls `send()` on the native socket handle, handling partial writes and buffer retries automatically.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument error |

---

## Cross-References

- **`NREAD_STR.md`** — Network read.
- **`NCONNECTED.md`** — Socket connection status.

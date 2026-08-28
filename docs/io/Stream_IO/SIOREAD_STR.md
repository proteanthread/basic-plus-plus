# SIOREAD$ Function Reference

The `SIOREAD$` built-in string function reads a specified maximum number of bytes from an active stream I/O channel into a string expression.

## Syntax

```basic
stream_chunk$ = SIOREAD$(channel%, byte_count%)
```

## Parameters

- **`channel%`** — Active stream channel number (`#1` to `#16`).
- **`byte_count%`** — Number of bytes to read ($1 \le \text{count} \le 65535$).

## Return Value

- Returns a **string** (`VAL_STRING`) of length $\le \text{byte\_count}\%$.

---

## Code Examples

```basic
10 OPEN "binary.dat" FOR INPUT AS #1
20 Header$ = SIOREAD$(1, 16) : REM Read first 16 bytes
30 PRINT "Header length: "; LEN(Header$)
40 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`SIOREAD$` reads directly from the stream channel input buffer.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 62 | Input Past End (`ERR_INPUT_PAST_END`) | Reading past end of stream |

---

## Cross-References

- **`SIOREADLN_STR.md`** — Read line from stream.
- **`SIOWRITE.md`** — Write to stream.

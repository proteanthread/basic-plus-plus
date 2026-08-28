# SIOSEEK Statement Reference

The `SIOSEEK` statement positions the active read/write file pointer on a stream I/O channel to a specified byte offset.

## Syntax

```basic
SIOSEEK channel%, byte_offset&
```

## Parameters

- **`channel%`** — Active stream channel number (`#1` to `#16`).
- **`byte_offset&`** — Destination byte position ($0$-based).

---

## Code Examples

```basic
10 OPEN "records.dat" FOR RANDOM AS #1
20 SIOSEEK 1, 1024 : REM Seek to byte 1024
30 Data$ = SIOREAD$(1, 64)
40 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`SIOSEEK` calls `fseek()` / `_fseeki64()` on the underlying stream descriptor.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 5 | Illegal Function Call (`ERR_ILLEGAL_FUNCTION_CALL`) | Negative offset |

---

## Cross-References

- **`File_IO/SEEK.md`** — Standard SEEK statement.
- **`SIOREAD_STR.md`** — Stream read.

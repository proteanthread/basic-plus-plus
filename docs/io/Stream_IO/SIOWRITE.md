# SIOWRITE Statement Reference

The `SIOWRITE` statement writes a string or raw binary payload to an active stream I/O channel.

## Syntax

```basic
SIOWRITE channel%, payload_string$
```

## Parameters

- **`channel%`** — Active stream channel number (`#1` to `#16`).
- **`payload_string$`** — String expression containing the data bytes to write.

---

## Code Examples

```basic
10 OPEN "output.txt" FOR OUTPUT AS #1
20 SIOWRITE 1, "Hello from BASIC++ stream I/O!" + CHR$(10)
30 SIOFLUSH 1
40 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`SIOWRITE` writes bytes directly into the stream channel output buffer.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument error |

---

## Cross-References

- **`SIOFLUSH.md`** — Flush stream buffers.
- **`SIOREAD_STR.md`** — Read from stream.

# SIOFLUSH Statement Reference

The `SIOFLUSH` statement forces any buffered output data on an active stream I/O channel to be flushed immediately to the underlying physical medium or OS device.

## Syntax

```basic
SIOFLUSH channel%
```

## Parameters

- **`channel%`** — Active stream channel number (`#1` to `#16`).

---

## Code Examples

```basic
10 OPEN "log.txt" FOR OUTPUT AS #1
20 PRINT #1, "Transaction committed at "; TIME$
30 SIOFLUSH 1 : REM Ensure buffer is committed to disk
40 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`SIOFLUSH` invokes `fflush()` on host file pointers or triggers hardware synchronization barriers.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`SIOWRITE.md`** — Stream write.
- **`SIOSTATUS.md`** — Stream status.

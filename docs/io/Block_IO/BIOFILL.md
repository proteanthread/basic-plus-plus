# BIOFILL Statement Reference

The `BIOFILL` statement populates a contiguous byte range of a block I/O channel or virtual device buffer with a specified constant byte value.

## Syntax

```basic
BIOFILL channel%, offset&, length&, byte_value%
```

## Parameters

- **`channel%`** — Active block channel number (`#1` to `#16`).
- **`offset&`** — Starting byte offset in the block medium ($0$-based).
- **`length&`** — Number of bytes to fill.
- **`byte_value%`** — An integer byte value ($0$ to $255$).

---

## Code Examples

### Example 1: Formatting a Sector with Zeros
```basic
10 OPEN "disk.img" FOR BLOCK AS #1
20 BIOFILL 1, 0, 512, 0 : REM Zero-fill sector 0
30 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`BIOFILL` executes `memset()` directly across the mapped channel buffer.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 5 | Illegal Function Call (`ERR_ILLEGAL_FUNCTION_CALL`) | `byte_value%` not in `0..255` |
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`BIOCOPY.md`** — Block memory copy.
- **`BIOWRITE.md`** — Block write.

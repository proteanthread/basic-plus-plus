# BIOCOMPARE Function Reference

The `BIOCOMPARE` built-in function performs a fast byte-by-byte comparison between two block I/O channels or virtual device memory buffers.

## Syntax

```basic
diff% = BIOCOMPARE(chan1%, offset1&, chan2%, offset2&, length&)
```

## Parameters

- **`chan1%`**, **`chan2%`** — Active block channel numbers (`#1` to `#16`).
- **`offset1&`**, **`offset2&`** — Starting byte offsets in each respective channel ($0$-based).
- **`length&`** — Number of bytes to compare.

## Return Value

- Returns **`0`** if both memory ranges are identical.
- Returns non-zero (first mismatched byte difference $B_1 - B_2$) if differences are detected.

---

## Code Examples

### Example 1: Comparing Backup Disk Sectors
```basic
10 OPEN "master.img" FOR BLOCK AS #1
20 OPEN "backup.img" FOR BLOCK AS #2
30 Diff% = BIOCOMPARE(1, 0, 2, 0, 512) : REM Compare boot sector (512 bytes)
40 IF Diff% = 0 THEN PRINT "Boot sectors match!" ELSE PRINT "Mismatch found!"
50 CLOSE #1 : CLOSE #2
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`BIOCOMPARE` executes direct `memcmp()` across mapped channel buffers.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Either channel not open |
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument error |

---

## Cross-References

- **`BIOCOPY.md`** — Block memory copy.
- **`BIOCHECKSUM.md`** — Checksum verification.
- **`BIOFILL.md`** — Block memory fill.

# BIOCOPY Statement Reference

The `BIOCOPY` statement executes a high-speed direct block memory transfer between two block I/O channels or virtual devices.

## Syntax

```basic
BIOCOPY source_channel%, source_offset&, dest_channel%, dest_offset&, length&
```

## Parameters

- **`source_channel%`**, **`dest_channel%`** — Open block channel numbers (`#1` to `#16`).
- **`source_offset&`**, **`dest_offset&`** — Starting byte offsets in source and destination channels.
- **`length&`** — Number of bytes to copy.

---

## Code Examples

### Example 1: Cloning a 512-Byte Disk Sector
```basic
10 OPEN "source.img" FOR BLOCK AS #1
20 OPEN "target.img" FOR BLOCK AS #2
30 BIOCOPY 1, 0, 2, 0, 512 : REM Copy sector 0
40 CLOSE #1 : CLOSE #2
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`BIOCOPY` uses optimized `memcpy()` or OS memory-mapped block transfers between device contexts.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument error |

---

## Cross-References

- **`BIOFILL.md`** — Block memory fill.
- **`BIOWRITE.md`** — Block write from string.
- **`BIOREAD_STR.md`** — Block read into string.

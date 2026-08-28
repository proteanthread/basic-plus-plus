# BIOWRITE Statement Reference

The `BIOWRITE` statement writes a raw binary byte payload from a string expression directly into a block I/O channel or virtual block device at a specified byte offset.

## Syntax

```basic
BIOWRITE channel%, offset&, byte_data$
```

## Parameters

- **`channel%`** — Active block channel number (`#1` to `#16`).
- **`offset&`** — Starting destination byte offset in the block storage medium ($0$-based).
- **`byte_data$`** — A string expression containing the binary payload to write.

---

## Code Examples

### Example 1: Writing Master Boot Record Signature
```basic
10 OPEN "disk.img" FOR BLOCK AS #1
20 Sig$ = CHR$(&H55) + CHR$(&HAA) : REM Standard 0x55AA MBR signature
30 BIOWRITE 1, 510, Sig$          : REM Write to bytes 510-511
40 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`BIOWRITE` invokes `vdev_write_block()` directly, writing `byte_data$.length` bytes into the underlying block buffer.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 70 | Permission Denied (`ERR_PERMISSION_DENIED`) | Write attempted on read-only block medium |

---

## Cross-References

- **`BIOREAD_STR.md`** — Binary block read.
- **`BIOFILL.md`** — Block memory fill.
- **`BIOCOPY.md`** — Block channel copy.

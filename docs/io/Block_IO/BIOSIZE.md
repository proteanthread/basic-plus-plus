# BIOSIZE Function Reference

The `BIOSIZE` built-in function returns the total storage capacity or file size in bytes of an active block I/O channel or virtual block device.

## Syntax

```basic
size_in_bytes& = BIOSIZE(channel%)
```

## Parameters

- **`channel%`** — Active block channel number (`#1` to `#16`).

## Return Value

- Returns a **64-bit integer / long number** (`VAL_NUMBER`) indicating the total byte capacity of the medium.

---

## Code Examples

### Example 1: Calculating Floppy Disk Sector Capacity
```basic
10 OPEN "floppy.img" FOR BLOCK AS #1
20 TotalBytes& = BIOSIZE(1)
30 Sectors% = TotalBytes& / 512
40 PRINT "Total Capacity: "; TotalBytes&; " bytes ("; Sectors%; " sectors)"
50 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`BIOSIZE` queries `vdev_get_block_size(vdev, channel)` from the virtual device context.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`BIOSTATUS.md`** — Device readiness status.
- **`BIOREAD_STR.md`** — Block reading.
- **`File_IO/LOF.md`** — Standard file length query.

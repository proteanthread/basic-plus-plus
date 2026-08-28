# BIOSTATUS Function Reference

The `BIOSTATUS` built-in function queries the hardware status, readiness flags, and error state bitmask of an active block I/O channel or virtual block device.

## Syntax

```basic
status_mask% = BIOSTATUS(channel%)
```

## Parameters

- **`channel%`** — Active block channel number (`#1` to `#16`).

## Return Value & Status Bitmask

| Bit | Value | Meaning |
|-----|-------|---------|
| — | **`0`** | Device Ready / Success (No Error). |
| `0` | **`1`** | Device Busy / Operation in Progress. |
| `1` | **`2`** | Write Protected / Read-Only Media. |
| `2` | **`4`** | CRC / Parity Data Error. |
| `3` | **`8`** | Seek Fault / Sector Not Found. |
| `7` | **`128`**| General Hardware I/O Fault. |

---

## Code Examples

### Example 1: Checking Write Protection
```basic
10 OPEN "flash.bin" FOR BLOCK AS #1
20 Status% = BIOSTATUS(1)
30 IF (Status% AND 2) THEN PRINT "Warning: Medium is Write-Protected!"
40 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`BIOSTATUS` retrieves `vdev_get_block_status(vdev, channel)`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`BIOSIZE.md`** — Block size query.
- **`BIOWRITE.md`** — Block write.

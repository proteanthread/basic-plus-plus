# BIOCHECKSUM Function Reference

The `BIOCHECKSUM` built-in function calculates an integer CRC32 or Adler32 checksum over a specified byte range of a binary block I/O channel or virtual block device (`libserver` / `libkernel`).

## Syntax

```basic
checksum& = BIOCHECKSUM(channel%, offset&, length&)
checksum& = BIOCHECKSUM(channel%, offset&, length&, algorithm_code%)
```

## Parameters

- **`channel%`** — Active block device channel number (`#1` to `#16`).
- **`offset&`** — Starting byte offset in the block storage medium ($0$-based).
- **`length&`** — Number of contiguous bytes to include in the checksum calculation.
- **`algorithm_code%`** *(Optional)* — Checksum algorithm selector:
  - **`0`**: Standard IEEE 802.3 CRC32 (Default).
  - **`1`**: Fast Adler-32 checksum.

## Return Value

- Returns a **32-bit unsigned long integer** (`VAL_NUMBER`) representing the computed checksum.

---

## Code Examples

### Example 1: Verifying Block Sector Integrity
```basic
10 OPEN "firmware.bin" FOR BLOCK AS #1
20 Size& = BIOSIZE(1)
30 CRC& = BIOCHECKSUM(1, 0, Size&, 0)
40 PRINT "Firmware CRC32: &H"; HEX$(CRC&)
50 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c` & `server/crypto.c`)

In `engine/src/device/vdev.c`:
`BIOCHECKSUM` reads directly from channel buffer memory via `vdev_read_block()` without intermediate string allocations, feeding the byte stream into `crypto_crc32()` in `libserver`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 5 | Illegal Function Call (`ERR_ILLEGAL_FUNCTION_CALL`) | Invalid algorithm code or negative offset/length |
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open or not a block device |

---

## Cross-References

- **`BIOCOMPARE.md`** — Binary block comparison.
- **`BIOREAD_STR.md`** — Binary block read.
- **`BIOSIZE.md`** — Block device size query.

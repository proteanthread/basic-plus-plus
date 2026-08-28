# BIOREAD$ Function Reference

The `BIOREAD$` built-in string function reads a raw binary byte block from a block I/O channel or virtual device directly into a string expression.

## Syntax

```basic
block_data$ = BIOREAD$(channel%, offset&, length&)
```

## Parameters

- **`channel%`** — Active block channel number (`#1` to `#16`).
- **`offset&`** — Starting byte offset in the block storage medium ($0$-based).
- **`length&`** — Number of bytes to read ($1 \le \text{length} \le 65535$).

## Return Value

- Returns a **string** (`VAL_STRING`) of length $\le \text{length}\&$ containing the binary payload.

---

## Code Examples

### Example 1: Reading Disk Header Magic Bytes
```basic
10 OPEN "disk.vhd" FOR BLOCK AS #1
20 Header$ = BIOREAD$(1, 0, 8)
30 IF Header$ = "conectix" THEN PRINT "Valid VHD image header!"
40 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c` & `eval_builtins.c`)

In `engine/src/device/vdev.c`:
`BIOREAD$` invokes `vdev_read_block()` and packages the read buffer directly into a reference-counted string via `str_create_len()`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 62 | Input Past End (`ERR_INPUT_PAST_END`) | Reading past medium bounds |

---

## Cross-References

- **`BIOWRITE.md`** — Binary block write.
- **`BIOCOPY.md`** — Direct channel block copy.

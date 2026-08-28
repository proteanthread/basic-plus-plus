# SIOSTATUS Function Reference

The `SIOSTATUS` built-in function returns the operating state and error flags of an active stream I/O channel.

## Syntax

```basic
status% = SIOSTATUS(channel%)
```

## Parameters

- **`channel%`** — Active stream channel number (`#1` to `#16`).

## Return Value & Status Codes

| Code | Meaning |
|------|---------|
| **`0`** | Stream Ready / OK. |
| **`1`** | Stream EOF (End of Stream). |
| **`2`** | Stream Error / Fault. |

---

## Code Examples

```basic
10 OPEN "device.stream" AS #1
20 Status% = SIOSTATUS(1)
30 IF Status% = 0 THEN PRINT "Stream operational!"
40 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`SIOSTATUS` tests `feof()` and `ferror()` on the underlying stream descriptor.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`SIOAVAIL.md`** — Available stream bytes.
- **`SIOFLUSH.md`** — Stream flush.

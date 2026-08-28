# SIOAVAIL Function Reference

The `SIOAVAIL` built-in function returns the number of bytes available to read from a stream I/O channel without blocking.

## Syntax

```basic
bytes_avail% = SIOAVAIL(channel%)
```

## Parameters

- **`channel%`** — Active stream channel number (`#1` to `#16`).

## Return Value

- Returns an **integer** (`VAL_NUMBER`) indicating buffered bytes ready for immediate reading.

---

## Code Examples

```basic
10 OPEN "stream.pipe" FOR INPUT AS #1
20 IF SIOAVAIL(1) > 0 THEN
30     Line$ = SIOREADLN$(1)
40     PRINT "Stream input: "; Line$
50 END IF
60 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`SIOAVAIL` returns `channel->in_buffer_len - channel->in_buffer_pos`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`SIOREAD_STR.md`** — Stream read.
- **`SIOREADLN_STR.md`** — Stream read line.
- **`SIOSTATUS.md`** — Stream status.

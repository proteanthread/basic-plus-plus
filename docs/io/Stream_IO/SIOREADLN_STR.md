# SIOREADLN$ Function Reference

The `SIOREADLN$` built-in string function reads a line of text terminated by a newline (`CRLF` or `LF`) from an active stream I/O channel.

## Syntax

```basic
line_text$ = SIOREADLN$(channel%)
```

## Parameters

- **`channel%`** — Active stream channel number (`#1` to `#16`).

## Return Value

- Returns a **string** (`VAL_STRING`) containing line text without trailing line terminator characters.

---

## Code Examples

```basic
10 OPEN "config.ini" FOR INPUT AS #1
20 WHILE NOT EOF(1)
30     Line$ = SIOREADLN$(1)
40     PRINT "Line: "; Line$
50 WEND
60 CLOSE #1
```

---

## Engine Implementation (`device/vdev.c`)

In `engine/src/device/vdev.c`:
`SIOREADLN$` scans buffered bytes for `\n`, returning the stripped string slice.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 62 | Input Past End (`ERR_INPUT_PAST_END`) | Reading past end of stream |

---

## Cross-References

- **`SIOREAD_STR.md`** — Binary/block stream read.
- **`SIOAVAIL.md`** — Stream available bytes.

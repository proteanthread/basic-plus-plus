# `OPEN` File and Virtual Device Channel Open Statement

## 1. BASIC Usage and Keyword Definition

Opens a file or virtual device channel for sequential, random-access, or binary data input/output.

### Syntax Signatures:
```basic
OPEN mode$, [#]filenum%, filename$ [, reclen%]
OPEN filename$ [FOR mode] [ACCESS access] [LOCK lock] AS [#]filenum% [LEN = reclen%]
```

### Error Handling & Boundary Conditions:
- **Error 53 (ERR_FILE_NOT_FOUND)**: File not found in input mode.
- **Error 55 (ERR_FILE_ALREADY_OPEN)**: Channel already in use.

### Operational Notes:
- Supports modes: INPUT (I), OUTPUT (O), APPEND (A), RANDOM (R), BINARY (B).

---

## 2. Code Examples

```basic
10 OPEN "O", #1, "DATA.TXT"          : REM Classic syntax
20 OPEN "RECORD.DAT" FOR RANDOM AS #2 LEN = 64 : REM Extended syntax
30 PRINT #1, "Hello World"
40 CLOSE #1, #2
```

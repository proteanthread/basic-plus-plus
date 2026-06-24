# Error Handling in BASIC++

**Version 4.1.1**


---

## Table of Contents

- Error Trapping
  - ON ERROR GOTO
  - ON ERROR GOTO 0
- Error Variables
- RESUME Statements
- ERROR Statement
- Error Codes
- BEEP on Error
- Error Handling Patterns
  - Simple Guard
  - Retry with Timeout
  - Graceful Degradation
  - Error Logging
- Notes

---

BASIC++ provides structured error handling compatible with GW-BASIC and QBasic error trapping.

---

## 1. Error Trapping

### ON ERROR GOTO

```basic
10 ON ERROR GOTO 1000
20 PRINT 1/0              ' Division by zero!
30 PRINT "This won't print"
40 END
1000 PRINT "Error"; ERR; "at line"; ERL
1010 RESUME NEXT
```

When an error occurs, execution jumps to the handler line.

### ON ERROR GOTO 0

Disable error trapping (errors halt the program):

```basic
10 ON ERROR GOTO 0       ' Default behavior
```

---

## 2. Error Variables

| Variable | Description |
|----------|-------------|
| `ERR` | The error number of the last error |
| `ERL` | The line number where the error occurred |

```basic
1000 PRINT "Error #"; ERR
1010 PRINT "At line "; ERL
1020 IF ERR = 11 THEN PRINT "Division by zero"
1030 IF ERR = 6  THEN PRINT "Overflow"
1040 RESUME NEXT
```

---

## 3. RESUME Statements

| Statement | Description |
|-----------|-------------|
| `RESUME` | Retry the statement that caused the error |
| `RESUME NEXT` | Skip the failed statement, continue with the next |
| `RESUME line` | Jump to a specific line |

```basic
' RESUME — retry after fixing the problem
1000 A = 1              ' Set a safe value
1010 RESUME              ' Retry the failed line

' RESUME NEXT — skip and continue
1000 PRINT "Skipping error at line"; ERL
1010 RESUME NEXT

' RESUME line — jump to specific recovery point
1000 PRINT "Recovering..."
1010 RESUME 100          ' Continue from line 100
```

---

## 4. ERROR Statement

Trigger an error deliberately:

```basic
ERROR 5                  ' Raise error #5
ERROR 200                ' User-defined error
```

Useful for:
- Testing error handlers
- Signaling custom error conditions
- Chaining error handling

---

## 5. Error Codes

Standard GW-BASIC/QBasic error codes:

| Code | Description |
|------|-------------|
| 1 | NEXT without FOR |
| 2 | Syntax error |
| 3 | RETURN without GOSUB |
| 4 | Out of DATA |
| 5 | Illegal function call |
| 6 | Overflow |
| 7 | Out of memory |
| 8 | Undefined line number |
| 9 | Subscript out of range |
| 10 | Duplicate definition |
| 11 | Division by zero |
| 12 | Illegal direct |
| 13 | Type mismatch |
| 14 | Out of string space |
| 15 | String too long |
| 16 | String formula too complex |
| 17 | Can't continue |
| 18 | Undefined user function |
| 19 | No RESUME |
| 20 | RESUME without error |
| 24 | Device timeout |
| 25 | Device fault |
| 27 | Out of paper |
| 50 | FIELD overflow |
| 51 | Internal error |
| 52 | Bad file number |
| 53 | File not found |
| 54 | Bad file mode |
| 55 | File already open |
| 57 | Device I/O error |
| 58 | File already exists |
| 61 | Disk full |
| 62 | Input past end |
| 63 | Bad record number |
| 64 | Bad file name |
| 67 | Too many files |
| 68 | Device unavailable |
| 69 | Communication buffer overflow |
| 70 | Permission denied |
| 71 | Disk not ready |
| 72 | Disk media error |
| 73 | Advanced feature |
| 74 | Rename across disks |
| 75 | Path/file access error |
| 76 | Path not found |

**User-defined errors:** 200–255 are reserved for applications.

---

## 6. BEEP on Error

```basic
BEEP ON                  ' Enable error beep (default)
BEEP OFF                 ' Disable error beep
```

The beep is suppressed during batch execution and redirected output.

---

## 7. Error Handling Patterns

### A. Simple Guard

```basic
10 ON ERROR GOTO 100
20 OPEN "data.txt" FOR INPUT AS #1
30 ' Process file...
40 CLOSE #1
50 ON ERROR GOTO 0
60 END
100 IF ERR = 53 THEN PRINT "File not found!" : END
110 ON ERROR GOTO 0 : ERROR ERR  ' Re-raise unknown
```

### B. Retry with Timeout

```basic
10 ON ERROR GOTO 100
20 TRIES = 0
30 OPEN "lockfile.dat" FOR OUTPUT AS #1
40 PRINT #1, "locked"
50 CLOSE #1
60 END
100 TRIES = TRIES + 1
110 IF TRIES > 5 THEN PRINT "Failed!" : END
120 PRINT "Retry #"; TRIES
130 SHELL "sleep 1"
140 RESUME
```

### C. Graceful Degradation

```basic
10 ON ERROR GOTO 100
20 A$ = SHELL$("advanced_command")
30 GOTO 200
100 A$ = "default value"    ' Fallback
110 RESUME 200
200 PRINT "Result: "; A$
```

### D. Error Logging

```basic
10 ON ERROR GOTO 1000
20 ' ... program code ...
999 END
1000 PRINT >> "error.log", DATE$; " "; TIME$; " Error "; ERR; " at "; ERL
1010 RESUME NEXT
```

---

## 8. Notes

- `ON ERROR GOTO` is global — only one handler at a time
- `RESUME` clears the error state
- Nested errors (error inside error handler) halt the program
- In direct mode, errors always print immediately
- `ERR` and `ERL` are reset by `RESUME` or a successful `RUN`
- `ON ERROR GOTO 0` disables trapping and re-enables default error reporting (print message and stop)

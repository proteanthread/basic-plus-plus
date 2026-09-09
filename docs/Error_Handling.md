<!--
Title:        Error_Handling
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans)
Authority:    engine/include/types/errors.h, engine/src/vm/error.c
Generated:    no, manual reference
Status:       current
-->

# BASIC++ v6.5.2 Error Handling Reference

## 1. THE ERROR SYSTEM

BASIC++ implements a structured error handling architecture compatible with GW-BASIC and QuickBASIC numeric error codes. When a runtime error occurs, the virtual machine records the numeric fault code, captures the BASIC line number where the failure occurred, and sets an error message string. If no active error trap is registered, the interpreter prints the diagnostic message and line number, terminating program execution. If an error handler has been registered via `ON ERROR GOTO`, execution transfers immediately to the designated handler line.

The error state is encapsulated internally by the `BppError` structure defined in `engine/include/types/types.h`, which contains the numeric error code (1-255), category classification (syntax, runtime, system, or internal), diagnostic message string, line number, column offset, and source file identifier.

## 2. TRAPPING ERRORS WITH ON ERROR GOTO

`ON ERROR GOTO line` registers a global error handling subroutine. When an error occurs, execution jumps to the designated line number:

```basic
10 ON ERROR GOTO 100
20 OPEN "MISSING.DAT" FOR INPUT AS #1
30 PRINT "File opened successfully"
40 END
100 PRINT "Trapped Error"; ERR; "on line"; ERL; ": "; ERR$
110 RESUME NEXT
```

When the file operation on line 20 fails, the VM sets `ERR` to 53 (File not found) and `ERL` to 20, transferring control to line 100. `RESUME NEXT` returns execution to line 30, skipping the failed statement.

`ON ERROR GOTO 0` deactivates error trapping, restoring default termination behavior. If an unhandled error occurs while `ON ERROR GOTO 0` is active, the VM halts immediately.

## 3. THE ERROR VARIABLES

- `ERR`: Returns the numeric error code of the most recent error. Holds 0 if no error has occurred since program start or `CLEAR`.
- `ERL`: Returns the line number of the statement that caused the error. Returns 0 for errors originating in immediate mode.
- `ERR$`: Returns the error description string (e.g., "File not found", "Division by zero", "Subscript out of range").

## 4. RESUME VARIANTS

Error handlers must terminate with one of three `RESUME` statement variants:
- `RESUME` (or `RESUME 0`): Clears the error state and re-executes the statement that caused the error. Used when the handler has resolved the underlying condition.
- `RESUME NEXT`: Clears the error state and resumes execution at the statement immediately following the one that failed.
- `RESUME line`: Clears the error state and jumps to the specified line number.

Executing `RESUME` outside of an active error handler produces Error 20 (RESUME without error).

## 5. THE ERROR STATEMENT

`ERROR n` deliberately triggers a runtime error with code `n` (1-255). This allows custom error simulation, validation checking, and raising application-level faults:

```basic
10 ON ERROR GOTO 100
20 INPUT "Enter positive value: ", X
30 IF X <= 0 THEN ERROR 5  ' Illegal function call
40 PRINT "Square root is"; SQR(X)
50 END
100 PRINT "Input validation error: "; ERR$
110 RESUME 20
```

## 6. ERROR CODE REFERENCE TABLE

Authoritative numeric error codes from `engine/include/types/errors.h`:

| Code | Constant | Error Description Text |
| :--- | :--- | :--- |
| **1** | `ERR_NEXT_WITHOUT_FOR` | NEXT without FOR |
| **2** | `ERR_SYNTAX` | Syntax error |
| **3** | `ERR_RETURN_WITHOUT_GOSUB` | RETURN without GOSUB |
| **4** | `ERR_OUT_OF_DATA` | Out of DATA |
| **5** | `ERR_ILLEGAL_FUNCTION_CALL` | Illegal function call |
| **6** | `ERR_OVERFLOW` | Overflow |
| **7** | `ERR_OUT_OF_MEMORY` | Out of memory |
| **8** | `ERR_UNDEFINED_LINE` | Undefined line number |
| **9** | `ERR_SUBSCRIPT_OUT_OF_RANGE` | Subscript out of range |
| **10** | `ERR_REDIM_ARRAY` | Duplicate definition |
| **11** | `ERR_DIVISION_BY_ZERO` | Division by zero |
| **12** | `ERR_ILLEGAL_DIRECT` | Illegal in direct mode |
| **13** | `ERR_TYPE_MISMATCH` | Type mismatch |
| **14** | `ERR_OUT_OF_STRING_SPACE` | Out of string space |
| **15** | `ERR_STRING_TOO_LONG` | String too long |
| **16** | `ERR_STRING_FORMULA_TOO_COMPLEX` | String formula too complex |
| **17** | `ERR_CANNOT_CONTINUE` | Cannot continue |
| **18** | `ERR_UNDEFINED_USER_FUNCTION` | Undefined user function |
| **19** | `ERR_NO_RESUME` | No RESUME |
| **20** | `ERR_RESUME_WITHOUT_ERROR` | RESUME without error |
| **24** | `ERR_DEVICE_TIMEOUT` | Device timeout |
| **25** | `ERR_DEVICE_FAULT` | Device fault |
| **27** | `ERR_OUT_OF_PAPER` | Out of paper |
| **29** | `ERR_WHILE_WITHOUT_WEND` | WHILE without WEND |
| **30** | `ERR_FOR_WITHOUT_NEXT` | FOR without NEXT |
| **35** | `ERR_FIELD_NOT_DEFINED` | FIELD not defined |
| **52** | `ERR_BAD_FILE_NUMBER` | Bad file number |
| **53** | `ERR_FILE_NOT_FOUND` | File not found |
| **54** | `ERR_BAD_FILE_MODE` | Bad file mode |
| **55** | `ERR_FILE_ALREADY_OPEN` | File already open |
| **56** | `ERR_FIELD_OVERFLOW` | FIELD overflow |
| **57** | `ERR_DEVICE_IO_ERROR` | Device I/O error |
| **58** | `ERR_FILE_ALREADY_EXISTS` | File already exists |
| **59** | `ERR_BAD_RECORD_LENGTH` | Bad record length |
| **61** | `ERR_DISK_FULL` | Disk full |
| **62** | `ERR_INPUT_PAST_END` | Input past end |
| **63** | `ERR_BAD_RECORD_NUMBER` | Bad record number |
| **64** | `ERR_BAD_FILE_NAME` | Bad file name |
| **67** | `ERR_TOO_MANY_FILES` | Too many files |
| **68** | `ERR_DEVICE_UNAVAILABLE` | Device unavailable |
| **70** | `ERR_PERMISSION_DENIED` | Permission denied |
| **71** | `ERR_DISK_NOT_READY` | Disk not ready |
| **72** | `ERR_DISK_MEDIA_ERROR` | Disk media error |
| **73** | `ERR_ADVANCED_FEATURE_DISABLED` | Advanced feature disabled |
| **74** | `ERR_RENAME_ACROSS_DISKS` | Rename across disks |
| **75** | `ERR_PATH_FILE_ACCESS_ERROR` | Path/File access error |
| **76** | `ERR_PATH_NOT_FOUND` | Path not found |

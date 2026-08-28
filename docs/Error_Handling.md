# BASIC++ v6.5.2 Error Handling Reference

## 1. THE ERROR SYSTEM

BASIC++ uses a structured error system with GW-BASIC/QBASIC-compatible numeric error codes. When a runtime error occurs, the VM sets the error code, records the BASIC line number where the fault occurred, and stores a readable error message string. If no error handler is active, the interpreter prints the error message and line number, then returns to the prompt. If an error handler is active (established by ON ERROR GOTO), the VM transfers execution to the handler line instead.

The error state is represented internally by the BppError structure, which contains the error code (1-255), the error category (syntax, runtime, system, or internal), the error message text, the BASIC line number, the column position, and the host source file name for diagnostic purposes. This structure is defined in engine/include/types/types.h.

## 2. TRAPPING ERRORS WITH ON ERROR GOTO

ON ERROR GOTO line establishes a global error handler. After this statement executes, any subsequent runtime error transfers execution to the specified line number rather than printing an error message and stopping:

```basic
10 ON ERROR GOTO 100
20 OPEN "NOFILE.TXT" FOR INPUT AS #1
30 PRINT "File opened successfully"
40 END
100 PRINT "Error"; ERR; "on line"; ERL
110 RESUME NEXT
```

When the OPEN on line 20 fails because the file does not exist, the VM sets ERR to 53 (File not found) and ERL to 20, then jumps to line 100. The handler prints the error details and RESUME NEXT returns execution to line 30, skipping the failed OPEN.

ON ERROR GOTO 0 disables error trapping and restores the default behavior of printing errors and stopping execution. If an error occurs while ON ERROR GOTO 0 is active, the program stops immediately.

## 3. THE ERROR VARIABLES

ERR returns the numeric error code of the most recent error. It is zero if no error has occurred since the last CLEAR or RUN. The value persists until the next error occurs or until ERR is explicitly cleared.

ERL returns the line number where the most recent error occurred. For errors in immediate mode, ERL returns 0. ERL is a double-precision value because BASIC++ supports fractional line numbers.

ERR$ returns the error message text as a string: "Syntax error", "Type mismatch", "File not found", and so on. This is a BASIC++ extension not present in GW-BASIC.

## 4. RESUME VARIANTS

RESUME (or RESUME 0) returns from the error handler and re-executes the statement that caused the error. Use this when the handler has corrected the condition that caused the error (for example, creating a missing file or providing a default value).

RESUME NEXT returns from the error handler and continues execution at the statement immediately following the one that caused the error. This is the most common handler exit for errors that should be skipped.

RESUME line returns from the error handler and transfers execution to the specified line number. This allows routing to an alternative code path after an error.

Calling RESUME outside of an error handler produces Error 20 (RESUME without error). Using RESUME after ON ERROR GOTO 0 has disabled trapping produces the same error.

## 5. THE ERROR STATEMENT

ERROR n deliberately triggers a runtime error with the specified code. This is used to test error handlers and to raise custom application-level errors:

```basic
10 ON ERROR GOTO 100
20 ERROR 200
30 END
100 IF ERR = 200 THEN PRINT "Custom error caught"
110 RESUME NEXT
```

Error codes 1 through 76 are reserved for system errors. Codes 200 through 255 are available for application-defined errors. Using ERROR with a code in the reserved range triggers the corresponding system error behavior.

## 6. STRUCTURED EXCEPTION HANDLING WITH TRY/CATCH

BASIC++ extends traditional error handling with TRY...CATCH...END TRY blocks that provide structured exception handling with automatic stack unwinding:

```basic
10 TRY
20   OPEN "MISSING.TXT" FOR INPUT AS #1
30   INPUT #1, A$
40   CLOSE #1
50 CATCH
60   PRINT "Failed to read file: "; ERR$
70 END TRY
80 PRINT "Continuing normally"
```

If any statement inside the TRY block raises an error, execution transfers immediately to the CATCH block. After the CATCH block completes, execution continues at the statement after END TRY. If no error occurs, the CATCH block is skipped entirely.

TRY blocks can be nested. Each TRY pushes a BppTryFrame onto the TryStack, which records the CATCH line, the END TRY line, and the current depths of all internal stacks (GOSUB, FOR, WHILE, DO, SELECT, SUB). When an error triggers a CATCH, the VM restores all stack depths to their values at the time the TRY was entered, ensuring that partially-executed loops and subroutine calls are properly unwound.

THROW n raises a user-defined exception with the specified error code. THROW can be used inside or outside TRY blocks. If a TRY block is active, the nearest CATCH handles it. If no TRY block is active but ON ERROR GOTO is set, the error handler receives it. If neither is active, the program stops with the error.

## 7. ECMA-116 EXCEPTION HANDLING

The ECMA-116 Full BASIC dialect provides an alternative exception handling syntax:

```basic
10 WHEN EXCEPTION IN
20   OPEN "MISSING.TXT" FOR INPUT AS #1
30 USE
40   PRINT "Error caught: "; EXTEXT$
50 END WHEN
```

WHEN EXCEPTION IN establishes a protected block. USE begins the exception handler. END WHEN terminates the block. Inside the USE handler, EXTYPE returns the exception type code and EXTEXT$ returns the exception text. CAUSE EXCEPTION raises an exception. RETRY re-enters the protected block from the beginning. CONTINUE resumes at the statement after the one that caused the exception.

## 8. ERROR CODE REFERENCE

The following error codes are defined in engine/include/types/errors.h:

| Code | Name | Description |
|------|------|-------------|
| 0 | ERR_OK | No error |
| 1 | ERR_NEXT_WITHOUT_FOR | NEXT encountered without matching FOR |
| 2 | ERR_SYNTAX | Malformed statement or unexpected token |
| 3 | ERR_RETURN_WITHOUT_GOSUB | RETURN with no GOSUB on the stack |
| 4 | ERR_OUT_OF_DATA | READ with no remaining DATA items |
| 5 | ERR_ILLEGAL_FUNCTION_CALL | Argument out of valid range |
| 6 | ERR_OVERFLOW | Numeric result exceeds representable range |
| 7 | ERR_OUT_OF_MEMORY | Memory allocation failed |
| 8 | ERR_UNDEFINED_LINE | GOTO/GOSUB target line does not exist |
| 9 | ERR_SUBSCRIPT_OUT_OF_RANGE | Array index outside declared bounds |
| 10 | ERR_REDIM_ARRAY | Attempt to re-dimension a static array |
| 11 | ERR_DIVISION_BY_ZERO | Division or MOD with a zero divisor |
| 12 | ERR_ILLEGAL_DIRECT | Statement not allowed in immediate mode |
| 13 | ERR_TYPE_MISMATCH | Numeric operation on string or vice versa |
| 14 | ERR_OUT_OF_STRING_SPACE | String heap exhausted |
| 15 | ERR_STRING_TOO_LONG | String exceeds 255 characters |
| 16 | ERR_STRING_FORMULA_TOO_COMPLEX | Expression too deeply nested |
| 17 | ERR_CANNOT_CONTINUE | CONT after program modification |
| 18 | ERR_UNDEFINED_USER_FUNCTION | FN reference to undefined DEF FN |
| 19 | ERR_NO_RESUME | Error handler did not issue RESUME |
| 20 | ERR_RESUME_WITHOUT_ERROR | RESUME issued outside error handler |
| 24 | ERR_DEVICE_TIMEOUT | Device did not respond |
| 25 | ERR_DEVICE_FAULT | Device hardware failure |
| 27 | ERR_OUT_OF_PAPER | Printer out of paper |
| 29 | ERR_WHILE_WITHOUT_WEND | WHILE with no matching WEND |
| 30 | ERR_FOR_WITHOUT_NEXT | FOR with no matching NEXT |
| 35 | ERR_FIELD_NOT_DEFINED | FIELD referenced before definition |
| 52 | ERR_BAD_FILE_NUMBER | Channel not open or out of range |
| 53 | ERR_FILE_NOT_FOUND | Specified file does not exist |
| 54 | ERR_BAD_FILE_MODE | Operation incompatible with file mode |
| 55 | ERR_FILE_ALREADY_OPEN | Channel already in use |
| 56 | ERR_FIELD_OVERFLOW | FIELD total exceeds record length |
| 57 | ERR_DEVICE_IO_ERROR | Read or write operation failed |
| 58 | ERR_FILE_ALREADY_EXISTS | File exists and cannot be overwritten |
| 59 | ERR_BAD_RECORD_LENGTH | Record length mismatch |
| 61 | ERR_DISK_FULL | No free space on target volume |
| 62 | ERR_INPUT_PAST_END | Reading past end of file |
| 63 | ERR_BAD_RECORD_NUMBER | Record number out of range |
| 64 | ERR_BAD_FILE_NAME | Filename contains invalid characters |
| 67 | ERR_TOO_MANY_FILES | Maximum open file count (16) exceeded |
| 68 | ERR_DEVICE_UNAVAILABLE | Requested device not present |
| 70 | ERR_PERMISSION_DENIED | Operation blocked by security level |
| 71 | ERR_DISK_NOT_READY | Removable media not inserted |
| 72 | ERR_DISK_MEDIA_ERROR | Physical media read/write failure |
| 73 | ERR_ADVANCED_FEATURE_DISABLED | Feature not available in this edition |
| 74 | ERR_RENAME_ACROSS_DISKS | Cannot rename across drive boundaries |
| 75 | ERR_PATH_FILE_ACCESS_ERROR | Path access denied by OS |
| 76 | ERR_PATH_NOT_FOUND | Directory path does not exist |

## 9. ERROR CATEGORIES

Errors are classified into four categories defined by the BppErrorCategory enum:

ERR_CAT_SYNTAX covers parse-time errors (Error 2) where the lexer or parser encounters an unexpected token, a malformed expression, or an unterminated string literal.

ERR_CAT_RUNTIME covers execution-time errors including type mismatches, overflow, division by zero, undefined line references, and all control-flow stack violations (NEXT without FOR, RETURN without GOSUB, WEND without WHILE).

ERR_CAT_SYSTEM covers device, file, and operating system errors including all file I/O errors (52-76), device timeouts and faults (24-27), and permission denied (70).

ERR_CAT_INTERNAL covers compiler limits, VM panics, and internal fault conditions that indicate a bug in the interpreter rather than a problem in the user's program.

## 10. COMMON ERROR HANDLING PATTERNS

### Retry on failure

```basic
10 ON ERROR GOTO 100
20 OPEN "CONFIG.DAT" FOR INPUT AS #1
30 ON ERROR GOTO 0
40 REM ... continue with file
50 END
100 IF ERR = 53 THEN
110   PRINT "Creating default config..."
120   OPEN "CONFIG.DAT" FOR OUTPUT AS #1
130   PRINT #1, "DEFAULT=1"
140   CLOSE #1
150   RESUME
160 END IF
170 PRINT "Unexpected error:"; ERR$
180 END
```

### Skip and log errors

```basic
10 ON ERROR GOTO 100
20 FOR I = 1 TO 10
30   OPEN "FILE" + STR$(I) + ".DAT" FOR INPUT AS #1
40   INPUT #1, D$
50   CLOSE #1
60   PRINT D$
70 NEXT I
80 END
100 PRINT "Skipped file"; I; "- Error"; ERR$
110 RESUME NEXT
```

### TRY/CATCH for scoped handling

```basic
10 FOR I = 1 TO 100
20   TRY
30     X = 1000 / (I - 50)
40     PRINT I, X
50   CATCH
60     PRINT I, "Division by zero skipped"
70   END TRY
80 NEXT I
```

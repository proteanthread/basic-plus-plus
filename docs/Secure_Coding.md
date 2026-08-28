# BASIC++ v6.5.2 Secure Coding Practices

## 1. OVERVIEW

This guide covers secure coding practices for BASIC++ programs. While the security system (Security.md) controls what resources a program can access, secure coding practices address how a program handles data, validates input, and avoids common vulnerabilities.

## 2. INPUT VALIDATION

Never trust user input. Always validate type, length, and range before processing:

```basic
10 INPUT "Enter age (1-150): "; Age$
20 Age = VAL(Age$)
30 IF Age < 1 OR Age > 150 THEN
40   PRINT "Invalid age"
50   GOTO 10
60 END IF
```

For string input, check length before using the value:

```basic
10 LINE INPUT "Enter name (max 30 chars): "; Name$
20 IF LEN(Name$) > 30 THEN Name$ = LEFT$(Name$, 30)
30 IF LEN(Name$) = 0 THEN PRINT "Name required" : GOTO 10
```

## 3. PATH TRAVERSAL PREVENTION

When constructing file paths from user input, strip directory separators and parent references:

```basic
10 INPUT "Filename: "; F$
20 ' Remove path separators
30 FOR I = 1 TO LEN(F$)
40   C$ = MID$(F$, I, 1)
50   IF C$ = "/" OR C$ = "\" OR C$ = ":" THEN
60     PRINT "Invalid filename"
70     GOTO 10
80   END IF
90 NEXT I
100 IF INSTR(F$, "..") > 0 THEN PRINT "Invalid filename" : GOTO 10
110 OPEN F$ FOR INPUT AS #1
```

At security levels 2+ (STANDARD), the VFS automatically restricts file access to the current directory. But programs should validate input even at lower security levels.

## 4. ERROR HANDLING

Always handle errors. Unhandled errors in batch scripts (bs) produce exit code 1 and may expose internal state in error messages:

```basic
10 ON ERROR GOTO 9000
20 ' ... program logic ...
30 END
9000 ' Error handler
9010 PRINT "Error"; ERR; "at line"; ERL
9020 RESUME NEXT
```

For structured error handling, use TRY/CATCH to contain errors locally:

```basic
10 TRY
20   OPEN UserFile$ FOR INPUT AS #1
30 CATCH
40   PRINT "Cannot open file"
50   SYSTEM 1
60 END TRY
```

## 5. RESOURCE CLEANUP

Always close files and release resources on both success and error paths:

```basic
10 TRY
20   OPEN "DATA.TXT" FOR INPUT AS #1
30   ' ... process file ...
40   CLOSE #1
50 CATCH
60   CLOSE #1            ' Close even on error
70   PRINT "Processing failed"
80 END TRY
```

## 6. AVOIDING SHELL INJECTION

When constructing SHELL commands from user input, sanitize the input:

```basic
10 INPUT "Search term: "; Term$
20 ' Reject dangerous characters
30 FOR I = 1 TO LEN(Term$)
40   C$ = MID$(Term$, I, 1)
50   IF INSTR(";|&`$(){}[]<>!#", C$) > 0 THEN
60     PRINT "Invalid character in search term"
70     GOTO 10
80   END IF
90 NEXT I
100 SHELL "grep " + Term$ + " data.txt"
```

Better: avoid SHELL entirely and use BASIC++ file I/O to search:

```basic
10 INPUT "Search term: "; Term$
20 OPEN "data.txt" FOR INPUT AS #1
30 WHILE NOT EOF(1)
40   LINE INPUT #1, L$
50   IF INSTR(L$, Term$) > 0 THEN PRINT L$
60 WEND
70 CLOSE #1
```

## 7. SECURITY LEVEL RECOMMENDATIONS

| Use Case | Recommended Level |
|----------|------------------|
| Personal development | OPEN (0) or SAFE (1) |
| Student exercises | EDUCATIONAL (3) |
| CGI/web scripts | RESTRICTED (4) |
| Untrusted code execution | PARANOID (5) |
| Production batch processing | STANDARD (2) |

## 8. MODULE TRUST

Only load modules from trusted sources. Modules execute C code through the host interop layer. A malicious module could compromise the host system at security level 0. At level 3+, MODULE LOAD is denied.

## 9. DATA SANITIZATION FOR OUTPUT

When writing output that will be consumed by other systems (HTML, SQL, CSV), sanitize special characters:

```basic
' HTML output: escape < > & "
10 FUNCTION HtmlEscape$(s$)
20   Result$ = ""
30   FOR I = 1 TO LEN(s$)
40     C$ = MID$(s$, I, 1)
50     IF C$ = "<" THEN Result$ = Result$ + "&lt;"
60     ELSE IF C$ = ">" THEN Result$ = Result$ + "&gt;"
70     ELSE IF C$ = "&" THEN Result$ = Result$ + "&amp;"
80     ELSE IF C$ = CHR$(34) THEN Result$ = Result$ + "&quot;"
90     ELSE Result$ = Result$ + C$
100    END IF
110  NEXT I
120  HtmlEscape$ = Result$
130 END FUNCTION
```

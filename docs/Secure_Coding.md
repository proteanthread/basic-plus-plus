WRITING SAFE AND SECURE BASIC++ CODE
======================================
A Programmer's Guide to Defensive BASIC++

Version 4.0.1

TABLE OF CONTENTS
-----------------
  1.  Introduction
  2.  The Threat Model
  3.  Input Validation
  4.  Safe Numeric Handling
  5.  Safe String Handling
  6.  Defensive File I/O
  7.  Network Safety
  8.  Shell and System Safety
  9.  Memory and Resource Awareness
  10. Error Handling for Safety
  11. Security Levels in Practice
  12. Protecting Sensitive Data
  13. Safe Multi-File Programs (CHAIN / MERGE)
  14. Safe Module Usage
  15. Virtual Device Safety
  16. Coding Standards Checklist
  17. Common Vulnerabilities and Fixes
  18. Deployment Patterns
  19. Quick Reference Card


=====================================================================
1.  INTRODUCTION
=====================================================================

BASIC++ provides a security system (see Security.txt) that
controls which operations are allowed.  But the security system
only prevents dangerous operations -- it does not make your
CODE correct or safe.

This tutorial teaches you how to write BASIC++ programs that:

  * Validate all input before using it
  * Handle errors gracefully instead of crashing
  * Protect files and data from corruption
  * Avoid resource exhaustion (memory, string pool, stack)
  * Run safely in shared or untrusted environments
  * Resist common mistakes that cause data loss

The target audience is anyone writing BASIC++ programs that
will be run by others, deployed on shared systems, or used
to process real data.

PREREQUISITE READING:
  Security.txt             Security levels and sandboxing
  Error_Handling.txt        ON ERROR, ERR, ERL, RESUME
  File_IO.txt              File channels and modes


=====================================================================
2.  THE THREAT MODEL
=====================================================================

Before writing secure code, understand what you're defending
against:

2.1  Accidental Damage

  The most common threat.  Your own program has a bug:
    * Division by zero crashes a batch job
    * Unopened file channel corrupts output
    * Off-by-one in a loop overwrites good data
    * Missing CLOSE leaves a file locked or truncated

2.2  Bad Input

  Users type unexpected things:
    * Letters where you expect numbers
    * Extremely long strings that exhaust the pool
    * Empty input where you expect a value
    * Path traversal in filenames ("..\..\system.dat")

2.3  Resource Exhaustion

  Programs that run out of room:
    * Deep recursion overflows the 256-frame stack
    * String concatenation in a loop exhausts the pool
    * Too many open files (8-channel limit)
    * DIM arrays consuming all element pool space

2.4  Untrusted Code

  Running programs written by someone else:
    * SHELL commands that damage the system
    * KILL or RMDIR on critical files
    * POKE to arbitrary memory addresses
    * CHAIN to a malicious secondary program

The security level system (Security.txt) handles 2.4.  This
tutorial focuses on 2.1, 2.2, and 2.3 -- the problems that
the security system CANNOT solve for you.


=====================================================================
3.  INPUT VALIDATION
=====================================================================

Rule #1: NEVER trust INPUT.  Always validate before using.

3.1  Numeric Input Validation

  BAD -- crashes on non-numeric input:

    10 INPUT "Enter a number: "; N
    20 PRINT 100 / N

  GOOD -- validates and rejects bad input:

    10 INPUT "Enter a number: "; A$
    20 N = VAL(A$)
    30 IF A$ = "" THEN PRINT "No input." : GOTO 10
    40 IF N = 0 AND A$ <> "0" THEN PRINT "Not a number." : GOTO 10
    50 IF N = 0 THEN PRINT "Cannot divide by zero." : GOTO 10
    60 PRINT 100 / N

  KEY TECHNIQUE: Read everything as a string (A$), then convert
  with VAL().  This lets you detect non-numeric input without
  triggering a runtime error.

3.2  Range Checking

    10 INPUT "Enter age (1-120): "; A$
    20 AGE = VAL(A$)
    30 IF AGE < 1 OR AGE > 120 THEN
    40   PRINT "Invalid age.  Try again."
    50   GOTO 10
    60 END IF

3.3  String Length Validation

    10 INPUT "Enter name (max 30 chars): "; NM$
    20 IF LEN(NM$) = 0 THEN PRINT "Name required." : GOTO 10
    30 IF LEN(NM$) > 30 THEN
    40   PRINT "Name too long.  Truncating."
    50   NM$ = LEFT$(NM$, 30)
    60 END IF

3.4  Filename Validation

  CRITICAL: Never pass unvalidated user input to OPEN or KILL.

    10 INPUT "File to open: "; F$
    20 IF LEN(F$) = 0 THEN PRINT "No filename." : GOTO 10
    30 ' Block path traversal
    40 IF INSTR(F$, "..") > 0 THEN PRINT "Invalid path." : GOTO 10
    50 IF INSTR(F$, "/") > 0 THEN PRINT "Invalid path." : GOTO 10
    60 IF INSTR(F$, "\") > 0 THEN PRINT "Invalid path." : GOTO 10
    70 IF INSTR(F$, ":") > 0 THEN PRINT "Invalid path." : GOTO 10
    80 ' Only allow safe extensions
    90 IF RIGHT$(F$, 4) <> ".TXT" AND RIGHT$(F$, 4) <> ".DAT" THEN
    100   PRINT "Only .TXT and .DAT files allowed."
    110   GOTO 10
    120 END IF
    130 ON ERROR GOTO 500
    140 OPEN F$ FOR INPUT AS #1
    150 ON ERROR GOTO 0
    ...
    500 PRINT "Cannot open file: "; F$ : RESUME 10

  This blocks:
    * "..\secret.dat"    (path traversal)
    * "C:\windows\hosts" (absolute path)
    * "program.exe"      (dangerous extension)
    * ""                 (empty filename)

3.5  Menu Choice Validation

    10 PRINT "1. New Game"
    20 PRINT "2. Load Game"
    30 PRINT "3. Quit"
    40 INPUT "Choice: "; C$
    50 C = VAL(C$)
    60 IF C < 1 OR C > 3 THEN PRINT "Invalid choice." : GOTO 10
    70 ON C GOSUB 1000, 2000, 3000


=====================================================================
4.  SAFE NUMERIC HANDLING
=====================================================================

4.1  Division by Zero

  Always check the divisor before dividing:

    IF DIVISOR = 0 THEN
      PRINT "Cannot divide by zero."
    ELSE
      RESULT = DIVIDEND / DIVISOR
    END IF

4.2  Overflow Prevention

  BASIC++ integers are 'long' (typically 32-bit: -2147483648
  to 2147483647).  Watch for overflow in accumulations:

    ' BAD -- overflows silently on large data
    10 TOTAL = 0
    20 FOR I = 1 TO COUNT
    30   TOTAL = TOTAL + VALUES(I)
    40 NEXT I

    ' BETTER -- use floating point for large sums
    10 TOTAL# = 0.0
    20 FOR I = 1 TO COUNT
    30   TOTAL# = TOTAL# + VALUES(I)
    40 NEXT I

4.3  Safe Exponentiation

  Large exponents can overflow or take excessive time:

    10 INPUT "Base, Exponent: "; B, E
    20 IF E < 0 OR E > 20 THEN
    30   PRINT "Exponent must be 0-20."
    40   GOTO 10
    50 END IF
    60 PRINT B ^ E

4.4  Floating-Point Comparison

  Never use = to compare floating-point values:

    ' BAD
    IF X = 0.1 THEN PRINT "Match"

    ' GOOD -- use epsilon comparison
    EPSILON = 0.00001
    IF ABS(X - 0.1) < EPSILON THEN PRINT "Match"


=====================================================================
5.  SAFE STRING HANDLING
=====================================================================

5.1  String Pool Awareness

  BASIC++ uses a bump-allocated string pool.  Every string
  operation (concatenation, MID$, LEFT$, etc.) allocates new
  space.  The old string data is NOT freed until the pool resets
  (on RUN, NEW, or CLEAR).

  BAD -- exhausts string pool:

    10 A$ = ""
    20 FOR I = 1 TO 10000
    30   A$ = A$ + "*"         ' allocates new string each time!
    40 NEXT I

  Each iteration allocates a string 1 byte longer than the last.
  After 10000 iterations, the pool has consumed approximately
  50 million bytes of dead string data (1+2+3+...+10000).

  GOOD -- build in chunks or use a fixed buffer:

    10 CHUNK$ = "**********"   ' 10 chars
    20 A$ = ""
    30 FOR I = 1 TO 1000
    40   A$ = A$ + CHUNK$      ' 1000 iterations, not 10000
    50 NEXT I

  Or better -- use STRING$():

    10 A$ = STRING$(10000, "*")  ' single allocation

5.2  Monitoring Pool Usage

  Use FRE("") or SIZE to check available string space:

    10 AVAIL = FRE("")
    20 IF AVAIL < 1024 THEN
    30   PRINT "WARNING: Low string space ("; AVAIL; " bytes)"
    40   CLEAR                  ' reset string pool
    50 END IF

5.3  Defensive Concatenation

  When building strings in a loop, check available space:

    100 FOR I = 1 TO NLINES
    110   ON ERROR GOTO 900
    120   RESULT$ = RESULT$ + LINE$(I) + CHR$(13)
    130   ON ERROR GOTO 0
    140 NEXT I
    150 GOTO 200
    900 PRINT "String pool exhausted at line "; I
    910 RESUME 200
    200 ' Continue with partial result

5.4  Safe Substring Operations

  Always check string length before extracting substrings:

    ' BAD -- crashes if A$ is shorter than 5 chars
    B$ = MID$(A$, 3, 5)

    ' GOOD -- bounds-check first
    IF LEN(A$) >= 7 THEN
      B$ = MID$(A$, 3, 5)
    ELSE IF LEN(A$) >= 3 THEN
      B$ = MID$(A$, 3)         ' take whatever remains
    ELSE
      B$ = ""
    END IF

5.5  Trimming Input

  User input often has leading/trailing spaces:

    INPUT "Name: "; NM$
    NM$ = LTRIM$(RTRIM$(NM$))
    IF LEN(NM$) = 0 THEN PRINT "Empty name." : GOTO ...


=====================================================================
6.  DEFENSIVE FILE I/O
=====================================================================

File operations are the most common source of data loss.
Every file operation can fail, and every failure must be handled.

6.1  The Golden Rules

  1. Always use ON ERROR GOTO before OPEN
  2. Always CLOSE files when done (even on error paths)
  3. Never assume a file exists -- check first
  4. Never assume a write succeeded -- verify
  5. Write to a temporary file, then rename (atomic writes)

6.2  Safe File Reading

    100 ON ERROR GOTO 500
    110 OPEN "data.txt" FOR INPUT AS #1
    120 ON ERROR GOTO 0
    130 '
    140 ON ERROR GOTO 600
    150 WHILE NOT EOF(1)
    160   LINE INPUT #1, L$
    170   ' process L$
    180 WEND
    190 ON ERROR GOTO 0
    200 CLOSE #1
    210 GOTO 700
    500 PRINT "Cannot open data.txt"
    510 RESUME 700
    600 PRINT "Read error at line "; ERL
    610 CLOSE #1
    620 RESUME 700
    700 ' Continue program

  KEY: The error handler at line 600 calls CLOSE #1 before
  resuming.  This prevents file handle leaks.

6.3  Safe File Writing (Atomic Pattern)

  Never write directly to a critical file.  Write to a temp
  file first, then rename:

    100 ON ERROR GOTO 500
    110 OPEN "data.tmp" FOR OUTPUT AS #1
    120 FOR I = 1 TO COUNT
    130   PRINT #1, RECORDS$(I)
    140 NEXT I
    150 CLOSE #1
    160 '
    170 ' Only rename after successful write
    180 ON ERROR GOTO 600
    190 KILL "data.bak"           ' delete old backup
    200 ON ERROR GOTO 0
    210 NAME "data.txt" AS "data.bak"  ' current -> backup
    220 NAME "data.tmp" AS "data.txt"  ' temp -> current
    230 GOTO 700
    500 PRINT "Write failed!"
    510 CLOSE #1
    520 KILL "data.tmp"           ' clean up partial file
    530 RESUME 700
    600 ' Ignore error from deleting nonexistent backup
    610 RESUME NEXT
    700 ' Done

  This pattern guarantees:
    * If the write fails, "data.txt" is untouched
    * If power fails mid-write, "data.txt" is untouched
    * A backup ("data.bak") always exists after first save

6.4  File Channel Discipline

  BASIC++ has 8 file channels (#1 through #8).  Treat them like
  a limited resource:

    ' BAD -- opens channels without closing
    FOR I = 1 TO 100
      OPEN FILES$(I) FOR INPUT AS #1  ' forgets to CLOSE!
    NEXT I

    ' GOOD -- always close before reopen
    FOR I = 1 TO 100
      OPEN FILES$(I) FOR INPUT AS #1
      ' ... process file ...
      CLOSE #1
    NEXT I

6.5  EOF Checking

  Always check EOF before reading:

    ' BAD -- crashes on empty file
    10 OPEN "data.txt" FOR INPUT AS #1
    20 INPUT #1, A$              ' Error if file is empty!

    ' GOOD
    10 OPEN "data.txt" FOR INPUT AS #1
    20 IF EOF(1) THEN PRINT "File is empty." : CLOSE #1 : END
    30 INPUT #1, A$


=====================================================================
7.  NETWORK SAFETY
=====================================================================

When using BASIC++ network features (see Virtual_Network.txt),
additional precautions are needed.

7.1  Connection Timeouts

  Network connections can hang indefinitely.  Always set a
  timeout:

    10 VDEV OPEN "NET:", "tcp:example.com:80", 1
    20 VDEV IOCTL 1, "TIMEOUT", 5000    ' 5 second timeout
    30 ON ERROR GOTO 500
    40 VDEV WRITE 1, "GET / HTTP/1.0" + CHR$(13) + CHR$(10)
    50 ' ...
    500 PRINT "Network timeout or error."
    510 VDEV CLOSE 1
    520 RESUME 600

7.2  Validating Received Data

  Never trust data received from a network:

    ' Read response
    VDEV READ 1, RESPONSE$, 4096
    '
    ' Validate before parsing
    IF LEN(RESPONSE$) = 0 THEN PRINT "Empty response." : ...
    IF LEN(RESPONSE$) > 4096 THEN PRINT "Response too large." : ...
    '
    ' Check for expected format before extracting data
    IF LEFT$(RESPONSE$, 4) <> "HTTP" THEN
      PRINT "Invalid HTTP response."
    END IF

7.3  Don't Embed Credentials

  BAD -- password visible in source code:

    10 USER$ = "admin"
    20 PASS$ = "secret123"
    30 VDEV WRITE 1, "AUTH " + USER$ + " " + PASS$

  BETTER -- read from a separate config file:

    10 ON ERROR GOTO 500
    20 OPEN "credentials.dat" FOR INPUT AS #2
    30 INPUT #2, USER$
    40 INPUT #2, PASS$
    50 CLOSE #2
    60 VDEV WRITE 1, "AUTH " + USER$ + " " + PASS$
    70 ' Clear credentials from memory immediately
    80 USER$ = "" : PASS$ = ""

  BEST -- prompt the user at runtime:

    10 INPUT "Username: "; USER$
    20 INPUT "Password: "; PASS$    ' NOTE: visible on screen
    30 VDEV WRITE 1, "AUTH " + USER$ + " " + PASS$
    40 USER$ = "" : PASS$ = ""


=====================================================================
8.  SHELL AND SYSTEM SAFETY
=====================================================================

SHELL and SHELL$() execute operating system commands.  They are
the most dangerous functions in BASIC++.

8.1  Never Pass Unvalidated Input to SHELL

  BAD -- allows command injection:

    10 INPUT "Filename to list: "; F$
    20 SHELL "dir " + F$

  If the user types:    *.* & del *.*
  The SHELL executes:   dir *.* & del *.*

  GOOD -- validate first, or avoid SHELL entirely:

    10 INPUT "Filename: "; F$
    20 ' Only allow alphanumeric + dot + underscore
    30 FOR I = 1 TO LEN(F$)
    40   C$ = MID$(F$, I, 1)
    50   IF INSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._", UCASE$(C$)) = 0 THEN
    60     PRINT "Invalid character in filename."
    70     GOTO 10
    80   END IF
    90 NEXT I
    100 SHELL "dir " + F$

  BEST -- use BASIC++ file I/O instead of SHELL:

    10 INPUT "Filename: "; F$
    20 ON ERROR GOTO 500
    30 OPEN F$ FOR INPUT AS #1
    40 WHILE NOT EOF(1) : LINE INPUT #1, L$ : PRINT L$ : WEND
    50 CLOSE #1

8.2  Use the Security System

  If your program doesn't need SHELL access, lock it out early:

    10 SECURITY LEVEL 2         ' Blocks SHELL, KILL, CHDIR, etc.
    20 ' Rest of program cannot accidentally use SHELL


=====================================================================
9.  MEMORY AND RESOURCE AWARENESS
=====================================================================

BASIC++ runs within fixed memory pools.  Understanding the
limits prevents mysterious crashes.

9.1  Stack Depth (256 frames)

  Every GOSUB, FOR, WHILE, DO, and SUB/FUNCTION call pushes a
  stack frame.  Deep or runaway recursion overflows the stack.

  BAD -- unbounded recursion:

    1000 SUB Factorial(N)
    1010   IF N <= 1 THEN Factorial = 1 : EXIT SUB
    1020   Factorial = N * Factorial(N - 1)    ' recursive!
    1030 END SUB

  For N > 256, this overflows the stack.

  GOOD -- iterative when possible:

    1000 FUNCTION Factorial(N)
    1010   RESULT = 1
    1020   FOR I = 2 TO N
    1030     RESULT = RESULT * I
    1040   NEXT I
    1050   Factorial = RESULT
    1060 END FUNCTION

  If recursion is necessary, add a depth guard:

    1000 SUB TreeWalk(NODE, DEPTH)
    1010   IF DEPTH > 50 THEN PRINT "Too deep!" : EXIT SUB
    1020   ' ... process node ...
    1030   CALL TreeWalk(LEFT_CHILD, DEPTH + 1)
    1040   CALL TreeWalk(RIGHT_CHILD, DEPTH + 1)
    1050 END SUB

9.2  Program Line Limit (4096 lines)

  Large programs can hit the line limit.  Monitor with:

    PRINT "Lines used: "; SIZE("PROGRAM")

  Mitigation strategies:
    * Use SUB/FUNCTION to reduce line count
    * Use multi-statement lines (colon separator)
    * Split into multiple files with CHAIN or MERGE

9.3  Array Element Pool (8192 elements)

  All DIM arrays share a single element pool.  A single large
  array can consume the entire pool:

    DIM A(8000)      ' uses 8001 elements -- almost all of them!
    DIM B(100)       ' SORRY. Out of array space.

  Plan your array budgets:

    ' Document array usage
    DIM NAMES$(100)     ' 101 elements
    DIM SCORES(100)     ' 101 elements
    DIM GRID(50, 50)    ' 2601 elements
    '                     Total: 2803 of 8192

9.4  File Channel Limit (8 channels)

  Only 8 files can be open simultaneously.  In programs that
  process many files, close each one before opening the next:

    FOR I = 1 TO NFILES
      OPEN FILES$(I) FOR INPUT AS #1
      GOSUB 5000                   ' process
      CLOSE #1                     ' free the channel
    NEXT I


=====================================================================
10. ERROR HANDLING FOR SAFETY
=====================================================================

Proper error handling is the foundation of safe code.  See
Error_Handling.txt for the full reference.  This section covers
safety-specific patterns.

10.1  The Cleanup Handler Pattern

  When using resources (files, devices), errors must trigger
  cleanup:

    100 OPENED = 0
    110 ON ERROR GOTO 900
    120 OPEN "input.txt" FOR INPUT AS #1 : OPENED = 1
    130 OPEN "output.txt" FOR OUTPUT AS #2 : OPENED = 2
    140 '
    150 WHILE NOT EOF(1)
    160   LINE INPUT #1, L$
    170   PRINT #2, UCASE$(L$)
    180 WEND
    190 '
    200 CLOSE #2 : CLOSE #1 : OPENED = 0
    210 PRINT "Done."
    220 ON ERROR GOTO 0
    230 END
    900 ' --- CLEANUP HANDLER ---
    910 PRINT "Error "; ERR; " at line "; ERL
    920 IF OPENED >= 2 THEN CLOSE #2
    930 IF OPENED >= 1 THEN CLOSE #1
    940 OPENED = 0
    950 END

  The OPENED variable tracks which resources need cleanup.  The
  error handler closes only the resources that were successfully
  opened.

10.2  Guard Clauses

  Check preconditions at the top of SUBs and FUNCTIONs:

    1000 SUB ProcessRecord(INDEX)
    1010   ' Guard clauses
    1020   IF INDEX < 1 OR INDEX > MAX_RECORDS THEN
    1030     PRINT "Invalid record index: "; INDEX
    1040     EXIT SUB
    1050   END IF
    1060   IF LEN(RECORDS$(INDEX)) = 0 THEN
    1070     PRINT "Empty record at index: "; INDEX
    1080     EXIT SUB
    1090   END IF
    1100   ' ... process record ...
    1110 END SUB

10.3  Fail-Safe Defaults

  Initialize variables to safe defaults before entering sections
  that might error:

    RESULT = -1               ' safe default (indicates failure)
    ON ERROR GOTO 500
    ' ... computation that might fail ...
    RESULT = COMPUTED_VALUE   ' only set on success
    ON ERROR GOTO 0

    IF RESULT = -1 THEN
      PRINT "Computation failed, using default."
      RESULT = 0
    END IF


=====================================================================
11. SECURITY LEVELS IN PRACTICE
=====================================================================

The security system (Security.txt) is your safety net.  Use it.

11.1  Choosing a Level

  Ask yourself:

    Does my program need SHELL/EXEC?
      No  -> SECURITY LEVEL 2 or higher
      Yes -> SECURITY LEVEL 0 or 1 (validate inputs carefully)

    Does my program need file I/O?
      No  -> SECURITY LEVEL 3 (computation only)
      Yes -> SECURITY LEVEL 1 or 2

    Does my program need PEEK/POKE?
      No  -> SECURITY LEVEL 1 or higher
      Yes -> SECURITY LEVEL 0 (document why)

11.2  Set Security Early

  Always set the security level in the first few lines:

    10 SECURITY LEVEL 2       ' first executable line
    20 ' ... rest of program ...

  This ensures that even if a bug causes unexpected code paths,
  the security level is already in place.

11.3  Lock Down Untrusted Input Processing

  When processing user-supplied data files:

    10 SECURITY LEVEL 2         ' block system access
    20 ON ERROR GOTO 900        ' catch all errors
    30 INPUT "Data file: "; F$
    40 ' ... validate F$ (see section 3.4) ...
    50 OPEN F$ FOR INPUT AS #1
    60 ' ... process data ...
    70 CLOSE #1
    80 END
    900 PRINT "Error processing data: "; ERR
    910 CLOSE #1
    920 END

11.4  Security Level 3 for Computational Programs

  If your program only does math (no I/O), use level 3:

    10 SECURITY LEVEL 3
    20 ' Compute Fibonacci numbers
    30 INPUT "N: "; N
    40 A = 0 : B = 1
    50 FOR I = 1 TO N
    60   PRINT A;
    70   C = A + B : A = B : B = C
    80 NEXT I
    90 PRINT


=====================================================================
12. PROTECTING SENSITIVE DATA
=====================================================================

12.1  Clear Variables After Use

  Passwords, keys, and personal data should not linger in memory:

    INPUT "Password: "; PW$
    ' ... use PW$ for authentication ...
    PW$ = STRING$(LEN(PW$), "*")   ' overwrite contents
    PW$ = ""                        ' then clear

  Note: The old string data remains in the string pool until
  reset, but the variable no longer references it.

12.2  Don't LOG Sensitive Data

  BAD:
    PRINT >> "debug.log", "Password: "; PW$

  GOOD:
    PRINT >> "debug.log", "Auth attempt for user: "; USER$

12.3  Sanitize Output

  When displaying user-provided data, strip control characters:

    1000 SUB SafePrint(S$)
    1010   FOR I = 1 TO LEN(S$)
    1020     C = ASC(MID$(S$, I, 1))
    1030     IF C >= 32 AND C <= 126 THEN
    1040       PRINT MID$(S$, I, 1);
    1050     ELSE
    1060       PRINT "?";       ' replace non-printable
    1070     END IF
    1080   NEXT I
    1090   PRINT
    1100 END SUB


=====================================================================
13. SAFE MULTI-FILE PROGRAMS (CHAIN / MERGE)
=====================================================================

CHAIN loads and runs another BASIC program.  MERGE loads code
into the current program.  Both introduce trust boundaries.

13.1  Validate Before CHAINing

  Never CHAIN to a user-supplied filename without validation:

    ' Use the filename validation pattern from section 3.4
    ' THEN chain
    CHAIN "verified_program.bas"

13.2  Pass Data Safely

  Use COMMON to pass specific variables to CHAINed programs,
  rather than leaving all variables accessible:

    COMMON SHARED SCORE, LEVEL, PLAYER$
    CHAIN "next_level.bas"

13.3  Security Level Inheritance

  The security level is inherited by CHAINed programs.  If you
  set SECURITY LEVEL 2 before CHAIN, the chained program also
  runs at level 2 (and cannot lower it).  This is by design --
  use it:

    10 SECURITY LEVEL 2       ' sandbox
    20 CHAIN "untrusted.bas"  ' runs sandboxed


=====================================================================
14. SAFE MODULE USAGE
=====================================================================

Modules (External_Modules.txt) extend BASIC++ with new
capabilities.  Use them carefully.

14.1  Only Load What You Need

    ' BAD -- loads everything
    MODULE LOAD "USB"
    MODULE LOAD "NETWORK"
    MODULE LOAD "EXPERIMENTAL"

    ' GOOD -- load only what the program uses
    MODULE LOAD "USB"          ' needed for gamepad input

14.2  Check Module Availability

    ON ERROR GOTO 500
    MODULE LOAD "USB"
    ON ERROR GOTO 0
    HAS_USB = 1
    GOTO 100
    500 HAS_USB = 0
        RESUME 100
    100 IF NOT HAS_USB THEN PRINT "USB not available."


=====================================================================
15. VIRTUAL DEVICE SAFETY
=====================================================================

Virtual devices (Virtual_Devices.txt) provide I/O to hardware
and system resources.  Follow these rules:

15.1  Always Close Devices

    VDEV OPEN "SER:", "COM1:9600", 1
    ' ... communicate ...
    VDEV CLOSE 1               ' always close when done

15.2  Check Device Status Before Operations

    VDEV STATUS 1, S$
    IF INSTR(S$, "READY") = 0 THEN
      PRINT "Device not ready."
    ELSE
      VDEV WRITE 1, DATA$
    END IF

15.3  Handle Disconnection

  USB devices can be unplugged at any time:

    ON ERROR GOTO 800
    VDEV WRITE 1, COMMAND$
    ON ERROR GOTO 0
    GOTO 200
    800 PRINT "Device disconnected or error."
    810 VDEV CLOSE 1
    820 RESUME 200


=====================================================================
16. CODING STANDARDS CHECKLIST
=====================================================================

Use this checklist when reviewing BASIC++ code for safety:

  [ ] Security level set in the first 5 lines
  [ ] ON ERROR GOTO before every OPEN
  [ ] Every OPEN has a matching CLOSE (including error paths)
  [ ] All INPUT validated before use (type, range, length)
  [ ] No raw user input passed to SHELL or KILL
  [ ] Filenames validated (no path traversal, safe extensions)
  [ ] String concatenation loops bounded or pool-monitored
  [ ] Recursion has depth limits
  [ ] Division guarded against zero
  [ ] Array subscripts checked against DIM bounds
  [ ] Sensitive data cleared after use
  [ ] No credentials in source code
  [ ] Network data validated before parsing
  [ ] Timeouts set on network and device operations
  [ ] CLOSE called in error handlers for open resources
  [ ] Atomic write pattern used for critical files


=====================================================================
17. COMMON VULNERABILITIES AND FIXES
=====================================================================

  Vulnerability              Fix
  -------------------------  -----------------------------------
  Division by zero           Check divisor before dividing
  Unvalidated INPUT          Read as string, VAL(), range check
  Unclosed file              Add CLOSE to error handler
  String pool exhaustion     Use STRING$(), monitor FRE("")
  Stack overflow             Limit recursion depth, use loops
  Path traversal in OPEN     Validate filename (no ..\, /, :)
  Shell injection            Sanitize input or avoid SHELL
  Data loss on write error   Use temp file + rename pattern
  Credentials in source      Read from config file or prompt
  Crash on empty file        Check EOF before first read
  Wrong file mode            Verify mode matches usage
  Channel leak               Track open state, CLOSE on error
  Control char in output     Filter non-printable characters
  Runaway loop               Add iteration counter and limit
  Missing error handler      Add ON ERROR GOTO before risky ops


=====================================================================
18. DEPLOYMENT PATTERNS
=====================================================================

18.1  Self-Sandboxing Program

  The program locks itself down immediately:

    1 REM === SAFE CALCULATOR ===
    2 REM This program only does math.
    3 SECURITY LEVEL 3
    10 INPUT "Expression: "; E$
    20 ' ... parse and evaluate ...
    30 GOTO 10

18.2  Launcher with Sandbox

  A trusted launcher sets the security level, then runs
  untrusted code:

    1 REM === SECURE LAUNCHER ===
    5 SECURITY LEVEL 2
    10 INPUT "Program to run: "; P$
    20 ' ... validate P$ (section 3.4) ...
    30 CHAIN P$

18.3  Defensive Data Processor

  Reads an external data file with full error handling:

    1 REM === SAFE DATA PROCESSOR ===
    5 SECURITY LEVEL 2
    10 ON ERROR GOTO 900
    20 OPEN "input.csv" FOR INPUT AS #1
    30 OPEN "output.tmp" FOR OUTPUT AS #2
    40 LCOUNT = 0
    50 WHILE NOT EOF(1)
    60   LINE INPUT #1, L$
    70   LCOUNT = LCOUNT + 1
    80   IF LEN(L$) > 255 THEN L$ = LEFT$(L$, 255)
    90   GOSUB 500              ' process line
    100  PRINT #2, R$
    110 WEND
    120 CLOSE #2 : CLOSE #1
    130 NAME "output.tmp" AS "output.csv"
    140 PRINT LCOUNT; " lines processed."
    150 END
    500 ' --- Process one line ---
    510 R$ = UCASE$(L$)
    520 RETURN
    900 ' --- Error handler ---
    910 PRINT "Error "; ERR; " at line "; ERL
    920 CLOSE #2 : CLOSE #1
    930 KILL "output.tmp"
    940 END


=====================================================================
19. QUICK REFERENCE CARD
=====================================================================

  VALIDATE INPUT:
    INPUT "Value: "; A$
    N = VAL(A$)
    IF N < MIN OR N > MAX THEN ...

  SAFE DIVISION:
    IF D <> 0 THEN R = N / D ELSE R = 0

  SAFE FILE OPEN:
    ON ERROR GOTO handler
    OPEN file$ FOR mode AS #n
    ON ERROR GOTO 0

  ATOMIC WRITE:
    OPEN "file.tmp" FOR OUTPUT AS #1
    ' ... write ...
    CLOSE #1
    NAME "file.tmp" AS "file.dat"

  CLEANUP HANDLER:
    900 IF OPENED >= 1 THEN CLOSE #1
    910 END

  STRING POOL CHECK:
    IF FRE("") < 1024 THEN CLEAR

  STACK DEPTH GUARD:
    IF DEPTH > 50 THEN EXIT SUB

  SET SECURITY EARLY:
    1 SECURITY LEVEL 2

  SANITIZE FILENAMES:
    IF INSTR(F$, "..") > 0 THEN ... ' reject
    IF INSTR(F$, "\") > 0 THEN ...  ' reject
    IF INSTR(F$, "/") > 0 THEN ...  ' reject
    IF INSTR(F$, ":") > 0 THEN ...  ' reject

  CLEAR SENSITIVE DATA:
    PW$ = STRING$(LEN(PW$), "*") : PW$ = ""


=====================================================================
END OF DOCUMENT
=====================================================================

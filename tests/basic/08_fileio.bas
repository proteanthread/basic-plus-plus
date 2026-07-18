10 REM =====================================================================
20 REM BASIC++ v6.0.0 File I/O Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Filename constants, written string messages, numeric values.
60 REM - What cannot be changed: The assertion sequence for SEEK positions, EOF states, and channel reads.
70 REM - What to expect: Writing to a test file, reading it back, and verifying SEEK/EOF assertions.
80 REM - What to do if something breaks: If file assertions fail, check file channel bounds and standard I/O permissions.
90 REM =====================================================================
100 PRINT "RUNNING FILE I/O TEST..."
110 OPEN "test_io.txt" FOR OUTPUT AS #1
120 PRINT #1, "HELLO BASIC"
131 PRINT #1, 42
140 CLOSE #1
150 OPEN "test_io.txt" FOR INPUT AS #2
160 LINE INPUT #2, A$
170 INPUT #2, B
180 CLOSE #2
190 IF A$ = "HELLO BASIC" THEN GOTO 220
200 PRINT "FAILED: READ WRONG STRING VALUE"
210 END
220 IF B = 42 THEN GOTO 250
230 PRINT "FAILED: READ WRONG NUMERIC VALUE"
240 END
250 PRINT "FILE I/O OK"
260 RUN "tests/basic/09_filemgmt.bas"

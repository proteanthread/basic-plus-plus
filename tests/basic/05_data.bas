10 REM =====================================================================
20 REM BASIC++ v6.0.0 DATA/READ/RESTORE Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: DATA literal values, variable names, assertions.
60 REM - What cannot be changed: Expected reading order and line RESTORE values.
70 REM - What to expect: Execution verifies reading literals, unquoted string
80 REM   literals, RESTORE to start, and RESTORE to specific line. Prints "DATA OK".
90 REM - What to do if something breaks: Trace vm_build_data_table or data_ptr
100 REM   increments in src/statements/stmt_data.c.
110 REM =====================================================================
120 PRINT "RUNNING DATA TEST..."
130 DATA 10, 20, HELLO, 30
140 READ A, B, C$, D
150 IF A = 10 THEN GOTO 180
160 PRINT "FAILED: READ numeric literal A"
170 END
180 IF B = 20 THEN GOTO 210
190 PRINT "FAILED: READ numeric literal B"
200 END
210 IF C$ = "HELLO" THEN GOTO 240
220 PRINT "FAILED: READ unquoted string literal C$"
230 END
240 IF D = 30 THEN GOTO 270
250 PRINT "FAILED: READ numeric literal D"
260 END
270 RESTORE
280 READ X, Y, Z$, W
290 IF X = 10 THEN GOTO 320
300 PRINT "FAILED: RESTORE to start"
310 END
320 DATA 999
330 RESTORE 320
340 READ V
350 IF V = 999 THEN GOTO 380
360 PRINT "FAILED: RESTORE to line number"
370 END
380 PRINT "DATA OK"
390 RUN "tests/basic/06_error.bas"

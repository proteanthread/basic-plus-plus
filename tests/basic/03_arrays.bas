10 REM =====================================================================
20 REM BASIC++ v6.0.0 Arrays Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Bounds, variable names, test strings.
60 REM - What cannot be changed: Expected UBOUND values and assertion checks.
70 REM - What to expect: Execution verifies DIM, assignment, multi-dimensional
80 REM   string arrays, UBOUND, and ERASE. Prints "ARRAYS OK" on success.
90 REM - What to do if something breaks: If indices are incorrect, verify
100 REM   flat index mappings in src/runtime/arrays.c.
110 REM =====================================================================
120 PRINT "RUNNING ARRAYS TEST..."
130 DIM A(10)
140 LET A(5) = 42
150 IF A(5) = 42 THEN GOTO 180
160 PRINT "FAILED: 1D Array assignment"
170 END
180 DIM B$(3, 3)
190 LET B$(2, 2) = "HELLO"
200 IF B$(2, 2) = "HELLO" THEN GOTO 230
210 PRINT "FAILED: 2D String Array assignment"
220 END
230 IF UBOUND(A) = 10 THEN GOTO 260
240 PRINT "FAILED: UBOUND 1D"
250 END
260 IF UBOUND(B$, 2) = 3 THEN GOTO 290
270 PRINT "FAILED: UBOUND 2D"
280 END
290 ERASE A, B$
300 PRINT "ARRAYS OK"
310 RUN "tests/basic/04_options.bas"

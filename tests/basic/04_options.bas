10 REM =====================================================================
20 REM BASIC++ v6.0.0 Options Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Option base values, variable names, assertions.
60 REM - What cannot be changed: Expected UBOUND values and explicit checks.
70 REM - What to expect: Execution verifies OPTION BASE 1 (indexing 1 to 5),
80 REM   verifies OPTION EXPLICIT with DIM declarations, and prints "OPTIONS OK".
90 REM - What to do if something breaks: If base checks fail, verify get_flat_index
100 REM   in src/runtime/arrays.c. If explicit declarations fail, check var_lookup
110 REM   and var_set_explicit in src/runtime/variables.c.
120 REM =====================================================================
130 PRINT "RUNNING OPTIONS TEST..."
140 OPTION BASE 1
150 DIM A(5)
160 LET A(1) = 10
170 LET A(5) = 50
180 IF A(1) = 10 THEN GOTO 210
190 PRINT "FAILED: OPTION BASE 1 lower bound"
200 END
210 IF A(5) = 50 THEN GOTO 240
220 PRINT "FAILED: OPTION BASE 1 upper bound"
230 END
240 IF UBOUND(A) = 5 THEN GOTO 270
250 PRINT "FAILED: OPTION BASE 1 UBOUND"
260 END
270 OPTION EXPLICIT ON
280 DIM X
290 LET X = 100
300 IF X = 100 THEN GOTO 330
310 PRINT "FAILED: OPTION EXPLICIT scalar declaration"
320 END
330 DIM Y%
340 LET Y% = 200
350 IF Y% = 200 THEN GOTO 380
360 PRINT "FAILED: OPTION EXPLICIT integer declaration"
370 END
380 PRINT "OPTIONS OK"
390 RUN "tests/basic/05_data.bas"

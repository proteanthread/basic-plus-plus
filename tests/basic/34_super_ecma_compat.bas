10 REM =========================================================================
20 REM What can be changed: Loop bounds.
30 REM What cannot be changed: Arithmetic assertions.
40 REM What to expect: Verification of OPTION ARITHMETIC DECIMAL and BY STEP loops.
50 REM What to do if something breaks: Check eval.c rounding and stmt_loop.c parsing.
60 REM =========================================================================
100 PRINT "RUNNING TYMSHARE SUPER BASIC & ECMA-116 COMPATIBILITY TESTS (PHASE 22)..."
110 
120 REM 1. Legacy ::DIALECT directives removed (use DIALECT LOAD/REGISTER instead)
130 REM Previously: ::DIALECT "SBAS" and ::DIALECT "ECMA"
140 REM These tests use standard BASIC++ syntax throughout
150 
160 REM 2. Test OPTION ARITHMETIC DECIMAL rounding
170 OPTION ARITHMETIC DECIMAL
180 LET X = 0.1
190 LET Y = 0.2
200 LET Z = X + Y
210 PRINT "   0.1 + 0.2 under OPTION ARITHMETIC DECIMAL: "; Z
220 IF Z <> 0.3 THEN PRINT "FAIL: Decimal rounding failed": END
230 
240 REM 3. Test reversed loop stepping (BY STEP)
250 LET LSUM = 0
260 FOR I = 1 BY STEP 2 TO 5
270   LET LSUM = LSUM + I
280 NEXT I
290 PRINT "   Loop sum (1 BY STEP 2 TO 5): "; LSUM
300 IF LSUM <> 9 THEN PRINT "FAIL: Reversed step loop execution failed (expected 9, got "; LSUM; ")": END
310 RUN "tests/basic/35_joss_postfix_loops.bas"
320 PRINT "Tymshare Super BASIC & ECMA-116 Compatibility: PASS"
330 PRINT "ALL REGRESSION TESTS COMPLETED SUCCESSFULLY!"
340 END

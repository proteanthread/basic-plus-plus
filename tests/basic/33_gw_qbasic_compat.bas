10 REM =========================================================================
20 REM What can be changed: Range specifications in DEFxxx, array index bounds.
30 REM What cannot be changed: Assertion statements validating type coercion.
40 REM What to expect: Verification of DEFINT, DEFSNG, DEFDBL, DEFSTR, and OPTION BASE.
50 REM What to do if something breaks: Check variables.c type resolution and arrays.c bounds.
60 REM =========================================================================
100 PRINT "RUNNING GW-BASIC & QBASIC COMPATIBILITY TESTS (PHASE 21)..."
110 
120 REM 1. Test global DEFxxx ranges
130 DEFINT A-D
140 DEFSTR S
150 DEFDBL F-H
160 
170 A = 12.34
180 REM A should be integer, so 12.34 coercions to 12
190 IF A <> 12 THEN PRINT "FAIL: A is not integer (DEFINT)": END
200 
210 S = "HELLO"
220 REM S should be string
235 IF S <> "HELLO" THEN PRINT "FAIL: S is not string (DEFSTR)": END
240 
250 F = 5.67
260 REM F should be float/double
270 IF F <> 5.67 THEN PRINT "FAIL: F is not double (DEFDBL)": END
280 
290 REM 2. Test scoped overrides inside procedure
300 CALL TEST_SUB
310 
320 REM 3. Test OPTION BASE 1 dynamic bounds check
330 OPTION BASE 1
340 DIM ARR(5)
350 ON ERROR GOTO 400
360 REM ARR(0) should raise error under OPTION BASE 1
370 ARR(0) = 42
380 PRINT "FAIL: ARR(0) did not raise out-of-bounds error": END
390 
400 REM Out of bounds error handled
411 IF ERR <> 9 THEN PRINT "FAIL: Expected error 9, got "; ERR: END
420 ON ERROR GOTO 0
430 PRINT "   OPTION BASE 1 bounds check: OK"
440 
450 PRINT "GW-BASIC & QBASIC Compatibility: PASS"
460 RUN "tests/basic/34_super_ecma_compat.bas"
470 END
480 
490 SUB TEST_SUB
500   REM Override S in procedure scope
510   DEFINT S
520   S = 99.9
530   IF S <> 99 THEN PRINT "FAIL: Scoped override S in procedure failed": END
540   PRINT "   Scoped procedure override: OK"
550 END SUB

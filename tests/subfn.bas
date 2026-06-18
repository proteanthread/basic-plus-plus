10 REM ============================================================
20 REM  TEST: subfn.bas - Subroutines and Functions
30 REM ============================================================
40 REM  Tests three calling mechanisms:
50 REM   - DEF FN: single-line user-defined function (FNA)
60 REM   - CALL SUB: named subroutine with parameter
70 REM   - FUNCTION: named function with return value
80 REM
82 REM  EXPECTED: FNA(5) = 25, MYSUB prints "Inside MYSUB,
84 REM  A = 10", MYFUNC(5) = 10. No error messages.
86 REM  Ends with "Sub/Function tests completed."
88 REM ============================================================
90 REM
100 PRINT "Testing SUBs and FUNCTIONs..."
110 REM
120 REM --- DEF FN: inline function definition ---
130 REM  Defines FNA(X) = X * X (square function).
140 REM  FNA(5) should return 25.
150 DEF FNA(X) = X * X
160 IF FNA(5) <> 25 THEN PRINT "DEF FN ERROR" : END
170 REM
180 REM --- CALL SUB: named subroutine ---
190 REM  MYSUB(10) should print the value of A inside
200 REM  the subroutine. SUB is defined at line 1000.
210 CALL MYSUB(10)
220 REM
230 REM --- FUNCTION: named function with return ---
240 REM  MYFUNC(5) should return 5 * 2 = 10.
250 REM  FUNCTION is defined at line 1400.
260 IF MYFUNC(5) <> 10 THEN PRINT "FUNCTION ERROR"
270 REM
280 PRINT "Sub/Function tests completed."
290 END
300 REM
1000 REM --- SUB definition ---
1010 REM  MYSUB receives one parameter A and prints it.
1020 SUB MYSUB(A)
1030   PRINT "Inside MYSUB, A = "; A
1040 END SUB
1050 REM
1400 REM --- FUNCTION definition ---
1410 REM  MYFUNC receives one parameter B, returns B * 2.
1420 REM  Return value is assigned by setting MYFUNC = expr.
1430 FUNCTION MYFUNC(B)
1440   MYFUNC = B * 2
1450 END FUNCTION

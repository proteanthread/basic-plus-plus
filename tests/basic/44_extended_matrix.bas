10 REM =====================================================================
20 REM BASIC++ v6.0.0 Extended Matrix Math Integration Test
30 REM =====================================================================
40 REM - What can be changed: DATA values, equation scalar numbers.
50 REM - What cannot be changed: Matrix operations sequence, array names.
60 REM - What to expect: Performs matrix calculations and tests expression
70 REM   scalar multiplication. Chains to next test.
80 REM - What to do if something breaks: Trace solvers in src/statements/stmt_mat.c.
90 REM =====================================================================
100 PRINT "RUNNING EXTENDED MATRIX MATH TESTS..."
110 DIM A(2, 2)
120 DIM B(2, 2)
130 DIM C(2, 2)
140 MAT READ A
150 MAT READ B
160 REM Test MAT copy B = A
170 MAT B = A
180 REM Test expression scalar multiplication: MAT C = (A(1, 1) * 3 + 2) * B
190 MAT C = (A(1, 1) * 3 + 2.0) * B
200 MAT PRINT C;
210 REM Expected C(1, 1) = (2 * 3 + 2) * 2 = 8 * 2 = 16
220 IF C(1, 1) = 16 THEN GOTO 250
230 PRINT "FAILED: Extended scalar multiplication evaluation mismatch: "; C(1, 1)
240 END
250 PRINT "EXTENDED MATRIX MATH TESTS OK"
260 RUN "tests/basic/45_network_protocols.bas"
275 DATA 2, 4
280 DATA 6, 8
290 DATA 0, 0
300 DATA 0, 0

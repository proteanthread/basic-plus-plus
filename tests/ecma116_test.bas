10 REM ============================================================
20 REM  TEST: ecma116_test.bas - ECMA-116 Compliance & Exceptions
30 REM ============================================================
40 PRINT "Testing ECMA-116 Exceptions and Matrices..."
50 REM DIALECT "ECMA-116"
60 OPTION BASE 1
70 REM --- Test 1: WHEN EXCEPTION IN block ---
80 E = 0
90 WHEN EXCEPTION IN
100   PRINT "Inside protected block"
110   REM Trigger division by zero
120   A = 10 / 0
130 USE
140   PRINT "Exception caught! Code="; ERR; " Line="; ERL
150   E = 1
160   CONTINUE
170 END WHEN
180 IF E <> 1 THEN PRINT "EXCEPTION HANDLING FAILED" : END
190 REM --- Test 2: Matrix Identity and Transpose ---
200 DIM M1(3,3), M2(3,3), M3(3,3)
210 MAT M1 = IDN
220 REM Verify identity matrix values
230 IF M1(1,1) <> 1.0 OR M1(1,2) <> 0.0 OR M1(2,2) <> 1.0 THEN PRINT "MAT IDN ERROR" : END
240 M2(1,1) = 1 : M2(1,2) = 2 : M2(1,3) = 3
250 M2(2,1) = 4 : M2(2,2) = 5 : M2(2,3) = 6
260 M2(3,1) = 7 : M2(3,2) = 8 : M2(3,3) = 9
270 MAT M3 = TRN(M2)
280 IF M3(1,2) <> 4 OR M3(2,1) <> 2 THEN PRINT "MAT TRN ERROR" : END
290 REM --- Test 3: Matrix Inversion and Singularity ---
300 DIM A(2,2), B(2,2)
310 A(1,1) = 4 : A(1,2) = 7
320 A(2,1) = 2 : A(2,2) = 6
330 MAT B = INV(A)
340 REM Inverse of [4 7; 2 6] is [0.6 -0.7; -0.2 0.4]
350 REM Let's check a few values:
360 IF ABS(B(1,1) - 0.6) > 0.001 OR ABS(B(1,2) - (-0.7)) > 0.001 THEN PRINT "MAT INV ERROR" : END
370 REM Test singular matrix exception
380 DIM S(2,2), SI(2,2)
390 S(1,1) = 1 : S(1,2) = 2
400 S(2,1) = 2 : S(2,2) = 4
410 ES = 0
420 WHEN EXCEPTION IN
430   MAT SI = INV(S)
440 USE
450   PRINT "Caught singular matrix exception!"
460   ES = 1
470   CONTINUE
480 END WHEN
490 IF ES <> 1 THEN PRINT "MAT SINGULAR EXCEPTION FAILED" : END
500 PRINT "ECMA-116 tests completed successfully."
510 END

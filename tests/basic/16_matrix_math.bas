10 REM =====================================================================
20 REM BASIC++ v6.0.0 Matrix Math MAT Operations Integration Test
30 REM =====================================================================
40 REM - What can be changed: DATA numbers, print statements.
50 REM - What cannot be changed: Matrix dimensions, operation sequence.
60 REM - What to expect: Performs matrix calculations and prints results.
70 REM - What to do if something breaks: Trace solvers in src/statements/stmt_mat.c.
80 REM =====================================================================
90 PRINT "RUNNING MATRIX MATH TEST..."
100 DIM A(3, 3)
110 DIM B(3, 3)
120 DIM C(3, 3)
130 MAT READ A
140 MAT READ B
150 PRINT "MAT C = A + B"
160 MAT C = A + B
170 MAT PRINT C;
180 PRINT "MAT C = A - B"
190 MAT C = A - B
200 MAT PRINT C;
210 PRINT "MAT C = A * B"
220 MAT C = A * B
230 MAT PRINT C;
240 PRINT "MAT C = (2) * A"
250 MAT C = (2.0) * A
260 MAT PRINT C;
270 PRINT "MAT C = TRN(A)"
280 MAT C = TRN(A)
290 MAT PRINT C;
300 PRINT "MAT C = INV(A)"
310 MAT C = INV(A)
320 MAT PRINT C;
330 PRINT "MAT C = ZER"
340 MAT C = ZER
350 MAT PRINT C;
360 PRINT "MAT C = CON"
370 MAT C = CON
380 MAT PRINT C;
390 PRINT "MAT C = IDN"
400 MAT C = IDN
410 MAT PRINT C;
420 PRINT "MATRIX MATH OK"
430 RUN "tests/basic/17_program_edit.bas"
440 REM --- DATA for matrix A ---
450 DATA 2, 0, 0
460 DATA 0, 4, 0
470 DATA 0, 0, 8
480 REM --- DATA for matrix B ---
495 DATA 1, 2, 3
500 DATA 4, 5, 6
510 DATA 7, 8, 9

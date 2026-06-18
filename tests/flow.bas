10 REM ============================================================
20 REM  TEST: flow.bas - Flow Control
30 REM ============================================================
40 REM  Tests all loop and branching constructs:
50 REM   - FOR/NEXT with STEP (summation 1..10 = 55)
60 REM   - WHILE/WEND (count up to 5)
70 REM   - DO/LOOP UNTIL (count up to 3)
80 REM   - SELECT CASE with numeric match
90 REM
92 REM  EXPECTED: No error messages printed. FOR loop sums
94 REM  to 55, WHILE counts to 5, DO counts to 3, SELECT
96 REM  CASE prints "GOOD" for case 2. Ends with
97 REM  "Flow control tests completed."
98 REM ============================================================
99 REM
100 PRINT "Testing Flow Control..."
110 REM
120 REM --- FOR/NEXT loop ---
130 REM  Sum integers 1 through 10 with STEP 1.
140 REM  Expected result: SUM = 1+2+...+10 = 55.
150 SUM = 0
160 FOR I = 1 TO 10 STEP 1
170   SUM = SUM + I
180 NEXT I
190 IF SUM <> 55 THEN PRINT "FOR LOOP ERROR" : END
200 REM
210 REM --- WHILE/WEND loop ---
220 REM  Increment X from 0 while X < 5.
230 REM  Expected result: X = 5 after loop exits.
240 X = 0
250 WHILE X < 5
260  X = X + 1
270 WEND
280 IF X <> 5 THEN PRINT "WHILE LOOP ERROR"
290 REM
300 REM --- DO/LOOP UNTIL ---
310 REM  Increment Y from 0, loop until Y = 3.
320 REM  Expected result: Y = 3 after loop exits.
330 Y = 0
340 DO
350  Y = Y + 1
360 LOOP UNTIL Y = 3
370 IF Y <> 3 THEN PRINT "DO LOOP ERROR"
380 REM
390 REM --- SELECT CASE ---
400 REM  Z=2 should match CASE 2 and print "GOOD".
410 REM  CASE 1 and CASE ELSE should be skipped.
420 Z = 2
430 SELECT CASE Z
440  CASE 1: PRINT "BAD"
450  CASE 2: PRINT "GOOD"
460  CASE ELSE: PRINT "BAD"
470 END SELECT
480 REM
490 PRINT "Flow control tests completed."
500 END

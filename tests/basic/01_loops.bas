10 REM =====================================================================
20 REM BASIC++ v6.0.0 Loop Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Loop bounds, variable names, print logs.
60 REM - What cannot be changed: Expected sums and termination bounds.
70 REM - What to expect: Execution sums 1-5 in FOR loop (15), checks WHILE,
80 REM   runs DO UNTIL loop, and prints "LOOPS OK".
90 REM - What to do if something breaks: If sums are incorrect, check the
100 REM   loop variable assignment or stack push/pop in stmt_loop.c.
110 REM =====================================================================
120 PRINT "RUNNING LOOPS TEST..."
130 LET S = 0
140 FOR I = 1 TO 5
150 LET S = S + I
160 NEXT I
170 IF S = 15 THEN GOTO 200
180 PRINT "FAILED: FOR loop sum was "; S
190 END
200 PRINT "FOR LOOP PASSED"
210 LET W = 1
220 WHILE W < 4
230 LET W = W + 1
240 WEND
250 IF W = 4 THEN GOTO 280
260 PRINT "FAILED: WHILE loop counter was "; W
270 END
280 PRINT "WHILE LOOP PASSED"
290 LET D = 0
300 DO
310 LET D = D + 1
320 LOOP UNTIL D = 3
330 IF D = 3 THEN GOTO 360
340 PRINT "FAILED: DO UNTIL loop counter was "; D
350 END
360 PRINT "LOOPS OK"
370 RUN "tests/basic/02_functions.bas"

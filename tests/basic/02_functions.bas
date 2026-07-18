10 REM =====================================================================
20 REM BASIC++ v6.0.0 Functions Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Test values, print statements, string messages.
60 REM - What cannot be changed: Expected function output assertions.
70 REM - What to expect: Execution verifies math and string functions (SQR,
80 REM   ABS, LEN, ASC, CHR$, LEFT$, RIGHT$, MID$, INSTR, VAL, STR$) and
90 REM   prints "FUNCTIONS OK" on success.
100 REM - What to do if something breaks: Check the function implementation
110 REM   and string memory releases in src/expression/eval.c.
120 REM =====================================================================
130 PRINT "RUNNING FUNCTIONS TEST..."
140 IF SQR(9) = 3 THEN GOTO 170
150 PRINT "FAILED: SQR(9)"
160 END
170 IF ABS(-5) = 5 THEN GOTO 200
180 PRINT "FAILED: ABS(-5)"
190 END
200 IF LEN("HELLO") = 5 THEN GOTO 230
210 PRINT "FAILED: LEN"
220 END
230 IF ASC("A") = 65 THEN GOTO 260
240 PRINT "FAILED: ASC"
250 END
260 IF CHR$(66) = "B" THEN GOTO 290
270 PRINT "FAILED: CHR$"
280 END
290 IF LEFT$("BASIC", 3) = "BAS" THEN GOTO 320
300 PRINT "FAILED: LEFT$"
310 END
320 IF RIGHT$("BASIC", 2) = "IC" THEN GOTO 350
330 PRINT "FAILED: RIGHT$"
340 END
350 IF MID$("BASIC", 2, 3) = "ASI" THEN GOTO 380
360 PRINT "FAILED: MID$"
370 END
380 IF INSTR("BASIC", "SI") = 3 THEN GOTO 410
390 PRINT "FAILED: INSTR"
400 END
410 IF VAL("12.34") = 12.34 THEN GOTO 440
420 PRINT "FAILED: VAL"
430 END
440 IF STR$(5.5) = " 5.5" THEN GOTO 470
450 PRINT "FAILED: STR$"
460 END
470 PRINT "FUNCTIONS OK"
480 RUN "tests/basic/03_arrays.bas"

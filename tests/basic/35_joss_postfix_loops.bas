10 REM =====================================================================
20 REM BASIC++ JOSS Postfix Conditionals & Loops Integration Test
30 REM =====================================================================
40 REM - What can be changed: Range values or print statement outputs.
50 REM - What cannot be changed: The exact control flow structures and assertions.
60 REM - What to expect: Verification of postfix IF/UNLESS, DEMAND, EH option, and loops.
70 REM - What to do if something breaks: Verify parsing in src/vm/vm.c and loops in stmt_loop.c.
80 REM =====================================================================
90 PRINT "RUNNING JOSS & POSTFIX INTEGRATION TESTS..."
100 REM 1. Test postfix IF and UNLESS statement modifiers
110 X = 10
120 PRINT "POSTFIX IF OK" IF X = 10
130 PRINT "POSTFIX IF ERROR" IF X = 5
140 PRINT "POSTFIX UNLESS OK" UNLESS X = 5
150 PRINT "POSTFIX UNLESS ERROR" UNLESS X = 10
151 Y = 5
152 PRINT "POSTFIX MULTI OK" IF X = 10 IF Y = 5
153 PRINT "POSTFIX MULTI SKIP" IF X = 10 IF Y = 10
154 PRINT "POSTFIX MULTI UNLESS OK" IF X = 10 UNLESS Y = 10
155 IF X = 10 THEN PRINT "BRANCH POSTFIX OK" IF Y = 5 ELSE PRINT "BRANCH POSTFIX FAIL" IF Y = 5
160 REM 2. Test OPTION EH (error overriding)
170 OPTION EH ON
180 ON ERROR GOTO 210
190 X = 1 / 0 : REM Trigger divide by zero (or any error)
200 GOTO 240
210 REM Error handler
220 PRINT "ERR MSG: "; ERR$
230 RESUME 240
240 ON ERROR GOTO 0
250 OPTION EH OFF
260 REM 3. Test multi-variable FOR loops
270 PRINT "NESTED MULTI-VAR FOR LOOP:"
280 FOR A, B = 1 TO 2
290 PRINT "A="; A; " B="; B
300 NEXT B, A
310 REM 4. Test JOSS-style discrete range/values loop
320 PRINT "JOSS DISCRETE FOR LOOP:"
330 FOR I = 1(2)5, 10
340 PRINT "I="; I
350 NEXT I
360 PRINT "JOSS & POSTFIX OK"
370 RUN "tests/basic/36_rpn_calculator.bas"

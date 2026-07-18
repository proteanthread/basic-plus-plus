10 REM =====================================================================
20 REM What can be changed: Numeric values and test assertions.
30 REM What cannot be changed: The exact stack operator names and structure.
40 REM What to expect: Verification of DUP, DROP, SWAP, OVER, ROT, CLEAR, DEPTH, PICK, ROLL.
50 REM What to do if something breaks: Check RPN execution in src/expression/eval.c.
60 REM =====================================================================
70 PRINT "RUNNING RPN STACK OPERATORS REGRESSION TESTS..."
80 REM Test DUP
90 A = { 5 DUP * }
100 PRINT "5 DUP * ="; A
110 IF A <> 25 THEN PRINT "FAILED DUP": END
120 REM Test SWAP & DROP
130 B = { 3 4 SWAP DROP }
140 PRINT "3 4 SWAP DROP ="; B
150 IF B <> 4 THEN PRINT "FAILED SWAP/DROP": END
160 REM Test OVER
170 C = { 2 3 OVER + + }
180 PRINT "2 3 OVER + + ="; C
190 IF C <> 7 THEN PRINT "FAILED OVER": END
200 REM Test ROT
210 D = { 1 2 3 ROT + + }
220 PRINT "1 2 3 ROT + + ="; D
230 IF D <> 6 THEN PRINT "FAILED ROT": END
240 REM Test DEPTH
250 E = { 10 20 DEPTH }
260 PRINT "10 20 DEPTH (should be 2) ="; E
270 IF E <> 2 THEN PRINT "FAILED DEPTH": END
280 REM Test PICK
290 F = { 10 20 30 1 PICK }
300 PRINT "10 20 30 1 PICK (should copy 20) ="; F
310 IF F <> 20 THEN PRINT "FAILED PICK": END
320 REM Test ROLL
330 G = { 10 20 30 2 ROLL + }
340 PRINT "10 20 30 2 ROLL + (rolls 10 to top, stack [20, 30, 10], 30 + 10) ="; G
350 IF G <> 40 THEN PRINT "FAILED ROLL": END
360 PRINT "RPN STACK OPERATORS OK"
370 RUN "tests/basic/39_maps_serialization.bas"

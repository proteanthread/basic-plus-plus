10 REM Test Suite for Built-in Variables PI, CLOCK, and CLOCK$
20 PRINT "Testing PI..."
30 P1 = PI
40 P2 = PI()
50 IF ABS(P1 - 3.141592653589793) > 1E-12 THEN PRINT "ERROR: PI value mismatch": END
60 IF ABS(P2 - 3.141592653589793) > 1E-12 THEN PRINT "ERROR: PI() value mismatch": END
70 PRINT "PI test passed: "; P1

80 PRINT "Testing CLOCK$..."
90 CSTR$ = CLOCK$
100 IF LEN(CSTR$) <> 19 THEN PRINT "ERROR: CLOCK$ length mismatch (expected 19, got "; LEN(CSTR$); ")": END
110 IF MID$(CSTR$, 5, 1) <> "-" OR MID$(CSTR$, 8, 1) <> "-" OR MID$(CSTR$, 11, 1) <> " " OR MID$(CSTR$, 14, 1) <> ":" OR MID$(CSTR$, 17, 1) <> ":" THEN PRINT "ERROR: CLOCK$ format mismatch: "; CSTR$: END
120 PRINT "CLOCK$ test passed: "; CSTR$

130 PRINT "Testing CLOCK..."
140 C1 = CLOCK
150 C2 = CLOCK()
160 IF C1 < 20260000000000 OR C1 > 30000000000000 THEN PRINT "ERROR: CLOCK value out of range: "; C1: END
170 IF C2 < 20260000000000 OR C2 > 30000000000000 THEN PRINT "ERROR: CLOCK() value out of range: "; C2: END
180 PRINT "CLOCK test passed: "; C1
190 PRINT "ALL TESTS PASSED SUCCESSFULLY!"
200 END

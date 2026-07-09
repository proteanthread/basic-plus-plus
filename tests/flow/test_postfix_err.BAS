10 REM ============================================================
20 REM TEST: Postfix Conditionals Syntax Errors
30 REM GOAL: Verify syntax error is thrown for declaratives
40 REM EXPECTATIONS: DIM ... IF ... should throw a trappable syntax error
50 REM ============================================================

100 PRINT "Testing Postfix Syntax Errors..."
110 ON ERROR GOTO 200
120 X = 10
130 DIM A(10) IF X = 10
140 ASSERT 0, "Syntax error not thrown for declarative postfix conditional"
150 END

200 PRINT "Error caught: "; ERR$
210 E = ERR
220 ASSERT E > 0, "Expected non-zero error code"
230 RESUME 300

300 PRINT "Postfix Syntax Error tests passed."
310 END

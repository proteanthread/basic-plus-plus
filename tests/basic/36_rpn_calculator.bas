10 REM =====================================================================
20 REM BASIC++ RPN Calculator Integration Test
30 REM =====================================================================
40 REM - What can be changed: Expression numbers or operators.
50 REM - What cannot be changed: The exact control flow structures and assertions.
60 REM - What to expect: Verification of RPN expression evaluation via {} blocks.
70 REM - What to do if something breaks: Verify RPN evaluation in src/expression/eval.c.
80 REM =====================================================================
90 PRINT "RUNNING RPN CALCULATOR INTEGRATION TESTS..."
100 REM RPN is always available via {} blocks (no OPTION RPN toggle needed)
110 REM Test binary operators
120 X = { 3 4 * 5 + }
130 PRINT "3 4 * 5 + ="; X
131 IF X <> 17 THEN PRINT "FAILED binary op test": END
140 REM Test unary minus
150 Y = { 5 - }
160 PRINT "Unary minus 5 ="; Y
161 IF Y <> -5 THEN PRINT "FAILED unary minus": END
170 REM Test variable lookup inside RPN
180 Z = { X Y + }
190 PRINT "X Y + ="; Z
191 IF Z <> 12 THEN PRINT "FAILED variable lookup in RPN": END
201 REM Test on-the-fly RPN in curly braces
202 W = { 3 4 * 5 + }
203 PRINT "On-the-fly RPN { 3 4 * 5 + } ="; W
204 IF W <> 17 THEN PRINT "FAILED on-the-fly RPN": END
205 U = { 2 3 + } * { 4 5 + }
206 PRINT "Composite RPN { 2 3 + } * { 4 5 + } ="; U
207 IF U <> 45 THEN PRINT "FAILED composite RPN": END
210 PRINT "RPN OK"
215 PRINT "ALL REGRESSION TESTS COMPLETED SUCCESSFULLY!"
220 RUN "tests/basic/37_try_catch.bas"

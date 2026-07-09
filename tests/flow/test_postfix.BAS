10 REM ============================================================
20 REM TEST: Postfix Conditionals
30 REM GOAL: Verify IF (POSTFIX) and UNLESS execute correctly
40 REM EXPECTATIONS: IF should only execute when condition is true. 
50 REM               UNLESS should only execute when condition is false.
60 REM ============================================================

100 PRINT "Testing Postfix IF..."
110 X = 10
120 FLAG = 0
130 FLAG = 1 IF X = 10
140 ASSERT FLAG = 1, "Postfix IF failed on TRUE condition"

150 FLAG = 0
160 FLAG = 1 IF X = 5
170 ASSERT FLAG = 0, "Postfix IF failed on FALSE condition"

200 PRINT "Testing Postfix UNLESS..."
210 Y = 20
220 FLAG2 = 0
230 FLAG2 = 1 UNLESS Y = 20
240 ASSERT FLAG2 = 0, "Postfix UNLESS failed on TRUE condition"

250 FLAG2 = 0
260 FLAG2 = 1 UNLESS Y = 10
270 ASSERT FLAG2 = 1, "Postfix UNLESS failed on FALSE condition"

300 PRINT "Testing Postfix with GOTO..."
310 Z = 5
320 GOTO 350 IF Z = 5
330 ASSERT 0, "GOTO IF failed to branch"

350 GOTO 380 UNLESS Z = 10
360 ASSERT 0, "GOTO UNLESS failed to branch"

380 PRINT "Postfix Conditional tests passed."
390 END

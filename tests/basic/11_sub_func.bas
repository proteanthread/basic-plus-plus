10 REM =====================================================================
20 REM BASIC++ v6.0.0 SUB and FUNCTION Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Argument values, procedure print strings.
60 REM - What cannot be changed: Scopes parameters binding, local variable declarations, and returns assertions.
70 REM - What to expect: Calls a SUB that updates a local variable, calls a FUNCTION, and asserts return results.
80 REM - What to do if something breaks: Check active scopes prefix bindings and SubStack frames depth.
90 REM =====================================================================
100 PRINT "RUNNING SUB/FUNCTION TEST..."
110 LET G% = 100
120 CALL MY_SUB(10, 20)
130 LET RES% = MY_FUNC%(5)
140 IF RES% = 25 THEN GOTO 170
150 PRINT "FAILED: FUNCTION RETURNED WRONG VALUE"
160 END
170 IF G% = 100 THEN GOTO 200
180 PRINT "FAILED: GLOBAL VARIABLE CORRUPTED"
190 END
200 PRINT "SUB/FUNCTION OK"
205 ON ERROR GOTO 208
206 OPEN "lite.flag" FOR INPUT AS #1: CLOSE #1: RUN "tests/basic/15_multitasking.bas"
208 RUN "tests/basic/12_dialect.bas"
220 SUB MY_SUB(A, B)
230   LET G% = 50
240   PRINT "SUB CALL WORKED"
250 END SUB
260 FUNCTION MY_FUNC%(X)
270   MY_FUNC% = X * X
280 END FUNCTION

10 REM =====================================================================
20 REM BASIC++ ENUM Statement Integration Test
30 REM =====================================================================
40 REM - What can be changed: Enum member names, custom values.
50 REM - What cannot be changed: ENUM...END ENUM structure, variable asserts.
60 REM - What to expect: Verification of ENUM constant variable mapping.
70 REM - What to do if something breaks: Check stmt_struct.c or lexer.c.
80 REM =====================================================================
100 PRINT "RUNNING ENUM INTEGRATION TESTS..."

110 ENUM Color
120   Red
130   Green = 5
140   Blue
150 END ENUM

160 REM Verify values
170 IF Color.Red <> 0 THEN PRINT "FAILED Color.Red check": END
180 IF Color.Green <> 5 THEN PRINT "FAILED Color.Green check": END
190 IF Color.Blue <> 6 THEN PRINT "FAILED Color.Blue check": END

200 ENUM Status
210   Pending = 100
220   Active
230   Inactive = 200
240 END ENUM

250 IF Status.Pending <> 100 THEN PRINT "FAILED Status.Pending check": END
260 IF Status.Active <> 101 THEN PRINT "FAILED Status.Active check": END
270 IF Status.Inactive <> 200 THEN PRINT "FAILED Status.Inactive check": END

280 PRINT "ENUM INTEGRATION TESTS OK"
290 RUN "tests/basic/55_with_test.bas"

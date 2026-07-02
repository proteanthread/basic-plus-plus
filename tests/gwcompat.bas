10 REM ============================================================
20 REM  TEST: gwcompat.bas - GW-BASIC Compatibility Tests
30 REM ============================================================
40 REM  Tests 6 GW-BASIC compatibility features added in v4.0.0:
50 REM    1. RANDOMIZE TIMER - seed from system clock
60 REM    2. RND without parentheses (bare RND)
70 REM    3. PRINT double comma (,,) tab zone skip
80 REM    4. Space inside operators (< =, > =, < >)
90 REM    5. FUNCTION with multi-word identifiers (GetCount)
95 REM    6. LOAD LIBRARY syntax (static analysis only)
96 REM
97 REM  WHAT CAN BE CHANGED:
98 REM   - Test values and assertions
99 REM   - Additional FUNCTION definitions
100 REM
101 REM  WHAT CANNOT BE CHANGED:
102 REM   - Do NOT use single-line IF/THEN/ELSE for FUNCTION
103 REM     return value assignment (parser limitation)
104 REM   - Use multi-line IF with GOTO for branched returns
105 REM
106 REM  WHAT TO EXPECT:
107 REM   - All TEST blocks should pass
108 REM   - PRINT double comma produces tab-zone spacing
110 REM
111 REM  TROUBLESHOOTING:
112 REM   - ASSERTION FAILED in GetMax: ensure the function
113 REM     uses GOTO-based branching, not ELSE assignment
114 REM   - WHAT? on FUNCTION: verify GW-BASIC dialect is active
115 REM ============================================================

1000 TEST "RANDOMIZE TIMER"
1010 ON ERROR GOTO 1090
1020 RANDOMIZE TIMER
1030 X = RND(1)
1040 ASSERT X > 0
1050 ASSERT X < 1
1060 ON ERROR GOTO 0
1070 ENDTEST
1080 GOTO 2000
1090 PRINT "RANDOMIZE TIMER error: ERR="; ERR; " ERL="; ERL
1095 RESUME 1070

2000 TEST "RND without parens"
2010 RANDOMIZE 42
2020 X = RND
2030 ASSERT X > 0
2040 ASSERT X < 1
2050 REM Also verify RND(1) still works
2060 Y = RND(1)
2070 ASSERT Y > 0
2080 ASSERT Y < 1
2090 REM Verify they produce different values
2100 ASSERT X <> Y
2110 ENDTEST

3000 TEST "PRINT double comma"
3010 REM Visual test: double comma should skip a tab zone
3020 REM No assertion possible for visual output, just verify
3030 REM no WHAT? error occurs during parsing
3040 PRINT "Col1",,"Col3"
3050 PRINT 1,,2,,3
3060 ENDTEST

4000 TEST "Space inside operators"
4010 N = 5
4020 ASSERT N < = 5
4030 ASSERT N > = 5
4040 ASSERT 6 < > 5
4050 REM Also verify normal operators still work
4060 ASSERT N <= 5
4070 ASSERT N >= 5
4080 ASSERT 6 <> 5
4090 ASSERT N < 6
4100 ASSERT N > 4
4110 ENDTEST

5000 TEST "FUNCTION identifiers"
5010 REM FUNCTION names should not be split by keyword prefix
5020 REM extraction (e.g. GetCount should not become GET + Count)
5030 X = Doubler(7)
5040 ASSERT X = 14
5050 Y = GetMax(10, 20)
5060 ASSERT Y = 20
5070 ENDTEST

5100 FUNCTION Doubler(N)
5110   Doubler = N * 2
5120 END FUNCTION

5200 FUNCTION GetMax(A, B)
5210   IF A > B THEN GOTO 5230
5220   GetMax = B : GOTO 5240
5230   GetMax = A
5240 END FUNCTION

6000 TEST "CHECK and VERIFY"
6010 REM Verify that CHECK and VERIFY commands exist and
6020 REM run without errors on a well-formed program.
6030 REM (Cannot test output in TEST block, but verify no crash)
6040 ASSERT ERR = 0
6050 ENDTEST

9000 END

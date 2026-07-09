10 REM ============================================================
20 REM TEST: ERR$
30 REM GOAL: Verify error string fetching.
40 REM EXPECTATIONS: ERR$ should return a non-empty string for standard errors.
50 REM ============================================================
60 PRINT "Testing ERR$..."
70 ON ERROR GOTO 100
80 ERROR 53 : REM File not found
90 ENDTEST
100 E$ = ERR$
110 ASSERT LEN(E$) > 0, "ERR$ returned empty string"
120 PRINT "ERR$ returned: "; E$
130 RESUME 140
140 PRINT "ERR$ passed."

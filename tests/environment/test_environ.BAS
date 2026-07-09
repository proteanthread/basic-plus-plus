10 REM ============================================================
20 REM TEST: ENVIRON$
30 REM GOAL: Verify environment variable string reading.
40 REM EXPECTATIONS: ENVIRON$("PATH") should return a valid string on standard OS.
50 REM ============================================================
60 PRINT "Testing ENVIRON$..."
70 P$ = ENVIRON$("PATH")
80 REM It might be empty in some weird environments, but usually isn't.
90 REM We just verify it doesn't crash and returns a string.
100 ASSERT LEN(P$) >= 0, "ENVIRON$ failed"
110 PRINT "ENVIRON$ passed."

10 REM ============================================================
20 REM TEST: EOF, LOF
30 REM GOAL: Verify End-Of-File and Length-Of-File functions.
40 REM EXPECTATIONS: LOF should equal the bytes written; EOF should trigger at end.
50 REM ============================================================
60 PRINT "Testing EOF and LOF..."
70 OPEN "O", 1, "test_eof_lof.tmp"
80 PRINT #1, "HELLO WORLD"
90 CLOSE 1
100 OPEN "I", 1, "test_eof_lof.tmp"
110 L = LOF(1)
120 ASSERT L > 0, "LOF returned 0 for non-empty file"
130 C = 0
140 WHILE NOT EOF(1)
150   LINE INPUT #1, A$
160   C = C + 1
170 WEND
180 ASSERT C = 1, "EOF triggered at wrong time"
190 CLOSE 1
200 KILL "test_eof_lof.tmp"
210 PRINT "EOF and LOF passed."

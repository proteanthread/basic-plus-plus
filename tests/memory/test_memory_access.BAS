10 REM ============================================================
20 REM TEST: PEEK, POKE, PEEKB, POKEB, SIZE
30 REM GOAL: Verify memory access and size utilities.
40 REM EXPECTATIONS: Can POKE and PEEK safely, SIZE returns int.
50 REM ============================================================
60 PRINT "Testing Memory utilities..."
70 REM BASIC++ virtual memory is mapped, address 1000 is safe in GW mode
80 POKE 1000, 42
90 V = PEEK(1000)
100 ASSERT V = 42, "PEEK/POKE roundtrip failed"
110 POKEB 1, 1001, 84
120 V2 = PEEKB(1, 1001)
130 ASSERT V2 = 84, "PEEKB/POKEB roundtrip failed"
140 S = SIZE
150 ASSERT S > 0, "SIZE returned 0"
160 PRINT "Memory utilities passed."

10 REM ============================================================
20 REM TEST: BIOCHECKSUM
30 REM GOAL: Verify CRC-16 calculation for block device data
40 REM EXPECTATIONS: BIOCHECKSUM("ABC") should return a deterministic integer.
50 REM ============================================================
60 PRINT "Testing BIOCHECKSUM..."
70 DATA$ = "HELLO WORLD"
80 CHK = BIOCHECKSUM(DATA$)
90 ASSERT CHK <> 0, "BIOCHECKSUM failed to return non-zero CRC"
100 PRINT "BIOCHECKSUM passed: "; CHK

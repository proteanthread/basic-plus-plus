10 REM ============================================================
20 REM TEST: CSRLIN
30 REM GOAL: Verify that CSRLIN correctly returns the current cursor line.
40 REM EXPECTATIONS: CSRLIN should return an integer between 1 and terminal height.
50 REM ============================================================
60 PRINT "Testing CSRLIN..."
70 LOCATE 5, 1
80 CL = CSRLIN
90 ASSERT CL = 5, "CSRLIN did not return 5 after LOCATE 5,1"
100 PRINT "CSRLIN passed."

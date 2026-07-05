10 REM QBASIC Static Variables Test
20 FOR I = 1 TO 3
30   CALL CountUp()
40 NEXT I
50 PRINT "STRUCTURED_STATIC_TEST PASSED"
60 END
70 SUB CountUp ()
80   STATIC counter
90   counter = counter + 1
100  PRINT "Counter is now "; counter
110  IF counter > 3 THEN PRINT "FAILED: Static state corrupted!": END
120 END SUB

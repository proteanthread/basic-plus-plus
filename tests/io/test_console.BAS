' --- CONSOLE & DISPLAY TEST ---
' Tests: CSRLIN, POS

' CSRLIN and POS(0) query the console/terminal cursor position.
' To avoid host OS vulnerabilities, we must ensure they don't break on edge cases
' and only return bounded integer values.

PRINT "TESTING POS..."
P = POS(0)
IF P < 1 OR P > 1000 THEN PRINT "POS BOUNDS FAILED": GOTO 9999

PRINT "TESTING CSRLIN..."
R = CSRLIN
IF R < 1 OR R > 1000 THEN PRINT "CSRLIN BOUNDS FAILED": GOTO 9999

' Test multiple calls
FOR I = 1 TO 10
  P = POS(0)
  R = CSRLIN
NEXT I

' Check that arguments outside expected ranges for POS don't cause a crash.
' QBASIC POS takes a dummy argument, usually 0. Let's pass large numbers or negatives.
P = POS(9999)
P = POS(-1)
' As long as it didn't crash, we pass.

PRINT "ALL TESTS PASSED"
END
9999 PRINT "TEST SUITE FAILED"

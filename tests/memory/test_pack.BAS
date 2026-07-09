' --- MEMORY PACKING TEST ---
' Tests: MKD$, MKI$, MKS$, CVD, CVI, CVS

' Integer packing (2 bytes)
I1 = 12345
S1$ = MKI$(I1)
I2 = CVI(S1$)
IF I1 <> I2 THEN PRINT "MKI$/CVI FAILED": GOTO 9999

' Single packing (4 bytes)
F1 = 3.14159
S2$ = MKS$(F1)
F2 = CVS(S2$)
' floats might have precision differences so we check bound
IF F2 < 3.14 OR F2 > 3.15 THEN PRINT "MKS$/CVS FAILED": GOTO 9999

' Double packing (8 bytes)
D1 = 1.23456789
S3$ = MKD$(D1)
D2 = CVD(S3$)
IF D2 < 1.23 OR D2 > 1.24 THEN PRINT "MKD$/CVD FAILED": GOTO 9999

PRINT "ALL TESTS PASSED"
END
9999 PRINT "TEST SUITE FAILED"

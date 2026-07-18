' What can be changed: Filename and values.
' What cannot be changed: Test structure and output assertions.
' What to expect: Proper buffering of FIELD string variables.
' What to do if something breaks: Check VDev internal record buffers.

10 OPEN "test_rand.dat" FOR RANDOM AS #1 LEN = 10
20 FIELD 1, 2 AS I$, 4 AS S$, 4 AS D$
30 LSET I$ = MKI$(42)
40 LSET S$ = MKS$(3.125)
50 LSET D$ = "TEST"
60 PUT #1, 1
70 CLOSE 1

100 OPEN "test_rand.dat" FOR RANDOM AS #1 LEN = 10
110 FIELD 1, 2 AS I$, 4 AS S$, 4 AS D$
120 GET #1, 1
122 PRINT "LEN I$:", LEN(I$)
124 PRINT "LEN S$:", LEN(S$)
126 PRINT "LEN D$:", LEN(D$)
130 IF CVI(I$) <> 42 THEN PRINT "FAIL I$": END
140 IF CVS(S$) <> 3.125 THEN PRINT "FAIL S$": END
150 IF D$ <> "TEST" THEN PRINT "FAIL D$": END
160 PRINT "PASS"
170 CLOSE 1
180 SYSTEM

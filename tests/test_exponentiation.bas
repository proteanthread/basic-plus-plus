10 REM Test Suite for Exponentiation Operators (^ and **)
20 PRINT "Testing PI^PI and PI**PI..."
30 E1 = PI^PI
40 E2 = PI**PI
50 IF ABS(E1 - 36.4621596072079) > 1E-5 THEN PRINT "ERROR: PI^PI mismatch: "; E1: END
60 IF ABS(E2 - 36.4621596072079) > 1E-5 THEN PRINT "ERROR: PI**PI mismatch: "; E2: END
70 PRINT "PI^PI test passed: "; E1

80 PRINT "Testing Unary Minus Precedence (-2^2)..."
90 U1 = -2^2
100 U2 = (-2)^2
110 IF U1 <> -4 THEN PRINT "ERROR: -2^2 should be -4, got "; U1: END
120 IF U2 <> 4 THEN PRINT "ERROR: (-2)^2 should be 4, got "; U2: END
130 PRINT "Unary minus precedence test passed: -2^2 = "; U1; ", (-2)^2 = "; U2

140 PRINT "Testing Right-to-Left Associativity (2^3^2)..."
150 R1 = 2^3^2
160 R2 = (2^3)^2
170 IF R1 <> 512 THEN PRINT "ERROR: 2^3^2 should be 512, got "; R1: END
180 IF R2 <> 64 THEN PRINT "ERROR: (2^3)^2 should be 64, got "; R2: END
190 PRINT "Associativity test passed: 2^3^2 = "; R1; ", (2^3)^2 = "; R2

200 PRINT "Testing integer exponentiation (10**3)..."
210 P10 = 10**3
220 IF P10 <> 1000 THEN PRINT "ERROR: 10**3 should be 1000, got "; P10: END
230 PRINT "10**3 test passed: "; P10

240 PRINT "ALL EXPONENTIATION TESTS PASSED SUCCESSFULLY!"
250 END

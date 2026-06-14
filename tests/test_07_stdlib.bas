' ====================================================================
' BASIC++ Test 07: Standard Library and Dialect API
' ====================================================================

10 PRINT "--- Testing Core Immutable API ---"
20 PRINT "String length of 'BASIC++': "; LEN("BASIC++")
30 PRINT "RND(10): "; RND(10)
40 PRINT "RND(10) again (PCG32 deterministic check): "; RND(10)

50 PRINT "--- Testing Dialect-Overridable API ---"
60 PRINT "If Dialect is GWBS, print formatting may differ."
70 LOAD PROFILE "QBASIC"
80 PRINT "Loaded QBASIC profile..."

90 PRINT "--- Testing VDEV I/O Abstraction ---"
100 PRINT "This output is routed securely through the Virtual Device (vdev) API."

110 PRINT "--- Testing Extensible Error Architecture ---"
120 PRINT "ERR$(11) should be 'Division by zero': "; ERR$(11)
130 PRINT "ERR$(2) should be 'Syntax error': "; ERR$(2)
140 PRINT "ERR$(999) should be 'Unknown Error' (unless registered): "; ERR$(999)

150 END

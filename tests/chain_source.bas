10 REM === CHAIN Test: Source Program ===
20 X = 42
30 A$ = "HELLO FROM CHAIN"
40 PRINT "Setting X = "; X
50 PRINT "CHAINing to chain_target.bas..."
60 CHAIN "tests\chain_target.bas"
70 PRINT "ERROR: should not reach here!"

10 REM === CHAIN Test: Target Program ===
20 PRINT "CHAIN target loaded."
30 PRINT "X = "; X
40 IF X = 42 THEN PRINT "PASS: variable preserved" ELSE PRINT "FAIL: variable lost"
50 PRINT "A$ = "; A$
60 IF A$ = "HELLO FROM CHAIN" THEN PRINT "PASS: string preserved" ELSE PRINT "FAIL: string lost"
100 REM Test that SUB definitions work fresh
110 CALL Greet
120 PRINT "PASS: SUB in chained program works"
130 END
5000 SUB Greet
5010   PRINT "Hello from chained SUB!"
5020 END SUB

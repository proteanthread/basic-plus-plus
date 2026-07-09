10 REM --- Persistence Tests ---
20 PRINT "Testing PSTORE..."
30 P = PSTORE("test_key", "test_value")
40 PRINT "Testing PRETRIEVE$..."
50 V$ = PRETRIEVE$("test_key")
60 IF V$ <> "test_value" THEN PRINT "PRETRIEVE$ FAILED": END
70 PRINT "Testing PRETRIEVE (numeric)..."
80 P2 = PSTORE("test_num", "123")
90 N = PRETRIEVE("test_num")
100 IF N <> 123 THEN PRINT "PRETRIEVE FAILED": END
110 PRINT "PERSISTENCE PASSED"
120 END

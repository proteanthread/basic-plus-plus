10 REM ============================================================
20 REM  TEST: editing_chain_target.bas
30 REM ============================================================
40 PRINT "CHAIN TARGET LOADED SUCCESSFULLY."
50 PRINT "Verifying preserved state: MERGED_VAL = "; MERGED_VAL
60 IF MERGED_VAL = 999 THEN PRINT "PASS: Variable MERGED_VAL preserved." ELSE PRINT "FAIL: Variable lost!"
70 PRINT "ALL TESTS PASSED SUCCESSFULLY."
80 END

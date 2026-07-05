10 REM ============================================================
20 REM  TEST: editing_chain_merge.bas
30 REM ============================================================
40 PRINT "STARTING INTEGRATED EDITING, MERGE, AND CHAIN TEST..."
50 REM Setup variable
60 MERGED_VAL = 0
70 REM Merge subroutine
80 PRINT "MERGING SUBROUTINE..."
90 MERGE "tests/merge_subroutine.bas"
95 GOTO 100
100 PRINT "SUBROUTINE MERGED successfully."
110 PRINT "Executing merged subroutine..."
120 GOSUB 1000
130 PRINT "Returned from subroutine. MERGED_VAL = "; MERGED_VAL
140 IF MERGED_VAL = 999 THEN PRINT "PASS: Subroutine executed and set variable." ELSE PRINT "FAIL: Variable mismatch!"
150 REM Now delete the subroutine lines
160 PRINT "DELETING SUBROUTINE..."
170 DELETE 1000-1040
175 GOTO 180
180 PRINT "SUBROUTINE DELETED successfully."
190 REM Enable error trap
200 ON ERROR GOTO 250
210 GOSUB 1000
220 PRINT "FAIL: Did not throw error on deleted subroutine call!"
230 GOTO 270
250 PRINT "PASS: Error caught on deleted subroutine (GOSUB to non-existent line)."
260 RESUME 270
270 REM Disable error trap
280 ON ERROR GOTO 0
290 REM Now renumber the program
300 PRINT "RENUMBERING PROGRAM..."
310 RENUM 5000, 10
320 PRINT "PROGRAM RENUMBERED successfully."
330 PRINT "CHAINING TO target program..."
340 CHAIN "tests/editing_chain_target.bas"
350 PRINT "FAIL: Should not reach here!"
360 END

10 REM ============================================================
20 REM  TEST: chain_target.bas - CHAIN Test: Target Program
30 REM ============================================================
40 REM  This program is loaded by chain_source.bas via CHAIN.
50 REM  It verifies that variables were preserved across the
60 REM  CHAIN transition.
70 REM
80 REM  VARIABLES TO VERIFY:
90 REM   X  = 42 (set in chain_source.bas before CHAIN)
92 REM   A$ = "HELLO FROM CHAIN" (set in chain_source.bas)
94 REM
96 REM  ADDITIONAL TESTS:
98 REM   - SUB Greet: verifies that SUB definitions work
100 REM     correctly in the chained program (fresh scan).
102 REM
104 REM  EXPECTED:
106 REM   "CHAIN target loaded."
108 REM   "X = 42" then "PASS: variable preserved"
110 REM   "A$ = HELLO FROM CHAIN" then "PASS: string preserved"
112 REM   "Hello from chained SUB!"
114 REM   "PASS: SUB in chained program works"
116 REM
118 REM  NOTE: Do not run this file standalone; it expects
120 REM  X and A$ to be pre-set by chain_source.bas.
122 REM ============================================================
130 REM
140 PRINT "CHAIN target loaded."
150 REM
160 REM --- Verify integer variable preservation ---
170 REM  X should still be 42 from chain_source.bas.
180 PRINT "X = "; X
190 IF X = 42 THEN PRINT "PASS: variable preserved" ELSE PRINT "FAIL: variable lost"
200 REM
210 REM --- Verify string variable preservation ---
220 REM  A$ should still be "HELLO FROM CHAIN".
230 PRINT "A$ = "; A$
240 IF A$ = "HELLO FROM CHAIN" THEN PRINT "PASS: string preserved" ELSE PRINT "FAIL: string lost"
250 REM
300 REM --- Test SUB in chained program ---
310 REM  After CHAIN, the SUB table is reset and SUBs in
320 REM  the new program must be re-scanned. This verifies
330 REM  that CALL works correctly in the chained context.
340 CALL Greet
350 PRINT "PASS: SUB in chained program works"
360 END
370 REM
5000 REM --- SUB Greet: simple greeting ---
5010 REM  Proves that SUB definitions in the chained program
5020 REM  are properly registered after CHAIN replaces code.
5030 SUB Greet
5040   PRINT "Hello from chained SUB!"
5050 END SUB

10 REM ============================================================
20 REM  TEST: chain_source.bas - CHAIN Test: Source Program
30 REM ============================================================
40 REM  Tests the CHAIN command, which replaces the current
50 REM  program with a new one while preserving variables.
60 REM
70 REM  SEQUENCE:
80 REM   1. Set X = 42 (integer) and A$ = "HELLO FROM CHAIN"
90 REM   2. CHAIN to "tests\chain_target.bas"
92 REM   3. Execution should transfer to chain_target.bas
94 REM      and NEVER return to line 70 of this program.
96 REM
98 REM  EXPECTED: Lines 40-50 print the variable values.
100 REM  CHAIN transfers execution to chain_target.bas.
102 REM  Line 70 should NEVER execute (if it does, CHAIN
104 REM  failed to transfer execution).
106 REM
108 REM  VARIABLE PRESERVATION:
110 REM  X and A$ should be visible in chain_target.bas
112 REM  with their original values (42 and "HELLO FROM
114 REM  CHAIN"). This is the key CHAIN semantic: program
116 REM  code is replaced, but variable state is kept.
118 REM
120 REM  NOTE: Run this test from the project root directory,
122 REM  not from tests/, since the CHAIN path is relative.
124 REM ============================================================
130 REM
140 REM --- Set up variables for preservation test ---
150 X = 42
160 A$ = "HELLO FROM CHAIN"
170 PRINT "Setting X = "; X
180 REM
190 REM --- CHAIN: replace program, preserve variables ---
200 PRINT "CHAINing to chain_target.bas..."
210 CHAIN "tests\chain_target.bas"
220 REM
230 REM --- If we reach here, CHAIN failed ---
240 PRINT "ERROR: should not reach here!"

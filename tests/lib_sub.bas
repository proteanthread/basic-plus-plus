1 REM ============================================================
2 REM  TEST: lib_sub.bas - External Library Definitions
3 REM ============================================================
4 REM
5 REM  PURPOSE:
6 REM  Provides SUB and FUNCTION definitions loaded by
7 REM  external_call.bas via DECLARE EXTERNAL ... FROM.
8 REM
9 REM  WHAT CAN BE CHANGED:
10 REM   - Additional SUB/FUNCTION definitions can be added
11 REM   - Parameter names and internal logic
12 REM
13 REM  WHAT CANNOT BE CHANGED:
14 REM   - SUB/FUNCTION names must NOT start with a BASIC
15 REM     keyword (e.g. PRINTVAL fails because PRINT is
16 REM     extracted as a keyword prefix by the lexer)
17 REM   - SUB/FUNCTION names must match DECLARE statements
18 REM     in external_call.bas exactly
19 REM   - This file is NOT standalone; it is loaded by
20 REM     external_call.bas at runtime
21 REM
22 REM  WHAT TO EXPECT:
23 REM   - Running this file standalone will define routines
24 REM     but never call them (no visible output)
25 REM   - When loaded via DECLARE EXTERNAL FROM, the SUB
26 REM     and FUNCTION become callable from the host program
27 REM
28 REM  TROUBLESHOOTING:
29 REM   - WHAT? on SUB: ensure E116 or QBASIC dialect
30 REM   - WHAT? on name: ensure name avoids keyword prefixes
31 REM ============================================================
32 REM
40 SUB ShowVal(V)
50   PRINT "LIBRARY SUB VAL="; V
60 END SUB
70 FUNCTION AddFour(V)
80   AddFour = V + 4
90 END FUNCTION

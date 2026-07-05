1 REM ============================================================
2 REM  TEST: external_call.bas - External Library Declarations
3 REM ============================================================
4 REM
5 REM  PURPOSE:
6 REM  Tests DECLARE EXTERNAL SUB/FUNCTION with FROM clause,
7 REM  loading callable routines from an external .BAS file.
8 REM
9 REM  WHAT CAN BE CHANGED:
10 REM   - The external file path (must match actual location)
11 REM   - Parameter values passed to ShowVal and AddFour
12 REM   - Additional DECLARE EXTERNAL statements can be added
13 REM
14 REM  WHAT CANNOT BE CHANGED:
15 REM   - SUB/FUNCTION names must NOT start with a BASIC
16 REM     keyword (e.g. PRINTVAL fails because PRINT is
17 REM     extracted as a keyword prefix by the lexer)
18 REM   - The DECLARE EXTERNAL ... FROM syntax
19 REM   - lib_sub.bas must exist and contain matching defs
20 REM
21 REM  WHAT TO EXPECT:
22 REM   - ShowVal(42) prints "LIBRARY SUB VAL= 42"
23 REM   - AddFour(38) returns 42, prints "LIBRARY FUNC VAL= 42"
24 REM   - No errors should appear
25 REM
26 REM  TROUBLESHOOTING:
27 REM   - WHAT? on DECLARE: DECLARE requires QBASIC or E116
28 REM     dialect (included in default basicpp-console.cfg)
29 REM   - WHAT? on SUB name: ensure name does not start
30 REM     with a BASIC keyword (PRINT, GET, LET, etc.)
31 REM   - File not found: verify lib_sub.bas path relative to
32 REM     the working directory (run from project root)
33 REM ============================================================
34 REM
40 DECLARE EXTERNAL SUB ShowVal(V) FROM "tests/lib_sub.bas"
50 DECLARE EXTERNAL FUNCTION AddFour(V) FROM "tests/lib_sub.bas"
60 CALL ShowVal(42)
70 PRINT "LIBRARY FUNC VAL="; AddFour(38)
80 PRINT "External call tests completed successfully."
90 END

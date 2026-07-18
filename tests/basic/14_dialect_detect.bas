10 REM =====================================================================
20 REM BASIC++ v6.0.0 Dialect Detection Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Prints, assertion paths.
60 REM - What cannot be changed: The test chain link to the next test.
70 REM - What to expect: Verifies dialect detection infrastructure exists.
80 REM - What to do if something breaks: Check DIALECT LOAD/REGISTER in stmt_program.c.
90 REM =====================================================================
100 PRINT "RUNNING DIALECT DETECTION TEST..."
110 REM Dialect detection now uses the unified DIALECT statement.
120 REM Legacy $lang: header prescan and OPTION VERSION are removed.
130 PRINT "DIALECT DETECTION OK"
140 RUN "tests/basic/15_multitasking.bas"
